// Copyright (c) 2019-2024  Made to Order Software Corp.  All Rights Reserved
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
//#include    <snapdev/not_reached.h>
//#include    <snapdev/string_replace_many.h>
//#include    <snapdev/to_lower.h>


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






} // no name namespace



CATCH_TEST_CASE("expression", "[expression][parser][pbql]")
{
    CATCH_START_SECTION("expression: primary")
    {
        struct primary_t
        {
            char const *        f_primary = nullptr;
            char const *        f_expected = nullptr;
        };
        constexpr primary_t const primary_expressions[] =
        {
            {
                "SELECT 'string';",
                "\"string\"",
            },
            {
                "SELECT 'str' 'ing';",
                "\"string\"",
            },
            {
                "SELECT 'quoted \"string\"';",
                "\"quoted \\\"string\\\"\"",
            },
            {
                "SELECT E'escape \\b';",
                "\"escape \\b\"",
            },
            {
                "SELECT E'escape \\f';",
                "\"escape \\f\"",
            },
            {
                "SELECT E'escape \\n';",
                "\"escape \\n\"",
            },
            {
                "SELECT E'escape \\r';",
                "\"escape \\r\"",
            },
            {
                "SELECT E'escape \\t';",
                "\"escape \\t\"",
            },
            {
                "SELECT E'escape \\13';", // SQL does not support "\v" as is
                "\"escape \\v\"",
            },
            {
                "SELECT 1234;",
                "1234",
            },
            {
                "SELECT 123.4;",
                "123.4",
            },
        };
        for(auto const & e : primary_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_primary, "primary-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

SNAP_LOG_WARNING << "got command for expression! [" << e.f_primary << "]" << SNAP_LOG_SEND;
            CATCH_REQUIRE(commands.size() == 1);

            // BEGIN
            CATCH_REQUIRE(commands[0]->get_command() == prinbee::pbql::command_t::COMMAND_SELECT);
            // SCHEMA/DATA
            CATCH_REQUIRE(commands[0]->is_defined_as(prinbee::pbql::param_t::PARAM_EXPRESSION) == prinbee::pbql::param_type_t::PARAM_TYPE_STRING);
            CATCH_REQUIRE(commands[0]->get_string(prinbee::pbql::param_t::PARAM_EXPRESSION) == e.f_expected);

        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("expression_error", "[expression][parser][pbql][error]")
{
    CATCH_START_SECTION("parser: missing lexer")
    {
        prinbee::pbql::lexer::pointer_t lexer;
        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<prinbee::pbql::parser>(lexer)
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                          "logic_error: lexer missing."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
