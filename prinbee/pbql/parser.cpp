// Copyright (c) 2024  Made to Order Software Corp.  All Rights Reserved
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

#include    "prinbee/data/schema.h"
#include    "prinbee/exception.h"


// libutf8
//
//#include    <libutf8/libutf8.h>


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/to_upper.h>
//#include    <snapdev/not_reached.h>
//#include    <snapdev/pathinfo.h>
//#include    <snapdev/stream_fd.h>
//#include    <snapdev/unique_number.h>


//// C
////
//#include    <linux/fs.h>
//#include    <sys/ioctl.h>


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


command::vector_t const & parser::parse()
{
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
                std::string command(n->get_string_upper());
                switch(command[0])
                {
                case 'A':
                    if(command == "ALTER")
                    {
                        // read one more identifier to know what is going to be altered
                        //
                        n = f_lexer->get_next_token();
                        if(n->get_token() == token_t::TOKEN_IDENTIFIER)
                        {
                            command = n->get_string_upper();
                            if(command == "INDEX")
                            {
                                parse_alter_index();
                                continue;
                            }
                            else if(command == "TABLE")
                            {
                                parse_alter_table();
                                continue;
                            }
                            else if(command == "TYPE")
                            {
                                parse_alter_type();
                                continue;
                            }
                            else
                            {
                                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                                msg << n->get_location().get_location()
                                    << "ALTER is expected to be followed by INDEX or TABLE, not \""
                                    << command
                                    << "\".";
                                throw invalid_token(msg.str());
                            }
                        }
                        else
                        {
                            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                            msg << n->get_location().get_location()
                                << "ALTER is expected to be followed by an identifier: INDEX or TABLE.";
                            throw invalid_token(msg.str());
                        }
                    }
                    break;

                case 'B':
                    if(command == "BEGIN")
                    {
                        parse_transaction_command(command, command_t::COMMAND_BEGIN);
                        continue;
                    }
                    if(command == "BYE")
                    {
                        expect_semi_colon(command);
                        return f_commands;
                    }
                    break;

                case 'C':
                    if(command == "COMMIT")
                    {
                        parse_transaction_command(command, command_t::COMMAND_COMMIT);
                        continue;
                    }
                    if(command == "CREATE")
                    {
                        // read one more identifier to know what is going to be created
                        //
                        n = f_lexer->get_next_token();
                        if(n->get_token() == token_t::TOKEN_IDENTIFIER)
                        {
                            command = n->get_string_upper();
                            if(command == "CONTEXT")
                            {
                                parse_create_context();
                                continue;
                            }
                            if(command == "INDEX")
                            {
                                parse_create_index();
                                continue;
                            }
                            else if(command == "TABLE")
                            {
                                parse_create_table();
                                continue;
                            }
                            else if(command == "TYPE")
                            {
                                parse_create_type();
                                continue;
                            }
                            else
                            {
                                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                                msg << n->get_location().get_location()
                                    << "CREATE is expected to be followed by: CONTEXT, INDEX, TABLE, TYPE, not \""
                                    << command
                                    << "\".";
                                throw invalid_token(msg.str());
                            }
                        }
                        else
                        {
                            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                            msg << n->get_location().get_location()
                                << "CREATE is expected to be followed by an identifier: CONTEXT, INDEX, TABLE, TYPE.";
                            throw invalid_token(msg.str());
                        }
                    }
                    break;

                case 'Q':
                    if(command == "QUIT")
                    {
                        expect_semi_colon(command);
                        return f_commands;
                    }
                    break;

                case 'R':
                    if(command == "ROLLBACK")
                    {
                        parse_transaction_command(command, command_t::COMMAND_ROLLBACK);
                        continue;
                    }
                    break;

                case 'S':
                    if(command == "SELECT")
                    {
                        parse_select();
                        continue;
                    }
                    break;

                }

                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "found unknown command \""
                    << command
                    << "\".";
                throw invalid_token(msg.str());
            }
            break;

        default:
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "expected the beginning of the line to start with an identifier representing a PBQL keyword.";
                throw invalid_token(msg.str());
            }

        }
    }
}


