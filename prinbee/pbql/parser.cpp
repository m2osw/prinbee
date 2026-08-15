// Copyright (c) 2024-2025  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/prinbee
// contact@m2osw.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


/** \file
 * \brief Parser of the Prinbee Query Language.
 *
 * The Pribee Query Language (PBQL) is an SQL-like language. This file
 * handles the grammar from the token returned by the lexer.
 */

// self
//
#include    "prinbee/pbql/parser.h"

#include    "prinbee/database/context.h"
#include    "prinbee/data/schema.h"
#include    "prinbee/exception.h"


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/compare_switch_string.h>
#include    <snapdev/enum_class_math.h>
#include    <snapdev/to_upper.h>
//#include    <snapdev/stream_fd.h>
#include    <snapdev/tokenize_string.h>
//#include    <snapdev/unique_number.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace pbql
{



parser::parser(lexer::pointer_t l)
    : f_lexer(l)
{
    if(f_lexer == nullptr)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << "lexer missing.";
        throw logic_error(msg.str());
    }
}


/** \brief Allow for the capture of unknown commands by the user.
 *
 * The user of this class (see CUI for an example) can capture unknown
 * commands. The function is called if the command is not recognized
 * as an internal command. Note that this is somewhat limited since
 * it can only capture commands that start with an unrecognized
 * keyword. So for example, only PBQL can capture the commands that
 * start with ALTER, CREATE, DROP, etc. What the capture function
 * receives are other commands such as CLEARSCREEN that maybe would
 * clear the content of the CUI window.
 */
void parser::set_user_capture(capture_t capture)
{
    f_user_capture = capture;
}


bool parser::quit() const
{
    return f_quit;
}


bool parser::parsed() const
{
    return f_parsed;
}


command::vector_t const & parser::parse()
{
    if(f_parsed)
    {
        throw fatal_error("parse() was already called.");
    }
    f_parsed = true;

    for(;;)
    {
        node::pointer_t n(f_lexer->get_next_token());
        switch(n->get_token())
        {
        case token_t::TOKEN_EOF:
            // got to the end, we are done
            //
            return f_commands;

        case token_t::TOKEN_IDENTIFIER:
            {
                // select which function to call based on the identifier
                //
                std::string cmd(n->get_string_upper());
                switch(cmd[0])
                {
                case 'A':
                    if(snapdev::compare_switch_string<"ALTER">(cmd))
                    {
                        // read one more identifier to know what is going to be altered
                        //
                        n = f_lexer->get_next_token();
                        if(n->get_token() == token_t::TOKEN_IDENTIFIER)
                        {
                            cmd = n->get_string_upper();
                            if(snapdev::compare_switch_string<"CONTEXT">(cmd))
                            {
                                parse_alter_context();
                                continue;
                            }
                            else if(snapdev::compare_switch_string<"INDEX">(cmd))
                            {
                                parse_alter_index();
                                continue;
                            }
                            else if(snapdev::compare_switch_string<"TABLE">(cmd))
                            {
                                parse_alter_table();
                                continue;
                            }
                            else if(snapdev::compare_switch_string<"TYPE">(cmd))
                            {
                                parse_alter_type();
                                continue;
                            }
                            else
                            {
                                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                                msg << n->get_location().get_location()
                                    << "ALTER is expected to be followed by CONTEXT, INDEX, TABLE, or TYPE, not \""
                                    << cmd
                                    << "\".";
                                throw invalid_token(msg.str());
                            }
                        }
                        else
                        {
                            snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                            msg << n->get_location().get_location()
                                << "ALTER is expected to be followed by an identifier: CONTEXT, INDEX, TABLE, or TYPE.";
                            throw invalid_token(msg.str());
                        }
                    }
                    break;

                case 'B':
                    if(snapdev::compare_switch_string<"BEGIN">(cmd))
                    {
                        parse_transaction_command(cmd, command_t::COMMAND_BEGIN);
                        continue;
                    }
                    else if(snapdev::compare_switch_string<"BYE">(cmd))
                    {
                        f_quit = true;
                        expect_semi_colon(cmd);
                        return f_commands;
                    }
                    break;

                case 'C':
                    if(snapdev::compare_switch_string<"COMMIT">(cmd))
                    {
                        parse_transaction_command(cmd, command_t::COMMAND_COMMIT);
                        continue;
                    }
                    else if(snapdev::compare_switch_string<"CONFIG">(cmd))
                    {
                        parse_config();
                        continue;
                    }
                    else if(snapdev::compare_switch_string<"CREATE">(cmd))
                    {
                        // read one more identifier to know what is going to be created
                        //
                        n = f_lexer->get_next_token();
                        if(n->get_token() == token_t::TOKEN_IDENTIFIER)
                        {
                            cmd = n->get_string_upper();
                            if(snapdev::compare_switch_string<"CONTEXT">(cmd))
                            {
                                parse_create_context();
                                continue;
                            }
                            else if(snapdev::compare_switch_string<"INDEX">(cmd))
                            {
                                parse_create_index();
                                continue;
                            }
                            else if(snapdev::compare_switch_string<"TABLE">(cmd))
                            {
                                parse_create_table();
                                continue;
                            }
                            else if(snapdev::compare_switch_string<"TYPE">(cmd))
                            {
                                parse_create_type();
                                continue;
                            }
                            else
                            {
                                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                                msg << n->get_location().get_location()
                                    << "CREATE is expected to be followed by: CONTEXT, INDEX, TABLE, TYPE, not \""
                                    << cmd
                                    << "\".";
                                throw invalid_token(msg.str());
                            }
                        }
                        else
                        {
                            snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                            msg << n->get_location().get_location()
                                << "CREATE is expected to be followed by an identifier: CONTEXT, INDEX, TABLE, TYPE.";
                            throw invalid_token(msg.str());
                        }
                    }
                    break;

                case 'E':
                    if(snapdev::compare_switch_string<"EXIT">(cmd))
                    {
                        f_quit = true;
                        expect_semi_colon(cmd);
                        return f_commands;
                    }
                    break;

                case 'Q':
                    if(snapdev::compare_switch_string<"QUIT">(cmd))
                    {
                        f_quit = true;
                        expect_semi_colon(cmd);
                        return f_commands;
                    }
                    break;

                case 'R':
                    if(snapdev::compare_switch_string<"ROLLBACK">(cmd))
                    {
                        parse_transaction_command(cmd, command_t::COMMAND_ROLLBACK);
                        continue;
                    }
                    break;

                case 'S':
                    if(snapdev::compare_switch_string<"SELECT">(cmd))
                    {
                        parse_select();
                        continue;
                    }
                    else if(snapdev::compare_switch_string<"SET">(cmd))
                    {
                        parse_set();
                        continue;
                    }
                    else if(snapdev::compare_switch_string<"SHOW">(cmd))
                    {
                        parse_show();
                        continue;
                    }
                    break;

                }

                // no matches from the standard PBQL language,
                // send the command to the "user" (cui interface uses that
                // when setup in interactive mode)
                //
                if(f_user_capture != nullptr)
                {
                    if(f_user_capture(cmd))
                    {
                        // f_commands should be empty as a result
                        //
                        return f_commands;
                    }
                }

                snaplogger::message msg(snaplogger::severity_t::SEVERITY_SEVERE);
                msg << n->get_location().get_location()
                    << "found unknown command \""
                    << cmd
                    << "\".";
                throw invalid_token(msg.str());
            }
            break;

        default:
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_CRITICAL);
                msg << n->get_location().get_location()
                    << "expected the beginning of the line to start with an identifier representing a PBQL keyword.";
                throw invalid_token(msg.str());
            }

        }
    }
}


