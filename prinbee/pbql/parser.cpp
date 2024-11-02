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
}


void parser::run()
{
    for(;;)
    {
        node::pointer_t n(f_lexer->get_next_token());
        switch(n->get_token())
        {
        case token_t::TOKEN_EOF:
            // got to the end, we are done
            //
            return;

        case token_t::TOKEN_IDENTIFIER:
            {
                // select which function to call based on the identifier
                //
                std::string command(snapdev::to_upper(n->get_string()));
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
                            command = snapdev::to_upper(n->get_string());
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
                    if(command == "BYE")
                    {
                        // TODO: verify that is the only thing on this line
                        //
                        return;
                    }
                    break;

                case 'Q':
                    if(command == "QUIT")
                    {
                        // TODO: verify that is the only thing on this line
                        //
                        return;
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


void parser::expect_semi_colon(std::string const & command)
{
    node::pointer_t n(f_lexer->get_next_token());
    if(n->get_token() != token_t::TOKEN_SEMI_COLON)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << n->get_location().get_location()
            << "expected ';' at the end of '"
            << command
            << "' command.";
        throw invalid_token(msg.str());
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
    std::string command(snapdev::to_upper(n->get_string()));
    if(command == "IF")
    {
        n = f_lexer->get_next_token();
        if(n->get_token() != token_t::TOKEN_IDENTIFIER)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << n->get_location().get_location()
                << "expected EXISTS identifier after ALTER INDEX IF.";
            throw invalid_token(msg.str());
        }
        command = snapdev::to_upper(n->get_string());
        if(command != "EXISTS")
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << n->get_location().get_location()
                << "expected the EXISTS identifier after ALTER INDEX IF, not \""
                << command
                << "\".";
            throw invalid_token(msg.str());
        }
        n = f_lexer->get_next_token();
        if(n->get_token() != token_t::TOKEN_IDENTIFIER)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << n->get_location().get_location()
                << "expected an index name after ALTER INDEX IF EXISTS.";
            throw invalid_token(msg.str());
        }
    }

    std::string const name(n->get_string());

    n = f_lexer->get_next_token();
    if(n->get_token() != token_t::TOKEN_IDENTIFIER)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << n->get_location().get_location()
            << "expected an index action after ALTER INDEX [IF EXISTS] name <action>.";
        throw invalid_token(msg.str());
    }
    command = snapdev::to_upper(n->get_string());
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
            command = snapdev::to_upper(n->get_string());
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
                command = snapdev::to_upper(n->get_string());
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
            command = snapdev::to_upper(n->get_string());
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
}




} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