void parser::parse_alter_index()
{
    node::pointer_t n(f_lexer->get_next_token());
    if(n->get_token() != token_t::TOKEN_IDENTIFIER)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << n->get_location().get_location()
            << "expected an identifier after ALTER INDEX.";
        throw invalid_token(msg.str());
    }
    std::string command(n->get_string_upper());
    if(command == "IF")
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
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << n->get_location().get_location()
            << "expected an index action after ALTER INDEX [IF EXISTS] name <action>.";
        throw invalid_token(msg.str());
    }
    command = n->get_string_upper();
    enum add_drop_t
    {
        ADD_DROP_NONE,
        ADD_DROP_ADD,
        ADD_DROP_DROP,
    };
    add_drop_t add_drop(add_drop_t::ADD_DROP_NONE);
    switch(command[0])
    {
    case 'A':
        if(command == "ADD")
        {
            add_drop = add_drop_t::ADD_DROP_ADD;
        }
        break;

    case 'D':
        if(command == "DROP")
        {
            add_drop = add_drop_t::ADD_DROP_DROP;
        }
        break;

    case 'S':
        if(command == "SET")
        {
            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_IDENTIFIER)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "expected the name of a column after the ADD COLUMN of an ALTER INDEX [IF EXISTS] name ADD COLUMN <column-name>.";
                throw invalid_token(msg.str());
            }
            command = n->get_string_upper();
            bool negate(false);
            if(command == "NOT")
            {
                negate = true;
                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_IDENTIFIER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "expected an identifier after the SET NOT action of al ALTER INDEX [IF EXISTS] name SET NOT <sub-action>.";
                    throw invalid_token(msg.str());
                }
                command = n->get_string_upper();
            }
            if(command == "SECURE")
            {
                // TODO: set secure or NOT secure

                expect_semi_colon("ALTER INDEX ...;");
                return;
            }
            if(command == "SPARSE")
            {
                // TODO: set sparse or NOT sparse

                expect_semi_colon("ALTER INDEX ...;");
                return;
            }
            if(negate)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "unexpected NOT with the ALTER INDEX [IF EXISTS] name SET "
                    << command
                    << " action.";
                throw invalid_token(msg.str());
            }
            if(command == "MODEL")
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
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "expected an identifier with the model name after the SET MODEL action of al ALTER INDEX [IF EXISTS] name SET MODEL [=] <model>.";
                    throw invalid_token(msg.str());
                }
                model_t const model(name_to_model(n->get_string()));

                // TODO: set model in index
                snapdev::NOT_USED(model);

                expect_semi_colon("ALTER INDEX ...;");
                return;
            }
            if(command == "COMMENT")
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
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
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
            command = n->get_string_upper();
            if(command == "COLUMN")
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
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "expected the name of a column after the ADD COLUMN of an ALTER INDEX [IF EXISTS] name ADD COLUMN <column-name>.";
                throw invalid_token(msg.str());
            }
            else if(command != "EXPRESSION")
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
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << n->get_location().get_location()
                << "unexpected token after ALTER INDEX [IF EXISTS] name ADD/DROP ....";
            throw invalid_token(msg.str());
        }
    }

    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
    msg << n->get_location().get_location()
        << "unknown index action \""
        << command
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