void parser::parse_alter_context()
{
    throw not_yet_implemented("parser::alter_context()");
}


void parser::parse_alter_index()
{
    node::pointer_t n(f_lexer->get_next_token());
    if(n->get_token() != token_t::TOKEN_IDENTIFIER)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
        msg << n->get_location().get_location()
            << "expected an identifier after ALTER INDEX.";
        throw invalid_token(msg.str());
    }
    std::string cmd(n->get_string_upper());
    if(snapdev::compare_switch_string<"IF">(cmd))
    {
        bool optional_found(false);
        n = parser::keyword_string(
                  "ALTER INDEX IF"
                , { "EXISTS" }
                , optional_found
                , token_t::TOKEN_IDENTIFIER);
    }

    std::string const name(n->get_string_lower());

    n = f_lexer->get_next_token();
    if(n->get_token() != token_t::TOKEN_IDENTIFIER)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
        msg << n->get_location().get_location()
            << "expected an index action after ALTER INDEX [IF EXISTS] name <action>.";
        throw invalid_token(msg.str());
    }
    cmd = n->get_string_upper();
    enum add_drop_t
    {
        ADD_DROP_NONE,
        ADD_DROP_ADD,
        ADD_DROP_DROP,
    };
    add_drop_t add_drop(add_drop_t::ADD_DROP_NONE);
    switch(cmd[0])
    {
    case 'A':
        if(snapdev::compare_switch_string<"ADD">(cmd))
        {
            add_drop = add_drop_t::ADD_DROP_ADD;
        }
        break;

    case 'D':
        if(snapdev::compare_switch_string<"DROP">(cmd))
        {
            add_drop = add_drop_t::ADD_DROP_DROP;
        }
        break;

    case 'S':
        if(snapdev::compare_switch_string<"SET">(cmd))
        {
            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_IDENTIFIER)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "expected an identifier after the SET of an ALTER INDEX [IF EXISTS] name SET <sub-action>.";
                throw invalid_token(msg.str());
            }
            cmd = n->get_string_upper();
            bool negate(false);
            if(snapdev::compare_switch_string<"NOT">(cmd))
            {
                negate = true;
                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_IDENTIFIER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "expected an identifier after the SET NOT action of an ALTER INDEX [IF EXISTS] name SET NOT <sub-action>.";
                    throw invalid_token(msg.str());
                }
                cmd = n->get_string_upper();
            }
            if(snapdev::compare_switch_string<"SECURE">(cmd))
            {
                // TODO: set secure or NOT secure

                expect_semi_colon("ALTER INDEX ...;");
                return;
            }
            if(snapdev::compare_switch_string<"SPARSE">(cmd))
            {
                // TODO: set sparse or NOT sparse

                expect_semi_colon("ALTER INDEX ...;");
                return;
            }
            if(negate)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "unexpected NOT with the ALTER INDEX [IF EXISTS] name SET "
                    << cmd
                    << " action.";
                throw invalid_token(msg.str());
            }
            if(snapdev::compare_switch_string<"MODEL">(cmd))
            {
                n = f_lexer->get_next_token();
                if(n->get_token() == token_t::TOKEN_EQUAL)
                {
                    // skip the optional equal sign
                    //
                    n = f_lexer->get_next_token();
                }
                if(n->get_token() != token_t::TOKEN_IDENTIFIER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "expected an identifier with the model name after the SET MODEL action of an ALTER INDEX [IF EXISTS] name SET MODEL [=] <model>.";
                    throw invalid_token(msg.str());
                }
                model_t const model(name_to_model(n->get_string()));

                // TODO: set model in index
                snapdev::NOT_USED(model);

                expect_semi_colon("ALTER INDEX ...;");
                return;
            }
            if(snapdev::compare_switch_string<"COMMENT">(cmd))
            {
                n = f_lexer->get_next_token();
                if(n->get_token() == token_t::TOKEN_EQUAL)
                {
                    // skip the optional equal sign
                    //
                    n = f_lexer->get_next_token();
                }
                if(n->get_token() != token_t::TOKEN_STRING)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "expected a string to set the index comment: ALTER INDEX [IF EXISTS] name SET COMMENT [=] <comment>.";
                    throw invalid_token(msg.str());
                }

                // TODO: set comment
                //n->get_string();

                expect_semi_colon("ALTER INDEX ...;");
                return;
            }
        }
        break;

    }

    if(add_drop != add_drop_t::ADD_DROP_NONE)
    {
        n = f_lexer->get_next_token();
        if(n->get_token() == token_t::TOKEN_IDENTIFIER)
        {
            cmd = n->get_string_upper();
            if(snapdev::compare_switch_string<"COLUMN">(cmd))
            {
                n = f_lexer->get_next_token();
                if(n->get_token() == token_t::TOKEN_IDENTIFIER)
                {
                    // TODO: implement the ADD|DROP COLUMN <name>
                    return;
                }
                else if(n->get_token() == token_t::TOKEN_INTEGER
                     && add_drop == add_drop_t::ADD_DROP_DROP)
                {
                    // TODO: implement the DROP COLUMN <position>
                    return;
                }
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location();
                if(add_drop == add_drop_t::ADD_DROP_DROP)
                {
                    msg << "expected the name or position of a column after the DROP COLUMN of an ALTER INDEX [IF EXISTS] name DROP COLUMN <column-name>.";
                }
                else
                {
                    msg << "expected the name of a column after the ADD COLUMN of an ALTER INDEX [IF EXISTS] name ADD COLUMN <column-name>.";
                }
                throw invalid_token(msg.str());
            }
            else if(snapdev::compare_switch_string<"EXPRESSION">(cmd))
            {
                // TODO: implement ADD|DROP COLUMN <name>
                return;
            }
        }
        if(n->get_token() == token_t::TOKEN_OPEN_PARENTHESIS
        && add_drop == add_drop_t::ADD_DROP_ADD)
        {
            // TODO: read expression, here we expect an as2js expression
            //       I'm thinking of using the as2js lexer to read the data
            //       and get as2js tokens until one extra ')'
            // TODO: make sure command ends with ')'
            // TODO: implement ADD EXPRESSION ( ... )
            return;
        }
        else if(n->get_token() == token_t::TOKEN_INTEGER
             && add_drop == add_drop_t::ADD_DROP_DROP)
        {
            // TODO: implement the DROP [COLUMN | EXPRESSION] <position>
            return;
        }
        else
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
            msg << n->get_location().get_location()
                << "unexpected token after ALTER INDEX [IF EXISTS] name ADD/DROP ....";
            throw invalid_token(msg.str());
        }
    }

    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
    msg << n->get_location().get_location()
        << "unknown index action \""
        << cmd
        << "\" after ALTER INDEX [IF EXISTS] name <action>.";
    throw invalid_token(msg.str());
}


