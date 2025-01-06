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
#include    <snapdev/enum_class_math.h>


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
            {
                "SELECT true;",
                "true",
            },
            {
                "SELECT false;",
                "false",
            },
            {
                "SELECT True;",
                "true",
            },
            {
                "SELECT FALSE;",
                "false",
            },
            {
                "SELECT (TRUE);",
                "true",
            },
            {
                "SELECT (FaLsE);",
                "false",
            },
            {
                "SELECT table_name;",
                "table_name",
            },
            {
                "SELECT Table_Name;",
                "table_name",
            },
            {
                "SELECT *;",
                "ALL_FIELDS",
            },
        };
        for(auto const & e : primary_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_primary, "primary-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_primary << "]" << SNAP_LOG_SEND;
            CATCH_REQUIRE(commands.size() == 1);

            // BEGIN
            CATCH_REQUIRE(commands[0]->get_command() == prinbee::pbql::command_t::COMMAND_SELECT);
            // SCHEMA/DATA
            CATCH_REQUIRE(commands[0]->is_defined_as(prinbee::pbql::param_t::PARAM_EXPRESSION) == prinbee::pbql::param_type_t::PARAM_TYPE_STRING);
            CATCH_REQUIRE(commands[0]->get_string(prinbee::pbql::param_t::PARAM_EXPRESSION) == e.f_expected);

        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression: postfix")
    {
        struct postfix_t
        {
            char const *        f_postfix = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const postfix_expressions[] =
        {
            {
                "SELECT Table_Name.Column_Name.Field_Name;",
                { "table_name.column_name.field_name" },
            },
            {
                "SELECT Table_Name.*;",
                { "table_name" },
            },
            {
                "SELECT Cast1::BigInt, Cast2::Boolean, Cast3::Char,"
                    " Cast4::Double Precision,"
                    " Cast5::Int, Cast6::Int1, Cast7::Int2, Cast8::Int4,"
                    " Cast9::Int8, Cast10::Int16, Cast11::Int32, Cast12::Int64,"
                    " Cast13::Integer, Cast14::Float2, Cast15::Float4,"
                    " Cast16::Real, Cast17::SmallInt, Cast18::Text,"
                    " Cast19::Unsigned BigInt, Cast20::Unsigned Int,"
                    " Cast21::Unsigned Int1, Cast22::Unsigned Int2,"
                    " Cast23::Unsigned Int4, Cast24::Unsigned Int8,"
                    " Cast25::Unsigned Int16, Cast26::Unsigned Int32,"
                    " Cast27::Unsigned Int64, Cast28::Unsigned Integer"
                    ";",
                {
                    "new Integer(cast1)",
                    "new Boolean(cast2)",
                    "new String(cast3)",
                    "new Number(cast4)",
                    "new Integer(cast5)",
                    "new Integer(cast6)",
                    "new Integer(cast7)",
                    "new Integer(cast8)",
                    "new Integer(cast9)",
                    "new Integer(cast10)",
                    "new Integer(cast11)",
                    "new Integer(cast12)",
                    "new Integer(cast13)",
                    "new Number(cast14)",
                    "new Number(cast15)",
                    "new Number(cast16)",
                    "new Integer(cast17)",
                    "new String(cast18)",
                    "new Integer(cast19)",
                    "new Integer(cast20)",
                    "new Integer(cast21)",
                    "new Integer(cast22)",
                    "new Integer(cast23)",
                    "new Integer(cast24)",
                    "new Integer(cast25)",
                    "new Integer(cast26)",
                    "new Integer(cast27)",
                    "new Integer(cast28)",
                },
            },
            {
                "SELECT Table_Name.Array_Field[3];",
                { "table_name.array_field[3]" },
            },
        };
        for(auto const & e : postfix_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_postfix, "postfix-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

SNAP_LOG_WARNING << "got command for expression! [" << e.f_postfix << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
            CATCH_REQUIRE(commands.size() == 1);

            // BEGIN
            CATCH_REQUIRE(commands[0]->get_command() == prinbee::pbql::command_t::COMMAND_SELECT);
            // SCHEMA/DATA
            std::size_t const max(e.f_expected.size());
            CATCH_REQUIRE(max <= prinbee::pbql::MAX_EXPRESSIONS);
            for(std::size_t idx(0); idx < max; ++idx)
            {
                CATCH_REQUIRE(commands[0]->is_defined_as(prinbee::pbql::param_t::PARAM_EXPRESSION + idx) == prinbee::pbql::param_type_t::PARAM_TYPE_STRING);
                CATCH_REQUIRE(commands[0]->get_string(prinbee::pbql::param_t::PARAM_EXPRESSION + idx) == e.f_expected[idx]);
            }
            CATCH_REQUIRE(commands[0]->is_defined_as(prinbee::pbql::param_t::PARAM_EXPRESSION + max) == prinbee::pbql::param_type_t::PARAM_TYPE_UNKNOWN);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("expression_error", "[expression][parser][pbql][error]")
{
    CATCH_START_SECTION("expression_error: unknown primary expression")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT =;", "primary-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: primary-expression.pbql:1:8: expected a primary token not = (primary tokens are: string, number, true, false, identifier, '*', or an expression between parenthesis)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: missing ')'")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT (true;", "primary-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: primary-expression.pbql:1:14: expected ')' to close the grouped expressions."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: field name after '.*'")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name.*.more;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:20: no more '.' can be used after '.*'."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: field name cannot be an integer")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name.491;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:19: expected '*' or a field name after '.'."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: scope must be followed by an identifier")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name::491;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:20: a type name was expected after the '::' operator."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: unknown type after scope")
    {
        constexpr char const * bad_names[] = {
            "AMOEBA",
            "BRILLANT",
            "CHARLIE",
            "DARLING",
            "ENGINEERING",
            "FLAKY",
            "GLORY",
            "HOVERING",
            "INVENTORY",
            "JOUST",
            "KRAKEN",
            "LUNAR",
            "MOMENT",
            "NORTH",
            "OPAL",
            "PARACHUTE",
            "QUARTER",
            "REST",
            "STATUE",
            "TRICKERY",
            "UNIVERSE",
            "UNSIGNED NUMBER",
            "VERTICAL",
            "WISH",
            "XENOPHOBE",
            "YEAH",
            "ZEBRA",
        };
        for(auto const & n : bad_names)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            std::string in("SELECT table_name::");
            in += n;
            in += ";";
            lexer->set_input(std::make_shared<prinbee::pbql::input>(in, "postfix-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                      parser->parse()
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: postfix-expression.pbql:1:20: expected"
                              " the name of a type after the '::' operator, found \""
                            + std::string(n)
                            + "\" instead."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: type is DOUBLE PRECISION, not DOUBLE NUMBER")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name::DOUBLE NUMBER;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:20: expected"
                          " DOUBLE to be followed by the word PRECISION."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: type is DOUBLE PRECISION, not DOUBLE 3.1415926")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name::DOUBLE NUMBER;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:20: expected"
                          " DOUBLE to be followed by the word PRECISION."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: type is DOUBLE PRECISION, not DOUBLE 'PRECISION'")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name::DOUBLE NUMBER;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:20: expected"
                          " DOUBLE to be followed by the word PRECISION."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: type is UNSIGNED <int>, not UNSIGNED 42")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name::UNSIGNED 42;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:29: expected an integer name to follow the UNSIGNED word."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: type is UNSIGNED <int>, not UNSIGNED 'INTEGER'")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name::UNSIGNED 'INTEGER';", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:29: expected an integer name to follow the UNSIGNED word."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: missing ']'")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT MyTable.ExtendedField[INDEX;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:36: expected a closing square bracket (]), not ;."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