void parser::parse_transaction_command(std::string const & cmd_name, command_t cmd)
{
    transaction_t transaction_type(transaction_t::TRANSACTION_UNDEFINED);
    node::pointer_t n(f_lexer->get_next_token());
    if(n->get_token() == token_t::TOKEN_IDENTIFIER)
    {
        std::string keyword(n->get_string_upper());
        if(keyword == "WORK"
        || keyword == "TRANSACTION")
        {
            n = f_lexer->get_next_token();
        }
    }
    if(n->get_token() == token_t::TOKEN_IDENTIFIER)
    {
        bool schema_data_required(false);
        std::string keyword(n->get_string_upper());
        if(keyword == "ON")
        {
            schema_data_required = true;

            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_IDENTIFIER)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "expected identifier SCHEMA or DATA after BEGIN ON.";
                throw invalid_token(msg.str());
            }
            keyword = n->get_string_upper();
        }
        if(keyword == "SCHEMA")
        {
            transaction_type = transaction_t::TRANSACTION_SCHEMA;
            n = f_lexer->get_next_token();
        }
        else if(keyword == "DATA")
        {
            transaction_type = transaction_t::TRANSACTION_DATA;
            n = f_lexer->get_next_token();
        }
        else if(schema_data_required)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << n->get_location().get_location()
                << "expected identifier SCHEMA or DATA after BEGIN ON.";
            throw invalid_token(msg.str());
        }
    }

    std::string expr;
    if(cmd != command_t::COMMAND_BEGIN
    && n->get_token() == token_t::TOKEN_IDENTIFIER)
    {
        if(n->get_string_upper() != "IF")
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << n->get_location().get_location()
                << "expected IF clause or ';' at the end of a COMMIT or ROLLBACK.";
            throw invalid_token(msg.str());
        }
        n = f_lexer->get_next_token();
        expr = parse_expression(n);
        if(n->get_token() == token_t::TOKEN_IDENTIFIER)
        {
            if(n->get_string_upper() != "OTHERWISE")
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "expected OTHERWISE after the IF expression of COMMIT or ROLLBACK.";
                throw invalid_token(msg.str());
            }
            n = f_lexer->get_next_token();

            char const * expects(cmd == command_t::COMMAND_COMMIT
                                            ? "ROLLBACK"
                                            : "COMMIT");
            if(n->get_token() != token_t::TOKEN_IDENTIFIER
            || n->get_string_upper() != expects)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
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

    command::pointer_t command(std::make_shared<command>(cmd));
    command->set_int64(param_t::PARAM_TYPE, static_cast<std::int64_t>(transaction_type));
    if(!expr.empty())
    {
        command->set_string(param_t::PARAM_CONDITION, expr);
    }

    std::size_t idx(f_commands.size());
    if(cmd == command_t::COMMAND_BEGIN)
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
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
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
                    command->set_int64(param_t::PARAM_TYPE, f_commands[idx]->get_int64(param_t::PARAM_TYPE));
                }
                else
                {
                    if(transaction_type != static_cast<transaction_t>(f_commands[idx]->get_int64(param_t::PARAM_TYPE)))
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
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
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << n->get_location().get_location()
                << "found a dangling "
                << cmd_name
                << " (i.e. without a prior BEGIN).";
            throw invalid_entity(msg.str());
        }
    }

    f_commands.push_back(command);
}