void parser::parse_alter_table()
{
    throw not_yet_implemented("parser::parse_alter_table()");
}


void parser::parse_alter_type()
{
    throw not_yet_implemented("parser::parse_alter_type()");
}


void parser::parse_transaction_command(std::string const & cmd_name, command_t cmd_type)
{
    transaction_t transaction_type(transaction_t::TRANSACTION_UNDEFINED);
    node::pointer_t n(f_lexer->get_next_token());
    if(n->get_token() == token_t::TOKEN_IDENTIFIER)
    {
        std::string keyword(n->get_string_upper());
        if(snapdev::compare_switch_string<"WORK">(keyword)
        || snapdev::compare_switch_string<"TRANSACTION">(keyword))
        {
            n = f_lexer->get_next_token();
        }
    }
    if(n->get_token() == token_t::TOKEN_IDENTIFIER)
    {
        bool schema_data_required(false);
        std::string keyword(n->get_string_upper());
        if(snapdev::compare_switch_string<"ON">(keyword))
        {
            schema_data_required = true;

            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_IDENTIFIER)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "expected identifier SCHEMA or DATA after BEGIN ON.";
                throw invalid_token(msg.str());
            }
            keyword = n->get_string_upper();
        }
        if(snapdev::compare_switch_string<"SCHEMA">(keyword))
        {
            transaction_type = transaction_t::TRANSACTION_SCHEMA;
            n = f_lexer->get_next_token();
        }
        else if(snapdev::compare_switch_string<"DATA">(keyword))
        {
            transaction_type = transaction_t::TRANSACTION_DATA;
            n = f_lexer->get_next_token();
        }
        else if(schema_data_required)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
            msg << n->get_location().get_location()
                << "expected identifier SCHEMA or DATA after BEGIN ON.";
            throw invalid_token(msg.str());
        }
    }

    std::string expr;
    if(cmd_type != command_t::COMMAND_BEGIN
    && n->get_token() == token_t::TOKEN_IDENTIFIER)
    {
        if(snapdev::compare_switch_string<"IF">(n->get_string_upper()))
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
            msg << n->get_location().get_location()
                << "expected IF clause or ';' at the end of a COMMIT or ROLLBACK.";
            throw invalid_token(msg.str());
        }
        n = f_lexer->get_next_token();
        expr = parse_expression(n);
        if(n->get_token() == token_t::TOKEN_IDENTIFIER)
        {
            if(snapdev::compare_switch_string<"OTHERWISE">(n->get_string_upper()))
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "expected OTHERWISE after the IF expression of COMMIT or ROLLBACK.";
                throw invalid_token(msg.str());
            }
            n = f_lexer->get_next_token();

            char const * expects(cmd_type == command_t::COMMAND_COMMIT
                                            ? "ROLLBACK"
                                            : "COMMIT");
            if(n->get_token() != token_t::TOKEN_IDENTIFIER
            || n->get_string_upper() != expects)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "expected "
                    << expects
                    << " after OTHERWISE for command "
                    << cmd_name
                    << ".";
                throw invalid_token(msg.str());
            }
            n = f_lexer->get_next_token();
        }
    }

    expect_semi_colon(cmd_name, n);

    command::pointer_t cmd(std::make_shared<command>(cmd_type));
    cmd->set_int64(param_t::PARAM_TYPE, static_cast<std::int64_t>(transaction_type));
    if(!expr.empty())
    {
        cmd->set_string(param_t::PARAM_CONDITION, expr);
    }

    std::size_t idx(f_commands.size());
    if(cmd_type == command_t::COMMAND_BEGIN)
    {
        while(idx > 0)
        {
            --idx;
            if(f_commands[idx]->get_command() == command_t::COMMAND_COMMIT
            || f_commands[idx]->get_command() == command_t::COMMAND_ROLLBACK)
            {
                break;
            }
            if(f_commands[idx]->get_command() == command_t::COMMAND_BEGIN)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "work transactions cannot be nested (a BEGIN must first end with a COMMIT or ROLLBACK before another BEGIN is used).";
                throw invalid_entity(msg.str());
            }
        }
    }
    else
    {
        bool found(false);
        while(idx > 0)
        {
            --idx;
            if(f_commands[idx]->get_command() == command_t::COMMAND_COMMIT
            || f_commands[idx]->get_command() == command_t::COMMAND_ROLLBACK)
            {
                break;
            }
            if(f_commands[idx]->get_command() == command_t::COMMAND_BEGIN)
            {
                if(transaction_type == transaction_t::TRANSACTION_UNDEFINED)
                {
                    // mark the COMMIT or ROLLBACK with the BEGIN type
                    //
                    cmd->set_int64(param_t::PARAM_TYPE, f_commands[idx]->get_int64(param_t::PARAM_TYPE));
                }
                else
                {
                    if(transaction_type != static_cast<transaction_t>(f_commands[idx]->get_int64(param_t::PARAM_TYPE)))
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                        msg << n->get_location().get_location()
                            << "transaction type mismatch between BEGIN and "
                            << cmd_name
                            << ".";
                        throw invalid_type(msg.str());
                    }
                }
                found = true;
                break;
            }
        }
        if(!found)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
            msg << n->get_location().get_location()
                << "found a dangling "
                << cmd_name
                << " (i.e. without a prior BEGIN).";
            throw invalid_entity(msg.str());
        }
    }

    f_commands.push_back(cmd);
}


