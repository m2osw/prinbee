// Copyright (c) 2019-2025  Made to Order Software Corp.  All Rights Reserved
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

// prinbee
//
#include    <prinbee/pbql/parser.h>

#include    <prinbee/exception.h>


// self
//
#include    "catch_main.h"


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/not_reached.h>
#include    <snapdev/string_replace_many.h>
#include    <snapdev/to_lower.h>


// C++
//
//#include    <bitset>
//#include    <fstream>
//#include    <iomanip>


// C
//
//#include    <sys/stat.h>
//#include    <sys/types.h>


// last include
//
#include    <snapdev/poison.h>



namespace
{



std::string escape_quotes(std::string const & s)
{
    // double quotes which is the way to escape quotes in SQL
    //
    return snapdev::string_replace_many(s, {{"'", "''"}});
}


char const * optional_equal()
{
    switch(rand() % 5)
    {
    case 0:
        return " ";

    case 1:
        return "=";

    case 2:
        return " =";

    case 3:
        return "= ";

    case 4:
        return " = ";

    }
    snapdev::NOT_REACHED();
}



} // no name namespace



CATCH_TEST_CASE("parser", "[parser][pbql]")
{
    CATCH_START_SECTION("parser: begin + select + commit/rollback")
    {
        for(int state(0); state < 4; ++state) // COMMIT / COMMIT + IF / ROLLBACK / ROLLBACK + IF
        {
            for(int work(0); work < 3; ++work)
            {
                for(int type(0); type < 7; ++type)
                {
                    // BEGIN
                    std::string script("BEGIN");

                    // WORK/TRANSACTION
                    if(work == 1)
                    {
                        script += " WORK";
                    }
                    else if(work == 2)
                    {
                        script += " TRANSACTION";
                    }

                    // ON
                    if(type >= 3)
                    {
                        script += " ON";
                    }

                    // SCHEMA/DATA
                    prinbee::pbql::transaction_t expected_transaction(prinbee::pbql::transaction_t::TRANSACTION_UNDEFINED);
                    if(type == 1 || type == 3 || type == 5)
                    {
                        script += " SCHEMA";
                        expected_transaction = prinbee::pbql::transaction_t::TRANSACTION_SCHEMA;
                    }
                    else if(type == 2 || type == 4 || type == 6)
                    {
                        script += " DATA";
                        expected_transaction = prinbee::pbql::transaction_t::TRANSACTION_DATA;
                    }

                    // no IF for a BEGIN, type 5 and 6 are not use here

                    script += ";\n";

                    // COMMIT or ROLLBACK
                    script += state < 2 ? "COMMIT" : "ROLLBACK";

                    // WORK/TRANSACTION (the test would be better if the COMMIT/ROLLBACK would use a different set of parameters than the BEGIN...)
                    if(work == 1)
                    {
                        script += " WORK";
                    }
                    else if(work == 2)
                    {
                        script += " TRANSACTION";
                    }

                    // ON
                    if(type >= 3)
                    {
                        script += " ON";
                    }

                    // SCHEMA/DATA
                    //prinbee::pbql::transaction_t expected_transaction(prinbee::pbql::transaction_t::TRANSACTION_UNDEFINED);
                    if(type == 1 || type == 3 || type == 5)
                    {
                        script += " SCHEMA";
                        //expected_transaction = prinbee::pbql::transaction_t::TRANSACTION_SCHEMA;
                    }
                    else if(type == 2 || type == 4 || type == 6)
                    {
                        script += " DATA";
                        //expected_transaction = prinbee::pbql::transaction_t::TRANSACTION_DATA;
                    }

                    // IF <condition>
                    if(type == 5 || type == 6)
                    {
                        script += " IF a > b";
                        if(type == 6)
                        {
                            script += " OTHERWISE ";
                            script += state >= 2 ? "COMMIT" : "ROLLBACK"; // test flip from above
                        }
                    }

                    script += ";\n";

SNAP_LOG_WARNING << "script [" << script << "]" << SNAP_LOG_SEND;

                    prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
                    lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "begin-test.pbql"));
                    prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
                    prinbee::pbql::command::vector_t const & commands(parser->parse());

                    CATCH_REQUIRE(commands.size() == 2);

                    // BEGIN
                    CATCH_REQUIRE(commands[0]->get_command() == prinbee::pbql::command_t::COMMAND_BEGIN);
                    // SCHEMA/DATA
                    CATCH_REQUIRE(commands[0]->is_defined_as(prinbee::pbql::param_t::PARAM_TYPE) == prinbee::pbql::param_type_t::PARAM_TYPE_INT64);
                    CATCH_REQUIRE(commands[0]->get_int64(prinbee::pbql::param_t::PARAM_TYPE) == static_cast<std::int64_t>(expected_transaction));

                    // COMMIT / ROLLBACK
                    CATCH_REQUIRE(commands[1]->get_command() == (state < 2 ? prinbee::pbql::command_t::COMMAND_COMMIT : prinbee::pbql::command_t::COMMAND_ROLLBACK));
                    // SCHEMA/DATA
                    CATCH_REQUIRE(commands[1]->is_defined_as(prinbee::pbql::param_t::PARAM_TYPE) == prinbee::pbql::param_type_t::PARAM_TYPE_INT64);
                    CATCH_REQUIRE(commands[1]->get_int64(prinbee::pbql::param_t::PARAM_TYPE) == static_cast<std::int64_t>(expected_transaction));
                    if(type == 5 || type == 6)
                    {
                        CATCH_REQUIRE(commands[1]->is_defined_as(prinbee::pbql::param_t::PARAM_CONDITION) == prinbee::pbql::param_type_t::PARAM_TYPE_STRING);
                        CATCH_REQUIRE(commands[1]->get_string(prinbee::pbql::param_t::PARAM_CONDITION) == "a>b");
                    }
                    else
                    {
                        CATCH_REQUIRE(commands[1]->is_defined_as(prinbee::pbql::param_t::PARAM_CONDITION) == prinbee::pbql::param_type_t::PARAM_TYPE_UNKNOWN);
                    }
                }
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("parser: create context")
    {
        int counter(1);
        for(int exists(0); exists < 2; ++exists)
        {
            for(int using_path(0); using_path < 4; ++using_path)
            {
                for(int owner(0); owner < 7; ++owner)
                {
                    for(int comment(0); comment < 3; ++comment)
                    {
                        // CREATE CONTEXT
                        std::string script("CREATE CONTEXT ");

                        // IF NOT EXISTS
                        if(exists != 0)
                        {
                            script += "IF NOT EXISTS ";
                        }

                        // <context-name>
                        std::string context_name(SNAP_CATCH2_NAMESPACE::random_string(1, 97, SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_LABEL));
                        context_name += '_';
                        context_name += std::to_string(counter);
                        script += context_name;

                        // USING '<context-path>'
                        std::string context_path;
                        for(int segment(0); segment < using_path; ++segment)
                        {
                            if(!context_path.empty())
                            {
                                context_path += '/';
                            }
                            context_path += SNAP_CATCH2_NAMESPACE::random_string(1, 100, SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_LABEL);
                        }
                        if(!context_path.empty())
                        {
                            script += " USING '";
                            script += context_path;
                            script += '\'';
                        }

                        // WITH ( ... )
                        std::string ownership;
                        std::string group_member;
                        std::string description;
                        if(owner != 0 || comment != 0)
                        {
                            script += " WITH (";
                            bool quoted_owner(rand() & 1);
                            int order((rand() & 1) + 1);
                            char const * sep("");
                            for(int with(0); with < 2; ++with, order ^= 3)
                            {
                                if((order & 1) != 0)
                                {
                                    if(owner != 0)
                                    {
                                        // WITH ( OWNER )
                                        script += sep;
                                        script += "OWNER";
                                        script += optional_equal();
                                        if(quoted_owner)
                                        {
                                            script += '\'';
                                        }
                                        if((owner & 1) == 0)
                                        {
                                            ownership = SNAP_CATCH2_NAMESPACE::random_string(1, 32, SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_LABEL);
                                        }
                                        else
                                        {
                                            ownership = std::to_string(rand() & 0x7fff);
                                        }
                                        script += ownership;
                                        if(owner < 5)
                                        {
                                            if((owner & 2) == 0)
                                            {
                                                group_member = SNAP_CATCH2_NAMESPACE::random_string(1, 32, SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_LABEL);
                                            }
                                            else
                                            {
                                                group_member = std::to_string(rand() & 0x7fff);
                                            }
                                            script += ':';
                                            script += group_member;
                                        }
                                        if(quoted_owner)
                                        {
                                            script += '\'';
                                        }
                                        sep = rand() & 1 ? ", " : ",";
                                    }
                                }
                                if((order & 2) != 0)
                                {
                                    if(comment != 0)
                                    {
                                        // WITH ( COMMENT )
                                        description = SNAP_CATCH2_NAMESPACE::random_string(1, 500);
                                        script += sep;
                                        script += "COMMENT";
                                        script += optional_equal();
                                        script += '\'';
                                        script += escape_quotes(description);
                                        script += '\'';
                                        sep = rand() & 1 ? ", " : ",";
                                    }
                                }
                            }
                            script += ')';
                        }
                        script += ';';

//SNAP_LOG_WARNING << "script [" << script << "]" << SNAP_LOG_SEND;

                        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
                        lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
                        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
                        prinbee::pbql::command::vector_t const & commands(parser->parse());

                        CATCH_REQUIRE(commands.size() == 1);
                        // CREATE CONTEXT
                        CATCH_REQUIRE(commands[0]->get_command() == prinbee::pbql::command_t::COMMAND_CREATE_CONTEXT);
                        // [IF NOT EXISTS]
                        CATCH_REQUIRE(commands[0]->is_defined_as(prinbee::pbql::param_t::PARAM_IF_EXISTS) == prinbee::pbql::param_type_t::PARAM_TYPE_BOOL);
                        CATCH_REQUIRE(commands[0]->get_bool(prinbee::pbql::param_t::PARAM_IF_EXISTS) == (exists == 0));
                        // <context-name>
                        CATCH_REQUIRE(commands[0]->is_defined_as(prinbee::pbql::param_t::PARAM_NAME) == prinbee::pbql::param_type_t::PARAM_TYPE_STRING);
                        CATCH_REQUIRE(commands[0]->get_string(prinbee::pbql::param_t::PARAM_NAME) == snapdev::to_lower(context_name));
                        // [USING <context-path>]
                        CATCH_REQUIRE(commands[0]->is_defined_as(prinbee::pbql::param_t::PARAM_PATH) == prinbee::pbql::param_type_t::PARAM_TYPE_STRING);
                        if(context_path.empty())
                        {
                            CATCH_REQUIRE(commands[0]->get_string(prinbee::pbql::param_t::PARAM_PATH) == snapdev::to_lower(context_name));
                        }
                        else
                        {
                            CATCH_REQUIRE(commands[0]->get_string(prinbee::pbql::param_t::PARAM_PATH) == snapdev::to_lower(context_path));
                        }
                        // WITH ( OWNER [']<user>[:<group>]['] )
                        CATCH_REQUIRE(commands[0]->is_defined_as(prinbee::pbql::param_t::PARAM_USER) == prinbee::pbql::param_type_t::PARAM_TYPE_STRING);
                        CATCH_REQUIRE(commands[0]->get_string(prinbee::pbql::param_t::PARAM_USER) == ownership);
                        CATCH_REQUIRE(commands[0]->is_defined_as(prinbee::pbql::param_t::PARAM_GROUP) == prinbee::pbql::param_type_t::PARAM_TYPE_STRING);
                        CATCH_REQUIRE(commands[0]->get_string(prinbee::pbql::param_t::PARAM_GROUP) == group_member);
                        // WITH ( COMMENT '<description>' )
                        CATCH_REQUIRE(commands[0]->is_defined_as(prinbee::pbql::param_t::PARAM_DESCRIPTION) == prinbee::pbql::param_type_t::PARAM_TYPE_STRING);
                        CATCH_REQUIRE(commands[0]->get_string(prinbee::pbql::param_t::PARAM_DESCRIPTION) == description);

                        // to track the number of attempts in the context name
                        //
                        ++counter;
                    }
                }
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("parser_error", "[parser][pbql][error]")
{
    CATCH_START_SECTION("parser_error: missing lexer")
    {
        prinbee::pbql::lexer::pointer_t lexer;
        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<prinbee::pbql::parser>(lexer)
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                          "logic_error: lexer missing."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("parser_error: create context errors")
    {
        {
            // CREATE CONTEXT <identifier>
            std::string script("CREATE CONTEXT 123;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:16: expected an identifier after CREATE CONTEXT."));
        }

        {
            // CREATE CONTEXT IF <NOT>
            std::string script("CREATE CONTEXT IF FOO;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:19: expected the NOT identifier after CREATE CONTEXT IF, not \"FOO\"."));
        }

        {
            // CREATE CONTEXT IF NOT EXIST<S>
            std::string script("CREATE CONTEXT IF NOT EXIST;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:23: expected the EXISTS identifier after CREATE CONTEXT IF NOT, not \"EXIST\"."));
        }

        {
            // CREATE CONTEXT IF NOT EXISTS <identifier>
            std::string script("CREATE CONTEXT IF NOT EXISTS 123;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:30: expected a IDENTIFIER after CREATE CONTEXT IF NOT EXISTS, not a INTEGER."));
        }

        {
            // CREATE CONTEXT my_context USING 123;
            std::string script("CREATE CONTEXT my_context USING 123;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:33: expected a path after the USING keyword of CREATE CONTEXT."));
        }

        {
            // CREATE CONTEXT my_context USING 'path' <USING>;
            std::string script("CREATE CONTEXT my_context USING 'path' USING;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:40: USING keyword found twice after CREATE CONTEXT."));
        }

        {
            // CREATE CONTEXT my_context USING '<empty>';
            std::string script("CREATE CONTEXT my_context USING '';");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:33: expected a non-empty path after the USING keyword of CREATE CONTEXT."));
        }

        {
            // CREATE CONTEXT my_context WITH <comment>;
            std::string script("CREATE CONTEXT my_context WITH comment;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:32: WITH feature definitions must be defined between parenthesis, '(' missing in CREATE CONTEXT."));
        }

        {
            // CREATE CONTEXT my_context WITH ( <123>;
            std::string script("CREATE CONTEXT my_context WITH ( 123;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:34: WITH feature definitions must be named using an identifier in CREATE CONTEXT."));
        }

        {
            // CREATE CONTEXT my_context WITH ( OWNER name:'group';
            std::string script("CREATE CONTEXT my_context WITH ( OWNER name:'group';");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:46: "
                          "expected a group name after ':' in CREATE CONTEXT ... WITH ( OWNER <user>:<group> ), not a STRING."));
        }

        {
            // CREATE CONTEXT my_context WITH ( OWNER name:group, OWNER;
            std::string script("CREATE CONTEXT my_context WITH ( OWNER name:group, OWNER;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:58: "
                          "WITH OWNER found twice after CREATE CONTEXT."));
        }

        {
            // CREATE CONTEXT my_context WITH ( OWNER = 3.5;
            std::string script("CREATE CONTEXT my_context WITH ( OWNER = 3.5;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:42: "
                          "expected a string or an identifier after WITH ( OWNER <owner>[:<group>] )."));
        }

        {
            // CREATE CONTEXT my_context WITH ( COMMENT TRUE;
            std::string script("CREATE CONTEXT my_context WITH ( COMMENT TRUE;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:42: "
                          "expected a string for <description> in CREATE CONTEXT ... WITH ( COMMENT <description> ) got a IDENTIFIER."));
        }

        {
            // CREATE CONTEXT my_context WITH ( COMMENT 'good', COMMENT;
            std::string script("CREATE CONTEXT my_context WITH ( COMMENT 'good', COMMENT;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:58: "
                          "WITH COMMENT found twice after CREATE CONTEXT."));
        }

        {
            // CREATE CONTEXT my_context WITH ( COMMENT 'good'?;
            std::string script("CREATE CONTEXT my_context WITH ( COMMENT 'good' 123;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "create-context-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: create-context-test.pbql:1:49: "
                          "expected a comma to separate feature definitions in CREATE CONTEXT."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("parser_error: BEGIN/COMMIT/ROLLBACK mistakes")
    {
        {
            // BEGIN ON <123>
            std::string script("BEGIN ON 123;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "transaction-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: transaction-test.pbql:1:10: expected identifier SCHEMA or DATA after BEGIN ON."));
        }

        {
            // BEGIN ON TABLE
            std::string script("BEGIN ON TABLE;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "transaction-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: transaction-test.pbql:1:10: expected identifier SCHEMA or DATA after BEGIN ON."));
        }

        {
            // BEGIN ON SCHEMA IF
            std::string script("BEGIN ON SCHEMA IF;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "transaction-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: transaction-test.pbql:1:17: expected ';' at the end of 'BEGIN' command; not IDENTIFIER IF."));
        }

        {
            // COMMIT WORK ON SCHEMA WHEN
            std::string script("COMMIT WORK ON SCHEMA WHEN;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "transaction-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: transaction-test.pbql:1:23: expected IF clause or ';' at the end of a COMMIT or ROLLBACK."));
        }

        {
            // COMMIT TRANSACTION ON SCHEMA IF a <> b THEN
            std::string script("COMMIT TRANSACTION ON SCHEMA IF a <> b THEN;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "transaction-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: transaction-test.pbql:1:40: expected OTHERWISE after the IF expression of COMMIT or ROLLBACK."));
        }

        {
            // COMMIT TRANSACTION ON SCHEMA IF a <> b THEN
            std::string script("COMMIT TRANSACTION ON SCHEMA IF a <> b OTHERWISE 123;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "transaction-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: transaction-test.pbql:1:50: expected ROLLBACK after OTHERWISE for command COMMIT."));
        }

        {
            // COMMIT TRANSACTION ON SCHEMA IF a <> b THEN
            std::string script("ROLLBACK TRANSACTION ON DATA IF a = b OTHERWISE 123;");

            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(script, "transaction-test.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: transaction-test.pbql:1:49: expected COMMIT after OTHERWISE for command ROLLBACK."));
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