void parser::parse_create_context()
{
    node::pointer_t n(f_lexer->get_next_token());
    if(n->get_token() != token_t::TOKEN_IDENTIFIER)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << n->get_location().get_location()
            << "expected an identifier after CREATE CONTEXT.";
        throw invalid_token(msg.str());
    }

    command::pointer_t command(std::make_shared<command>(command_t::COMMAND_CREATE_CONTEXT));

    std::string keyword(n->get_string_upper());
    bool const if_not_exists(keyword == "IF");
    if(if_not_exists)
    {
        bool optional_found(false);
        n = parser::keyword_string(
                  "CREATE CONTEXT IF"
                , { "NOT", "EXISTS" }
                , optional_found
                , token_t::TOKEN_IDENTIFIER);
    }
    command->set_bool(param_t::PARAM_IF_EXISTS, !if_not_exists); // i.e. if IF_EXISTS is false, then IF NOT EXISTS was defined

    std::string const context_name(n->get_string_lower());
    if(!validate_name(context_name.c_str()))
    {
        // LCOV_EXCL_START
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << n->get_location().get_location()
            << "context name \""
            << context_name
            << "\" is not considered valid.";
        throw invalid_token(msg.str());
        // LCOV_EXCL_STOP
    }
    command->set_string(param_t::PARAM_NAME, context_name);

    std::string context_path;
    std::string owner;
    std::string group;
    std::string description;
    for(;;)
    {
        n = f_lexer->get_next_token();
        if(n->get_token() != token_t::TOKEN_IDENTIFIER)
        {
            break;
        }

        keyword = n->get_string_upper();
        if(keyword == "USING")
        {
            if(!context_path.empty())
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "USING keyword found twice after CREATE CONTEXT.";
                throw invalid_token(msg.str());
            }
            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_STRING)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "expected a path after the USING keyword of CREATE CONTEXT.";
                throw invalid_token(msg.str());
            }
            context_path = n->get_string_lower();
            if(context_path.empty())
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "expected a non-empty path after the USING keyword of CREATE CONTEXT.";
                throw invalid_token(msg.str());
            }
        }
        else if(keyword == "WITH")
        {
            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_OPEN_PARENTHESIS)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "WITH feature definitions must be defined between parenthesis, '(' missing in CREATE CONTEXT.";
                throw invalid_token(msg.str());
            }

            for(;;)
            {
                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_IDENTIFIER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
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

                if(keyword == "OWNER")
                {
                    if(!owner.empty())
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << n->get_location().get_location()
                            << "WITH OWNER found twice after CREATE CONTEXT.";
                        throw invalid_token(msg.str());
                    }

                    if(n->get_token() == token_t::TOKEN_STRING)
                    {
                        owner = n->get_string();

                        std::string::size_type const pos(owner.find(':'));
                        if(pos != std::string::npos)
                        {
                            group = owner.substr(pos + 1);
                            owner = owner.substr(0, pos);
                        }
                        //  else -- only the owner was specified

                        n = f_lexer->get_next_token();
                    }
                    else if(n->get_token() == token_t::TOKEN_IDENTIFIER
                         || n->get_token() == token_t::TOKEN_INTEGER)
                    {
                        if(n->get_token() == token_t::TOKEN_IDENTIFIER)
                        {
                            owner = n->get_string();
                        }
                        else
                        {
                            owner = to_string(n->get_integer());
                        }

                        n = f_lexer->get_next_token();
                        if(n->get_token() == token_t::TOKEN_COLON)
                        {
                            n = f_lexer->get_next_token();
                            if(n->get_token() == token_t::TOKEN_IDENTIFIER)
                            {
                                group = n->get_string();
                            }
                            else if(n->get_token() == token_t::TOKEN_INTEGER)
                            {
                                group = to_string(n->get_integer());
                            }
                            else
                            {
                                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                                msg << n->get_location().get_location()
                                    << "expected a group name after ':' in CREATE CONTEXT ... WITH ( OWNER <user>:<group> ), not a "
                                    << to_string(n->get_token())
                                    << ".";
                                throw invalid_token(msg.str());
                            }

                            n = f_lexer->get_next_token();
                        }
                    }
                    else
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << n->get_location().get_location()
                            << "expected a string or an identifier after WITH ( OWNER <owner>[:<group>] ).";
                        throw invalid_token(msg.str());
                    }
                }
                else if(keyword == "COMMENT")
                {
                    if(!description.empty())
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << n->get_location().get_location()
                            << "WITH COMMENT found twice after CREATE CONTEXT.";
                        throw invalid_token(msg.str());
                    }

                    if(n->get_token() != token_t::TOKEN_STRING)
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << n->get_location().get_location()
                            << "expected a string for <description> in CREATE CONTEXT ... WITH ( COMMENT <description> ) got a "
                            << to_string(n->get_token())
                            << ".";
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
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "expected a comma to separate feature definitions in CREATE CONTEXT.";
                    throw invalid_token(msg.str());
                }
            }
        }
    }

    expect_semi_colon("CREATE CONTEXT", n);

    if(context_path.empty())
    {
        context_path = context_name;    // path defaults to name if not defined by user
    }

    command->set_string(param_t::PARAM_PATH, context_path);
    command->set_string(param_t::PARAM_USER, owner);
    command->set_string(param_t::PARAM_GROUP, group);
    command->set_string(param_t::PARAM_DESCRIPTION, description);

    f_commands.push_back(command);
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
    command::pointer_t command(std::make_shared<command>(command_t::COMMAND_SELECT));

    node::pointer_t n(f_lexer->get_next_token());
    int count(0);
    for(;; ++count)
    {
        // SELECT DEFAULT VALUES ...
        //
        if(n->get_token() == token_t::TOKEN_IDENTIFIER
        && n->get_string_upper() == "DEFAULT")
        {
            if(command->is_defined_as(param_t::PARAM_EXPRESSION) == param_type_t::PARAM_TYPE_STRING)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "SELECT DEFAULT VALUES cannot be used with other fields.";
                throw invalid_token(msg.str());
            }

            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_IDENTIFIER
            && n->get_string_upper() != "VALUES")
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "SELECT DEFAULT is expected to be followed by VALUES.";
                throw invalid_token(msg.str());
            }

            n = f_lexer->get_next_token();
            break;
        }

        if(count >= MAX_EXPRESSIONS)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << n->get_location().get_location()
                << "SELECT can be followed by at most "
                << MAX_EXPRESSIONS
                << " expressions.";
            throw invalid_token(msg.str());
        }

        // SELECT <expr>
        //
        std::string const expr(parse_expression(n));
        command->set_string(static_cast<param_t>(static_cast<int>(param_t::PARAM_EXPRESSION) + count), expr);

        // SELECT <expr> AS <name>
        //
        if(n->get_token() == token_t::TOKEN_IDENTIFIER
        && n->get_string_upper() == "AS")
        {
            n = f_lexer->get_next_token();
            if(n->get_token() != token_t::TOKEN_IDENTIFIER)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "SELECT <expression> AS ... is expected to be followed by a name (an identifier).";
                throw invalid_token(msg.str());
            }
            command->set_string(static_cast<param_t>(static_cast<int>(param_t::PARAM_COLUMN_NAME) + count), n->get_string_lower());

            n = f_lexer->get_next_token();
        }
        else
        {
            std::string name("__col");
            name += std::to_string(count + 1);
            command->set_string(static_cast<param_t>(static_cast<int>(param_t::PARAM_COLUMN_NAME) + count), name);
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
    && n->get_string_upper() == "FROM")
    {
        n = f_lexer->get_next_token();
        count = 0;
        for(;; ++count)
        {
            if(count >= MAX_TABLES)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "SELECT ... FROM can be followed by at most "
                    << MAX_TABLES
                    << " table names.";
                throw invalid_token(msg.str());
            }

            if(n->get_token() != token_t::TOKEN_IDENTIFIER)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << n->get_location().get_location()
                    << "SELECT ... FROM <table-name> is expected to be the name of a table (an identifier).";
                throw invalid_token(msg.str());
            }
            // TODO: check that the identifier is not a keyword (WHERE, ORDER, LIMIT...)
            //
            command->set_string(static_cast<param_t>(static_cast<int>(param_t::PARAM_TABLE) + count), n->get_string());

            // ... FROM <table-name> AS <name>
            //
            n = f_lexer->get_next_token();
            if(n->get_token() == token_t::TOKEN_IDENTIFIER
            && n->get_string_upper() == "AS")
            {
                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_IDENTIFIER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "SELECT ... FROM <table-name> AS ... is expected to be followed by a name (an identifier).";
                    throw invalid_token(msg.str());
                }
                command->set_string(static_cast<param_t>(static_cast<int>(param_t::PARAM_TABLE_NAME) + count), n->get_string());

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
            if(keyword == "WHERE")
            {
                // WHERE <expr>
                //
                if(!where.empty())
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "SELECT ... WHERE ... found twice.";
                    throw invalid_token(msg.str());
                }
                n = f_lexer->get_next_token();
                where = parse_expression(n);
                command->set_string(param_t::PARAM_WHERE, where);
            }
            else if(keyword == "ORDER")
            {
                // ORDER BY PRIMARY KEY
                //   or
                // ORDER BY <index-name>
                //
                if(!order_by.empty())
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "SELECT ... ORDER BY ... found twice.";
                    throw invalid_token(msg.str());
                }
                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_IDENTIFIER
                || n->get_string_upper() != "BY")
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "SELECT ... ORDER ... is expected to be followed by the 'BY' keyword.";
                    throw invalid_token(msg.str());
                }

                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_IDENTIFIER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "SELECT ... ORDER BY ... is expected to be the name of an index or 'PRIMARY KEY'.";
                    throw invalid_token(msg.str());
                }
                if(n->get_string_upper() == "PRIMARY")
                {
                    n = f_lexer->get_next_token();
                    if(n->get_token() != token_t::TOKEN_IDENTIFIER
                    || n->get_string_upper() != "KEY")
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
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
                command->set_string(param_t::PARAM_ORDER_BY, order_by);
            }
            else if(keyword == "LIMIT")
            {
                // LIMIT <integer>
                //
                if(limit != 0)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "SELECT ... LIMIT ... found twice.";
                    throw invalid_token(msg.str());
                }
                n = f_lexer->get_next_token();
                if(n->get_token() != token_t::TOKEN_INTEGER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "SELECT ... LIMIT ... is expected to be followed by an integer.";
                    throw invalid_token(msg.str());
                }
                limit = n->get_integer().f_value[0];
                if(limit <= 0 || limit > MAX_LIMIT)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "SELECT ... LIMIT "
                        << limit
                        << " is out of range: (0, 1,000,000].";
                    throw invalid_token(msg.str());
                }
                command->set_int64(param_t::PARAM_LIMIT, limit);
            }
            else
            {
                break;
            }
        }
    }

SNAP_LOG_WARNING << "--- check for SELECT ';'..." << SNAP_LOG_SEND;
if(n->get_token() == token_t::TOKEN_IDENTIFIER)
{
SNAP_LOG_WARNING << "identifier [" << n->get_string() << "] instead of ';' ?!" << SNAP_LOG_SEND;
}
    expect_semi_colon("SELECT", n);

    f_commands.push_back(command);
}


void parser::expect_semi_colon(std::string const & command, node::pointer_t n)
{
    if(n == nullptr)
    {
        n = f_lexer->get_next_token();
    }
    if(n->get_token() != token_t::TOKEN_SEMI_COLON)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << n->get_location().get_location()
            << "expected ';' at the end of '"
            << command
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
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
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
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
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
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
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