void parser::parse_create_context()
{
    node::pointer_t n(f_lexer->get_next_token());
    if(n->get_token() != token_t::TOKEN_IDENTIFIER)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
        msg << n->get_location().get_location()
            << "expected an identifier after CREATE CONTEXT.";
        throw invalid_token(msg.str());
    }

    command::pointer_t cmd(std::make_shared<command>(command_t::COMMAND_CREATE_CONTEXT));

    std::string keyword(n->get_string_upper());
    bool const if_not_exists(snapdev::compare_switch_string<"IF">(keyword));
    if(if_not_exists)
    {
        bool optional_found(false);
        n = parser::keyword_string(
                  "CREATE CONTEXT IF"
                , { "NOT", "EXISTS" }
                , optional_found
                , token_t::TOKEN_IDENTIFIER);
    }
    cmd->set_bool(param_t::PARAM_IF_EXISTS, !if_not_exists); // i.e. set IF_EXISTS to false when IF NOT EXISTS is defined

    n = get_full_name(cmd, param_t::PARAM_NAME, param_t::PARAM_NAME_end, n);
    //std::string const context_name(n->get_string_lower());
    //if(!validate_name(context_name.c_str()))
    //{
    //    // LCOV_EXCL_START
    //    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
    //    msg << n->get_location().get_location()
    //        << "context name \""
    //        << context_name
    //        << "\" is not considered valid.";
    //    throw invalid_token(msg.str());
    //    // LCOV_EXCL_STOP
    //}
    //cmd->set_string(param_t::PARAM_NAME, context_name);

    std::string context_path;
    //std::string owner;
    //std::string group;
    std::string description;
    for(;;)
    {
        n = f_lexer->get_next_token();
        if(n->get_token() != token_t::TOKEN_IDENTIFIER)
        {
            break;
        }

        keyword = n->get_string_upper();
        if(snapdev::compare_switch_string<"USING">(keyword))
        {
            if(!context_path.empty())
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "USING keyword found twice after CREATE CONTEXT.";
                throw invalid_token(msg.str());
            }
            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_STRING)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "expected a path after the USING keyword of CREATE CONTEXT.";
                throw invalid_token(msg.str());
            }
            context_path = n->get_string_lower();
            if(context_path.empty())
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "expected a non-empty path after the USING keyword of CREATE CONTEXT.";
                throw invalid_token(msg.str());
            }
            std::vector<std::string> segments;
            snapdev::tokenize_string(segments, context_path, "/", true);
            if(segments.size() >= MAX_CONTEXT_NAME_SEGMENTS) // here we use >= because the USING path does not include the context name
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "expected a maximum number of "
                    << MAX_CONTEXT_NAME_SEGMENTS - 1
                    << " segments in the context path defined by the USING keyword of CREATE CONTEXT.";
                throw invalid_token(msg.str());
            }
        }
        else if(snapdev::compare_switch_string<"WITH">(keyword))
        {
            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_OPEN_PARENTHESIS)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "WITH feature definitions must be defined between parenthesis, '(' missing in CREATE CONTEXT.";
                throw invalid_token(msg.str());
            }

            for(;;)
            {
                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_IDENTIFIER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "WITH feature definitions must be named using an identifier in CREATE CONTEXT.";
                    throw invalid_token(msg.str());
                }

                keyword = n->get_string_upper();

                // the keyword can optionally be followed by an equal character
                //
                n = f_lexer->get_next_token();
                if(n->get_token() == token_t::TOKEN_EQUAL)
                {
                    // skip optional equal (=)
                    //
                    n = f_lexer->get_next_token();
                }

                //if(keyword == "OWNER")
                //{
                //    if(!owner.empty())
                //    {
                //        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                //        msg << n->get_location().get_location()
                //            << "WITH OWNER found twice after CREATE CONTEXT.";
                //        throw invalid_token(msg.str());
                //    }
                //
                //    if(n->get_token() == token_t::TOKEN_STRING)
                //    {
                //        owner = n->get_string();
                //
                //        std::string::size_type const pos(owner.find(':'));
                //        if(pos != std::string::npos)
                //        {
                //            group = owner.substr(pos + 1);
                //            owner = owner.substr(0, pos);
                //        }
                //        //  else -- only the owner was specified
                //
                //        n = f_lexer->get_next_token();
                //    }
                //    else if(n->get_token() == token_t::TOKEN_IDENTIFIER
                //         || n->get_token() == token_t::TOKEN_INTEGER)
                //    {
                //        if(n->get_token() == token_t::TOKEN_IDENTIFIER)
                //        {
                //            owner = n->get_string();
                //        }
                //        else
                //        {
                //            owner = to_string(n->get_integer());
                //        }
                //
                //        n = f_lexer->get_next_token();
                //        if(n->get_token() == token_t::TOKEN_COLON)
                //        {
                //            n = f_lexer->get_next_token();
                //            if(n->get_token() == token_t::TOKEN_IDENTIFIER)
                //            {
                //                group = n->get_string();
                //            }
                //            else if(n->get_token() == token_t::TOKEN_INTEGER)
                //            {
                //                group = to_string(n->get_integer());
                //            }
                //            else
                //            {
                //                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                //                msg << n->get_location().get_location()
                //                    << "expected a group name after ':' in CREATE CONTEXT ... WITH ( OWNER <user>:<group> ), not a "
                //                    << to_string(n->get_token())
                //                    << ".";
                //                throw invalid_token(msg.str());
                //            }
                //
                //            n = f_lexer->get_next_token();
                //        }
                //    }
                //    else
                //    {
                //        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                //        msg << n->get_location().get_location()
                //            << "expected a string or an identifier after WITH ( OWNER <owner>[:<group>] ).";
                //        throw invalid_token(msg.str());
                //    }
                //}
                //else
                if(snapdev::compare_switch_string<"COMMENT">(keyword))
                {
                    if(!description.empty())
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                        msg << n->get_location().get_location()
                            << "WITH COMMENT found twice after CREATE CONTEXT.";
                        throw invalid_token(msg.str());
                    }

                    if(n->get_token() != token_t::TOKEN_STRING)
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                        msg << n->get_location().get_location()
                            << "expected a string for <description> in CREATE CONTEXT ... WITH ( COMMENT <description> ) got a "
                            << to_string(n->get_token())
                            << " instead.";
                        throw invalid_token(msg.str());
                    }
                    description = n->get_string();

                    n = f_lexer->get_next_token();
                }

                if(n->get_token() == token_t::TOKEN_CLOSE_PARENTHESIS)
                {
                    break;
                }

                if(n->get_token() != token_t::TOKEN_COMMA)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "expected a comma to separate feature definitions in CREATE CONTEXT.";
                    throw invalid_token(msg.str());
                }
            }
        }
    }

    expect_semi_colon("CREATE CONTEXT", n);

    // name is the last segment, so we have up to 3 names in the path plus
    // the name of the context and that's where the files get saved
    //if(context_path.empty())
    //{
    //    context_path = context_name;    // path defaults to name if not defined by user
    //}

    cmd->set_string(param_t::PARAM_PATH, context_path);
    //cmd->set_string(param_t::PARAM_USER, owner);
    //cmd->set_string(param_t::PARAM_GROUP, group);
    cmd->set_string(param_t::PARAM_DESCRIPTION, description);

    f_commands.push_back(cmd);
}


