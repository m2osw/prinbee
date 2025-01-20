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

    CATCH_START_SECTION("expression: postfix (except functions)")
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
                { "table_name.ALL_FIELDS" },
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
                "SELECT BigInt(Cast1), Boolean(Cast2), Char(Cast3),"
                    " Double Precision(Cast4),"
                    " Int(Cast5), Int1(Cast6), Int2(Cast7), Int4(Cast8),"
                    " Int8(Cast9), Int16(Cast10), Int32(Cast11), Int64(Cast12),"
                    " Integer(Cast13), Float2(Cast14), Float4(Cast15),"
                    " Real(Cast16), SmallInt(Cast17), Text(Cast18),"
                    " Unsigned BigInt(Cast19), Unsigned Int(Cast20),"
                    " Unsigned Int1(Cast21), Unsigned Int2(Cast22),"
                    " Unsigned Int4(Cast23), Unsigned Int8(Cast24),"
                    " Unsigned Int16(Cast25), Unsigned Int32(Cast26),"
                    " Unsigned Int64(Cast27), Unsigned Integer(Cast28)"
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
                "SELECT BigInt Cast1, Boolean Cast2, Char Cast3,"
                    " Double Precision Cast4,"
                    " Int Cast5, Int1 Cast6, Int2 Cast7, Int4 Cast8,"
                    " Int8 Cast9, Int16 Cast10, Int32 Cast11, Int64 Cast12,"
                    " Integer Cast13, Float2 Cast14, Float4 Cast15,"
                    " Real Cast16, SmallInt Cast17, Text Cast18,"
                    " Unsigned BigInt Cast19, Unsigned Int Cast20,"
                    " Unsigned Int1 Cast21, Unsigned Int2 Cast22,"
                    " Unsigned Int4 Cast23, Unsigned Int8 Cast24,"
                    " Unsigned Int16 Cast25, Unsigned Int32 Cast26,"
                    " Unsigned Int64 Cast27, Unsigned Integer Cast28"
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

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_postfix << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
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

    CATCH_START_SECTION("expression: unary")
    {
        struct postfix_t
        {
            char const *        f_unary = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const unary_expressions[] =
        {
            {
                "SELECT +304;",
                { "304" },
            },
            {
                "SELECT -129;",
                { "-129" },
            },
            {
                "SELECT -(-912);",
                { "912" },
            },
            {
                "SELECT -+-192;",
                { "192" },
            },
            {
                "SELECT +-+-+-871;",
                { "-871" },
            },
        };
        for(auto const & e : unary_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_unary, "unary-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

SNAP_LOG_WARNING << "got command for expression! [" << e.f_unary << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
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

    CATCH_START_SECTION("expression: exponentiation")
    {
        struct postfix_t
        {
            char const *        f_exponentiation = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const exponentiation_expressions[] =
        {
            {
                "SELECT 2^8, 3^3, 5 ^ 7;",
                { "256", "27", "78125" },
            },
            {
                "SELECT '2'^8, 3^'3', '5' ^ '7';",
                { "256", "27", "78125" },
            },
            {
                "SELECT 4.11^2, 0.03^3;",
                { "16.8921", "0.000027" },
            },
            {
                "SELECT 2.01^3.11, 0.5^4.03;",
                { "8.768791", "0.061214" },
            },
            {
                "SELECT '2.01'^3.11, 0.5^'4.03';",
                { "8.768791", "0.061214" },
            },
            {
                "SELECT a^b, a^2, a^2^b, 3^2^d, a^2^3;",
                { "(a**b)", "(a**2)", "((a**2)**b)", "(9**d)", "((a**2)**3)" },
            },
        };
        for(auto const & e : exponentiation_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_exponentiation, "exponentiation-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_exponentiation << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
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

    CATCH_START_SECTION("expression: multiplicative")
    {
        struct postfix_t
        {
            char const *        f_multiplicative = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const multiplicative_expressions[] =
        {
            {
                "SELECT 2*8, 3 *3, 5 * 7, 5* 4;",
                { "16", "9", "35", "20" },
            },
            {
                "SELECT '2'*8, 3*'3', '5' * '7', 5* '4';",
                { "16", "9", "35", "20" },
            },
            {
                "SELECT 4.11*2, 0.03*3;",
                { "8.22", "0.09" },
            },
            {
                "SELECT 2.01*3.11, 0.5*4.03;",
                { "6.2511", "2.015" },
            },
            {
                "SELECT '2.01'*3.11, 0.5*'4.03';",
                { "6.2511", "2.015" },
            },
            {
                "SELECT a*b, a*2, a*2*b, 3*2*d, a*2*3;",
                { "a*b", "a*2", "a*2*b", "6*d", "a*2*3" },
            },
            {
                "SELECT 8/2, 13 /3, 85 / 7, 5/ 4;",
                { "4", "4", "12", "1" },
            },
            {
                "SELECT '8'/2, 13/'3', '85' / '7', 5/ '4';",
                { "4", "4", "12", "1" },
            },
            {
                "SELECT 4.11/2, 0.03/3;",
                { "2.055", "0.01" },
            },
            {
                "SELECT 2.01/3.11, 0.5/4.03;",
                { "0.646302", "0.124069" },
            },
            {
                "SELECT '2.01'/3.11, 0.5/'4.03';",
                { "0.646302", "0.124069" },
            },
            {
                "SELECT a/b, a/2, a/2/b, 3/2/d, a/2/3;",
                { "a/b", "a/2", "a/2/b", "1/d", "a/2/3" },
            },
            {
                "SELECT 8%5, 13 %3, 85 % 7, 5% 4;",
                { "3", "1", "1", "1" },
            },
            {
                "SELECT '8'%5, 23%'3', '85' % '7', 7% '4';",
                { "3", "2", "1", "3" },
            },
            {
                "SELECT 4.11%2, 0.03%3;",
                { "0.11", "0.03" },
            },
            {
                "SELECT 2.01%3.11, 0.5%4.03;",
                { "2.01", "0.5" },
            },
            {
                "SELECT '2.01'%3.11, 0.5%'4.03';",
                { "2.01", "0.5" },
            },
            {
                "SELECT a%b, a%2, a%2%b, 3%2%d, a%2%3;",
                { "a%b", "a%2", "a%2%b", "1%d", "a%2%3" },
            },
        };
        for(auto const & e : multiplicative_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_multiplicative, "multiplicative-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_multiplicative << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
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

    CATCH_START_SECTION("expression: additive")
    {
        struct postfix_t
        {
            char const *        f_additive = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const additive_expressions[] =
        {
            {
                "SELECT 2+8, 3 +3, 5 + 7, 5+ 4;",
                { "10", "6", "12", "9" },
            },
            {
                "SELECT '2'+8, 3+'3', '5' + '7', 5+ '4';",
                { "10", "6", "12", "9" },
            },
            {
                "SELECT 4.11+2, 0.03+3;",
                { "6.11", "3.03" },
            },
            {
                "SELECT 2.01+3.11, 0.5+4.03;",
                { "5.12", "4.53" },
            },
            {
                "SELECT '2.01'+3.11, 0.5+'4.03';",
                { "5.12", "4.53" },
            },
            {
                "SELECT a+b, a+2, a+2+b, 3+2+d, a+2+3;",
                { "a+b", "a+2", "a+2+b", "5+d", "a+2+3" },
            },
            {
                "SELECT 8-2, 13 -3, 85 - 7, 5- 4;",
                { "6", "10", "78", "1" },
            },
            {
                "SELECT '8'-2, 13-'3', '85' - '7', 5- '4';",
                { "6", "10", "78", "1" },
            },
            {
                "SELECT 4.11-2, 0.03-3;",
                { "2.11", "-2.97" },
            },
            {
                "SELECT 2.01-3.11, 0.5-4.03;",
                { "-1.1", "-3.53" },
            },
            {
                "SELECT '2.01'-3.11, 0.5-'4.03';",
                { "-1.1", "-3.53" },
            },
            {
                "SELECT a-b, a-2, a-2-b, 3-2-d, a-2-3;",
                { "a-b", "a-2", "a-2-b", "1-d", "a-2-3" },
            },
        };
        for(auto const & e : additive_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_additive, "additive-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_additive << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
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


CATCH_TEST_CASE("expression_functions", "[expression][parser][pbql]")
{
    CATCH_START_SECTION("expression: functions")
    {
        struct function_t
        {
            char const *        f_function = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
            std::vector<std::string>
                                f_column_name = std::vector<std::string>();
        };
        function_t const function_expressions[] =
        {
            {
                "SELECT @5 AS pos, @-5 AS neg, Sign(+32), SiGn(-9), siGN(0);",
                { "5", "5", "1", "-1", "0" },
                { "pos", "neg" },
            },
            {
                "SELECT @3.05, @-4.32, Abs(45.3), aBs(-5.91), sign(57.61), SIGN(-101.0043), sIGn(0.0);",
                { "3.05", "4.32", "45.3", "5.91", "1", "-1", "0" },
                { "__col1", "__col2", "__col3", "__col4", "__col5", "__col6", "__col7" },
            },
            {
                "SELECT @a, @-b as neg, ABS(c), abs(d) As lc, sign(e) AS s;",
                { "Math.abs(a)", "Math.abs(b)", "Math.abs(c)", "Math.abs(d)", "Math.sign(e)" },
                { "__col1", "neg", "__col3", "lc", "s" },
            },
            {
                "SELECT |/121, |/ 25.25, |/-81, |/a, |/-b, |/@c;",
                { "11.0", "5.024938", "NaN", "Math.sqrt(a)", "Math.sqrt(-b)", "Math.sqrt(Math.abs(c))" },
            },
            {
                "SELECT ||/1331, ||/ 25.25, ||/-729, ||/ -700 - 29, ||/a, ||/-b, ||/@c;",
                { "11.0", "2.933732", "-9.0", "-9.0", "Math.cbrt(a)", "Math.cbrt(-b)", "Math.cbrt(Math.abs(c))" },
            },
            {
                "SELECT Abs(45.3) - 9.1, Abs(-99) + 3;",
                { "36.2", "102" },
            },
            {
                "SELECT sin(4.3), cos(-0.75), tan(0.7775),"
                      " sinh(4.3), cosh(-0.75), tanh(0.7775),"
                      " asin(0.3), acos(-0.75), atan(0.7775), atan(45, 100),"
                      " asinh(4.3), acosh(1.75), atanh(0.7775);",
                { "-0.916166", "0.731689", "0.984327",
                  "36.843113", "1.294683", "0.651269",
                   "0.304693", "2.418858", "0.66087", "0.422854",
                   "2.165017", "1.15881",  "1.039018" },
            },
            {
                "SELECT sin(a), cos(b), tan(c),"
                      " sinh(d), cosh(e), tanh(f),"
                      " asin(g), acos(h), atan(i), atan(j, k),"
                      " asinh(l), acosh(m), atanh(n);",
                { "Math.sin(a)",   "Math.cos(b)",   "Math.tan(c)",
                  "Math.sinh(d)",  "Math.cosh(e)",  "Math.tanh(f)",
                  "Math.asin(g)",  "Math.acos(h)",  "Math.atan(i)", "Math.atan2(j,k)",
                  "Math.asinh(l)", "Math.acosh(m)", "Math.atanh(n)" },
            },
            {
                "SELECT ceil(17), ceil(4.3), ceil(-11.35),"
                      " floor(101), floor(9.75), floor(-0.75),"
                      " round(7.775), round(-14.1), round(17), round(-23),"
                      " trunc(4.3), trunc(-44.3), trunc(45), trunc(-90);",
                { "17", "5.0", "-11.0",
                  "101", "9.0", "-1.0",
                  "8", "-14", "17", "-23",
                  "4.0", "-44.0", "45", "-90" },
            },
            {
                "SELECT ceil(a), floor(b), round(c), trunc(d);",
                { "Math.ceil(a)", "Math.floor(b)", "Math.round(c)", "Math.trunc(d)" },
            },
            {
                "SELECT a || b, c || d || e || f, 'lit' || g, h || 'lit',"
                    " i || 'par' || 'tial', 'st' || 'art' || j, k || 'mid' || 'dle' || l;",
                { "String.concat(a,b)", "String.concat(c,d,e,f)", "String.concat(\"lit\",g)", "String.concat(h,\"lit\")",
                  "String.concat(i,\"partial\")", "String.concat(\"start\",j)", "String.concat(k,\"middle\",l)" },
            },
            {
                "SELECT exp(4.3), expm1(0.003501), pow(9.75, 3.07), pow(4, 13),"
                      " log(7.775), log1p(14.1), log10(10000), log2(65536);",
                { "73.699794", "0.003507", "1087.036608", "67108864",
                  "2.050913", "2.714695", "4.0", "16.0" },
            },
            {
                "SELECT exp(a), expm1(b), pow(c, d),"
                      " log(e), log1p(f), log10(g), log2(h);",
                { "Math.exp(a)", "Math.expm1(b)", "(c**d)",
                  "Math.log(e)", "Math.log1p(f)", "Math.log10(g)", "Math.log2(h)" },
            },
            {
                "SELECT rand();",
                { "Math.rand()" },
            },
            {
                "SELECT hypot(), hypot(55.003), hypot(19.75, 23.07),"
                      " hypot(7.775, 14.1, 100), hypot(-65.6);",
                { "0.0", "55.003", "30.369185",
                  "101.288008", "65.6" },
            },
            {
                "SELECT hypot(a), hypot(b, c), hypot(d, e, f);",
                { "Math.abs(a)", "Math.hypot(b,c)", "Math.hypot(d,e,f)" },
            },
            {
                "SELECT imul(33.2, 25.03), imul(-13.02, 5.78), imul(3, 9), imul(5, -45);",
                { "825", "-65", "27", "-225" },
            },
            {
                "SELECT imul(a, b);",
                { "Math.imul(a,b)" },
            },
            {
                "SELECT length('this string is 33 characters long'), length(a);",
                { "33", "a.length" },
            },
            {
                "SELECT min(), min(1.0), min(2), min(33.2, 25.03), min(-13.02, 5.78, -45, +1000), min(78, -9, 34, 2, -8);",
                { "Infinity", "1.0", "2", "25.03", "-45.0", "-9" },
            },
            {
                "SELECT min(a, b), min(c, d, e, f, g, h);",
                { "Math.min(a,b)", "Math.min(c,d,e,f,g,h)" },
            },
            {
                "SELECT max(), max(1.0), max(2), max(33.2, 25.03), max(-13.02, 5.78, -45, +1000), max(78, -9, 34, 2, -8);",
                { "-Infinity", "1.0", "2", "33.2", "1000.0", "78" },
            },
            {
                "SELECT max(a, b), max(c, d, e, f, g, h);",
                { "Math.max(a,b)", "Math.max(c,d,e,f,g,h)" },
            },
        };
        for(auto const & e : function_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_function, "function-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

SNAP_LOG_WARNING << "got command for expression! [" << e.f_function << "]" << SNAP_LOG_SEND;
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
                if(idx < e.f_column_name.size())
                {
                    CATCH_REQUIRE(commands[0]->get_string(prinbee::pbql::param_t::PARAM_COLUMN_NAME + idx) == e.f_column_name[idx]);
                }
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
                          "prinbee_exception: primary-expression.pbql:1:8: expected a primary token not '=' (primary tokens are: string, number, true, false, identifier, '*', or an expression between parenthesis)."));
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
                          "prinbee_exception: postfix-expression.pbql:1:29: expected an integer name to follow the UNSIGNED word (post casting)."));
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
                          "prinbee_exception: postfix-expression.pbql:1:29: expected an integer name to follow the UNSIGNED word (post casting)."));
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
                          "prinbee_exception: postfix-expression.pbql:1:36: expected a closing square bracket (]), not ';'."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