void parser::parse_create_index()
{
    throw not_yet_implemented("parser::parse_create_index()");
}


void parser::parse_create_table()
{
    throw not_yet_implemented("parser::parse_create_table()");
}


void parser::parse_create_type()
{
    throw not_yet_implemented("parser::parse_create_type()");
}


void parser::parse_select()
{
    command::pointer_t cmd(std::make_shared<command>(command_t::COMMAND_SELECT));

    node::pointer_t n(f_lexer->get_next_token());
    int count(0);
    for(;; ++count)
    {
        // SELECT DEFAULT VALUES ...
        //
        if(n->get_token() == token_t::TOKEN_IDENTIFIER
        && snapdev::compare_switch_string<"DEFAULT">(n->get_string_upper()))
        {
            if(cmd->is_defined_as(param_t::PARAM_EXPRESSION) == param_type_t::PARAM_TYPE_STRING)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "SELECT DEFAULT VALUES cannot be used with other fields.";
                throw invalid_token(msg.str());
            }

            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_IDENTIFIER
            && snapdev::compare_switch_string<"VALUES">(n->get_string_upper()))
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "SELECT DEFAULT is expected to be followed by VALUES.";
                throw invalid_token(msg.str());
            }

            n = f_lexer->get_next_token();
            break;
        }

        if(count >= MAX_EXPRESSIONS)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
            msg << n->get_location().get_location()
                << "SELECT can be followed by at most "
                << MAX_EXPRESSIONS
                << " expressions.";
            throw invalid_token(msg.str());
        }

        // SELECT <expr>
        //
        std::string const expr(parse_expression(n));
        cmd->set_string(param_t::PARAM_EXPRESSION + count, expr);

        // SELECT <expr> AS <name>
        //
        if(n->get_token() == token_t::TOKEN_IDENTIFIER
        && snapdev::compare_switch_string<"AS">(n->get_string_upper()))
        {
            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_IDENTIFIER)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "SELECT <expression> AS ... is expected to be followed by a name (an identifier).";
                throw invalid_token(msg.str());
            }
            cmd->set_string(param_t::PARAM_COLUMN_NAME + count, n->get_string_lower());

            n = f_lexer->get_next_token();
        }
        else
        {
            std::string name("__col");
            name += std::to_string(count + 1);
            cmd->set_string(param_t::PARAM_COLUMN_NAME + count, name);
        }

        if(n->get_token() != token_t::TOKEN_COMMA)
        {
SNAP_LOG_WARNING << "--- done parsing SELECT expressions..." << SNAP_LOG_SEND;
            break;
        }
        n = f_lexer->get_next_token();
    }

    // SELECT can be used to compute expressions and that's it, so the
    // FROM and following are all optional here
    //
    if(n->get_token() == token_t::TOKEN_IDENTIFIER
    && snapdev::compare_switch_string<"FROM">(n->get_string_upper()))
    {
        n = f_lexer->get_next_token();
        count = 0;
        for(;; ++count)
        {
            if(count >= MAX_TABLES)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "SELECT ... FROM can be followed by at most "
                    << MAX_TABLES
                    << " table names.";
                throw invalid_token(msg.str());
            }

            if(n->get_token() != token_t::TOKEN_IDENTIFIER)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "SELECT ... FROM <table-name> is expected to be the name of a table (an identifier).";
                throw invalid_token(msg.str());
            }
            // TODO: check that the identifier is not a keyword (WHERE, ORDER, LIMIT...)
            //
            cmd->set_string(param_t::PARAM_TABLE + count, n->get_string());

            // ... FROM <table-name> AS <name>
            //
            n = f_lexer->get_next_token();
            if(n->get_token() == token_t::TOKEN_IDENTIFIER
            && snapdev::compare_switch_string<"AS">(n->get_string_upper()))
            {
                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_IDENTIFIER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "SELECT ... FROM <table-name> AS ... is expected to be followed by a name (an identifier).";
                    throw invalid_token(msg.str());
                }
                cmd->set_string(param_t::PARAM_TABLE_NAME + count, n->get_string());

                n = f_lexer->get_next_token();
            }

            if(n->get_token() != token_t::TOKEN_COMMA)
            {
                break;
            }
        }

        std::string where;
        std::string order_by;
        std::int64_t limit(0);
        while(n->get_token() == token_t::TOKEN_IDENTIFIER)
        {
            std::string const keyword(n->get_string_upper());
            if(snapdev::compare_switch_string<"WHERE">(keyword))
            {
                // WHERE <expr>
                //
                if(!where.empty())
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "SELECT ... WHERE ... found twice.";
                    throw invalid_token(msg.str());
                }
                n = f_lexer->get_next_token();
                where = parse_expression(n);
                cmd->set_string(param_t::PARAM_WHERE, where);
            }
            else if(snapdev::compare_switch_string<"ORDER">(keyword))
            {
                // ORDER BY PRIMARY KEY
                //   or
                // ORDER BY <index-name>
                //
                if(!order_by.empty())
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "SELECT ... ORDER BY ... found twice.";
                    throw invalid_token(msg.str());
                }
                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_IDENTIFIER
                || !snapdev::compare_switch_string<"BY">(n->get_string_upper()))
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "SELECT ... ORDER ... is expected to be followed by the 'BY' keyword.";
                    throw invalid_token(msg.str());
                }

                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_IDENTIFIER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "SELECT ... ORDER BY ... is expected to be the name of an index or 'PRIMARY KEY'.";
                    throw invalid_token(msg.str());
                }
                if(snapdev::compare_switch_string<"PRIMARY">(n->get_string_upper()))
                {
                    n = f_lexer->get_next_token();
                    if(n->get_token() != token_t::TOKEN_IDENTIFIER
                    || !snapdev::compare_switch_string<"KEY">(n->get_string_upper()))
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                        msg << n->get_location().get_location()
                            << "SELECT ... ORDER BY PRIMARY ... is expected to be followed by the 'KEY' keyword.";
                        throw invalid_token(msg.str());
                    }

                    // TODO: determine the correct index name for the primary key
                    //
                    order_by = "primary_key";
                }
                else
                {
                    order_by = n->get_string_lower();
                }
                cmd->set_string(param_t::PARAM_ORDER_BY, order_by);
            }
            else if(snapdev::compare_switch_string<"LIMIT">(keyword))
            {
                // LIMIT <integer>
                //
                if(limit != 0)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "SELECT ... LIMIT ... found twice.";
                    throw invalid_token(msg.str());
                }
                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_INTEGER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "SELECT ... LIMIT ... is expected to be followed by an integer.";
                    throw invalid_token(msg.str());
                }
                limit = n->get_integer().f_value[0];
                if(limit <= 0 || limit > MAX_LIMIT)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "SELECT ... LIMIT "
                        << limit
                        << " is out of range: (0, "
                        << MAX_LIMIT
                        << "].";
                    throw invalid_token(msg.str());
                }
                cmd->set_int64(param_t::PARAM_LIMIT, limit);
            }
            else
            {
                break;
            }
        }
    }

    if(n->get_token() == token_t::TOKEN_IDENTIFIER)
    {
        std::string const keyword(n->get_string_upper());
        if(snapdev::compare_switch_string<"OUTPUT">(keyword))
        {
            // OUTPUT <mode>
            //
            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_IDENTIFIER)
            {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "SELECT ... OUTPUT ... is expected to be followed by a mode ('TABLE', 'PRINT', or 'JSON').";
                    throw invalid_token(msg.str());
            }
            std::string const mode(n->get_string_upper());
            if(snapdev::compare_switch_string<"TABLE">(mode))
            {
                cmd->set_int64(param_t::PARAM_MODE, static_cast<std::int64_t>(param_mode_t::PARAM_MODE_TABLE));
            }
            else if(snapdev::compare_switch_string<"PRINT">(mode))
            {
                cmd->set_int64(param_t::PARAM_MODE, static_cast<std::int64_t>(param_mode_t::PARAM_MODE_PRINT));
            }
            else if(snapdev::compare_switch_string<"JSON">(mode))
            {
                cmd->set_int64(param_t::PARAM_MODE, static_cast<std::int64_t>(param_mode_t::PARAM_MODE_JSON));
            }
            else
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "SELECT ... OUTPUT "
                    << mode
                    << " is not a known mode (try with 'TABLE', 'PRINT', or 'JSON' instead).";
                throw invalid_token(msg.str());
            }
        }
    }

SNAP_LOG_WARNING << "--- check for SELECT ';'..." << SNAP_LOG_SEND;
if(n->get_token() == token_t::TOKEN_IDENTIFIER)
{
SNAP_LOG_WARNING << "identifier [" << n->get_string() << "] instead of ';' ?!" << SNAP_LOG_SEND;
}
    expect_semi_colon("SELECT", n);

    f_commands.push_back(cmd);
}


void parser::parse_config()
{
    // CONFIG <path>::<name> = <expression>
    //
    // all the parameters are optional, although if a <path> is defined, then
    // a <name> exists too and the equal sign is mandatory if you assign
    // an expression
    //
    command::pointer_t cmd(std::make_shared<command>(command_t::COMMAND_CONFIG));

    prinbee::pbql::node::pointer_t n(f_lexer->get_next_token());
    if(n->get_token() != token_t::TOKEN_SEMI_COLON)
    {
        if(n->get_token() == prinbee::pbql::token_t::TOKEN_STRING)
        {
            // regular expression case
            //
            std::string name(n->get_string());
            if(!name.empty())
            {
                if(name.front() != '/'
                || name.back() != '/')
                {
                    // clearly mark it as a regular expression
                    //
                    name = advgetopt::quote(name, '/');
                }
                cmd->set_string(param_t::PARAM_NAME, name);
            }
            // else -- the empty string means display all like /.*/ or just CONFIG;

            n = f_lexer->get_next_token();
        }
        else if(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER)
        {
            std::string name(n->get_string());

            n = f_lexer->get_next_token();
            if(n->get_token() == prinbee::pbql::token_t::TOKEN_SCOPE)
            {
                n = f_lexer->get_next_token();
                if(n->get_token() != prinbee::pbql::token_t::TOKEN_IDENTIFIER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                    msg << n->get_location().get_location()
                        << "expected a configuration parameter name after the scope (::), not token '"
                        << to_string(n->get_token())
                        << "'.";
                    throw prinbee::invalid_token(msg.str());
                }

                // the name above was the "path" (left handside of the scope operator)
                //
                cmd->set_string(param_t::PARAM_PATH, name);

                // the right handside is the actual name
                //
                name = n->get_string();

                // when we have two names separated by a scope token, we support
                // setting/getting parameters from any layer; i.e. the pbql::...,
                // proxy::..., or prinbee::... or something like that

                n = f_lexer->get_next_token();
            }
            cmd->set_string(param_t::PARAM_NAME, name);

            if(n->get_token() == prinbee::pbql::token_t::TOKEN_EQUAL)
            {
                n = f_lexer->get_next_token();
                cmd->set_string(param_t::PARAM_EXPRESSION, parse_expression(n));

                // parse_expression() should stop on ';' when valid
            }
        }
        else
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
            msg << n->get_location().get_location()
                << "expected a parameter name after CONFIG, not token '"
                << to_string(n->get_token())
                << "'.";
            throw prinbee::invalid_token(msg.str());
        }

        expect_semi_colon("CONFIG", n);
    }
    // else ...
    // CONFIG; by itself to list all existing parameters

    f_commands.push_back(cmd);
}


void parser::parse_set()
{
    throw not_yet_implemented("parser::parse_set()");
}


void parser::parse_show()
{
    node::pointer_t n(f_lexer->get_next_token());
    if(n->get_token() != token_t::TOKEN_IDENTIFIER)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
        msg << n->get_location().get_location()
            << "expected an identifier after SHOW.";
        throw invalid_token(msg.str());
    }

    std::string const what(n->get_string_upper());
    if(snapdev::compare_switch_string<"CONTEXT">(what))
    {
        command::pointer_t cmd(std::make_shared<command>(command_t::COMMAND_SHOW_CONTEXT));
        n = get_full_name(cmd, param_t::PARAM_NAME, param_t::PARAM_NAME_end, nullptr);
        expect_semi_colon("SHOW CONTEXT", n);
        f_commands.push_back(cmd);
        return;
    }
    else if(snapdev::compare_switch_string<"CONTEXTS">(what))
    {
        command::pointer_t cmd(std::make_shared<command>(command_t::COMMAND_SHOW_CONTEXTS));
        expect_semi_colon("SHOW CONTEXTS", n);
        f_commands.push_back(cmd);
        return;
    }
    else if(snapdev::compare_switch_string<"INDEX">(what))
    {
        command::pointer_t cmd(std::make_shared<command>(command_t::COMMAND_SHOW_INDEX));
        expect_semi_colon("SHOW INDEX", n);
        f_commands.push_back(cmd);
        return;
    }
    else if(snapdev::compare_switch_string<"INDEXES">(what))
    {
        command::pointer_t cmd(std::make_shared<command>(command_t::COMMAND_SHOW_INDEXES));
        expect_semi_colon("SHOW INDEXES", n);
        f_commands.push_back(cmd);
        return;
    }
    else
    {
        // in case you misspell something
        //
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
        msg << n->get_location().get_location()
            << "expected a known name after SHOW, not '"
            << what
            << "'.";
        throw prinbee::invalid_token(msg.str());
    }
}


/** \brief Read the name of a context, table, field.
 *
 * This function can read the name of a context, table, or field. Those are
 * identifiers separated by periods. The context name can be 3 segments
 * forming a path, plus the name of the context itself. The table name
 * is just one name. A field name may be followed by a record field which
 * itself can include a sub-record field name, etc.
 *
 * \param[in,out] cmd  The command receiving the "table name".
 * \param[in] start  The first parameter receiving a name.
 * \param[in] end  The last possible parameter (inclusive).
 * \param[in] n  The node to start with or nullptr to retrieve another node.
 */
node::pointer_t parser::get_full_name(
      command::pointer_t cmd
    , param_t start
    , param_t end
    , node::pointer_t n)
{
    if(n == nullptr)
    {
        n = f_lexer->get_next_token();
    }
    for(; start <= end; ++start)
    {
        cmd->set_string(start, n->get_string_lower());
        n = f_lexer->get_next_token();
        if(n->get_token() != token_t::TOKEN_PERIOD)
        {
            break;
        }
        n = f_lexer->get_next_token();
        if(n->get_token() != token_t::TOKEN_IDENTIFIER)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
            msg << n->get_location().get_location()
                << "expected an identifier after the period '"
                << cmd
                << "' command; not "
                << to_string(n->get_token())
                << (n->get_token() == token_t::TOKEN_IDENTIFIER ? " " + n->get_string() : "")
                << ".";
            throw invalid_token(msg.str());
        }
    }

    return n;
}


void parser::expect_semi_colon(std::string const & cmd, node::pointer_t n)
{
    if(n == nullptr)
    {
        n = f_lexer->get_next_token();
    }
    if(n->get_token() != token_t::TOKEN_SEMI_COLON)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
        msg << n->get_location().get_location()
            << "expected ';' at the end of '"
            << cmd
            << "' command; not "
            << to_string(n->get_token())
            << (n->get_token() == token_t::TOKEN_IDENTIFIER ? " " + n->get_string() : "")
            << ".";
        throw invalid_token(msg.str());
    }
}


node::pointer_t parser::keyword_string(
      std::string commands
    , advgetopt::string_list_t const & keywords
    , bool & optional_found
    , token_t next_token_type)
{
    node::pointer_t n;
    optional_found = false;
    std::size_t const max(keywords.size());
    for(std::size_t idx(0); idx < max; ++idx)
    {
        std::string k(keywords[idx]);
#ifdef _DEBUG
        // LCOV_EXCL_START
        if(k.empty())
        {
            throw logic_error("keywords in keyword_string() cannot be empty words.");
        }
        // LCOV_EXCL_STOP
#endif
        bool const optional(k[0] == '?');
        if(optional)
        {
            k = k.substr(1);
#ifdef _DEBUG
            // LCOV_EXCL_START
            if(k.empty())
            {
                throw logic_error("keywords in keyword_string() cannot just be \"?\".");
            }
            // LCOV_EXCL_STOP
#endif
        }
        n = f_lexer->get_next_token();
        if(n->get_token() != token_t::TOKEN_IDENTIFIER)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
            msg << n->get_location().get_location()
                << "expected "
                << k
                << " identifier after "
                << commands
                << " not token '"
                << to_string(n->get_token())
                << "'.";
            throw invalid_token(msg.str());
        }
        std::string const cmd(n->get_string_upper());
        if(cmd != keywords[idx])
        {
            if(!optional)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
                msg << n->get_location().get_location()
                    << "expected the "
                    << k
                    << " identifier after "
                    << commands
                    << ", not \""
                    << cmd
                    << "\".";
                throw invalid_token(msg.str());
            }
        }
        else
        {
            if(optional)
            {
                optional_found = true;
            }
            commands += ' ';
            commands += cmd;
        }
    }

    n = f_lexer->get_next_token();
    if(next_token_type != token_t::TOKEN_UNKNOWN
    && n->get_token() != next_token_type)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_ERROR);
        msg << n->get_location().get_location()
            << "expected a "
            << to_string(next_token_type)
            << " after "
            << commands
            << ", not a "
            << to_string(n->get_token())
            << ".";
        throw invalid_token(msg.str());
    }

    return n;
}



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
