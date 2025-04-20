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
#include    <snapdev/enum_class_math.h>
#include    <snapdev/to_lower.h>


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
                    " Cast13::Integer, Cast14::Float4, Cast15::Float8, Cast16::Float10,"
                    " Cast17::Real, Cast18::SmallInt, Cast19::Text,"
                    " Cast20::Unsigned BigInt, Cast21::Unsigned Int,"
                    " Cast22::Unsigned Int1, Cast23::Unsigned Int2,"
                    " Cast24::Unsigned Int4, Cast25::Unsigned Int8,"
                    " Cast26::Unsigned Int16, Cast27::Unsigned Int32,"
                    " Cast28::Unsigned Int64, Cast29::Unsigned Integer"
                    ";",
                {
                    "new Integer(cast1)",
                    "!!(cast2)",
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
                    "new Number(cast17)",
                    "new Integer(cast18)",
                    "new String(cast19)",
                    "new Integer(cast20)",
                    "new Integer(cast21)",
                    "new Integer(cast22)",
                    "new Integer(cast23)",
                    "new Integer(cast24)",
                    "new Integer(cast25)",
                    "new Integer(cast26)",
                    "new Integer(cast27)",
                    "new Integer(cast28)",
                    "new Integer(cast29)",
                },
            },
            {
                "SELECT BigInt(Cast1), Boolean(Cast2), Char(Cast3),"
                    " Double Precision(Cast4),"
                    " Int(Cast5), Int1(Cast6), Int2(Cast7), Int4(Cast8),"
                    " Int8(Cast9), Int16(Cast10), Int32(Cast11), Int64(Cast12),"
                    " Integer(Cast13), Float4(Cast14), Float8(Cast15), Float10(Cast16),"
                    " Real(Cast17), SmallInt(Cast18), Text(Cast19),"
                    " Unsigned BigInt(Cast20), Unsigned Int(Cast21),"
                    " Unsigned Int1(Cast22), Unsigned Int2(Cast23),"
                    " Unsigned Int4(Cast24), Unsigned Int8(Cast25),"
                    " Unsigned Int16(Cast26), Unsigned Int32(Cast27),"
                    " Unsigned Int64(Cast28), Unsigned Integer(Cast29)"
                    ";",
                {
                    "new Integer(cast1)",
                    "!!(cast2)",
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
                    "new Number(cast17)",
                    "new Integer(cast18)",
                    "new String(cast19)",
                    "new Integer(cast20)",
                    "new Integer(cast21)",
                    "new Integer(cast22)",
                    "new Integer(cast23)",
                    "new Integer(cast24)",
                    "new Integer(cast25)",
                    "new Integer(cast26)",
                    "new Integer(cast27)",
                    "new Integer(cast28)",
                    "new Integer(cast29)",
                },
            },
            {
                "SELECT BigInt Cast1, Boolean Cast2, Char Cast3,"
                    " Double Precision Cast4,"
                    " Int Cast5, Int1 Cast6, Int2 Cast7, Int4 Cast8,"
                    " Int8 Cast9, Int16 Cast10, Int32 Cast11, Int64 Cast12,"
                    " Integer Cast13, Float4 Cast14, Float8 Cast15, Float10 Cast16,"
                    " Real Cast17, SmallInt Cast18, Text Cast19,"
                    " Unsigned BigInt Cast20, Unsigned Int Cast21,"
                    " Unsigned Int1 Cast22, Unsigned Int2 Cast23,"
                    " Unsigned Int4 Cast24, Unsigned Int8 Cast25,"
                    " Unsigned Int16 Cast26, Unsigned Int32 Cast27,"
                    " Unsigned Int64 Cast28, Unsigned Integer Cast29"
                    ";",
                {
                    "new Integer(cast1)",
                    "!!(cast2)",
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
                    "new Number(cast17)",
                    "new Integer(cast18)",
                    "new String(cast19)",
                    "new Integer(cast20)",
                    "new Integer(cast21)",
                    "new Integer(cast22)",
                    "new Integer(cast23)",
                    "new Integer(cast24)",
                    "new Integer(cast25)",
                    "new Integer(cast26)",
                    "new Integer(cast27)",
                    "new Integer(cast28)",
                    "new Integer(cast29)",
                },
            },
            {
                "SELECT Table_Name.Array_Field[3];",
                { "table_name.array_field[3]" },
            },
        };

        for(auto const & e : postfix_expressions)
        {
//SNAP_LOG_WARNING << "ready to parse next expression [" << e.f_postfix << "]" << SNAP_LOG_SEND;
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
                "SELECT +304, +'111', +3.45, +'9.03';",
                { "304", "111", "3.45", "9.03" },
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
            {
                "SELECT -'3101', +-'15.98', +a, - - b, -c;",
                { "-3101", "-15.98", "new Number(a)", "new Number(b)", "-c", },
            },
        };
        for(auto const & e : unary_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_unary, "unary-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_unary << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
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

    CATCH_START_SECTION("expression: other (unary other than + and - and binary operators not found somewhere else)")
    {
        struct postfix_t
        {
            char const *        f_additive = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const other_expressions[] =
        {
            {
                "SELECT 76 & 14, 3 | 9, 5 # 7;",
                { "12", "11", "2" },
            },
            {
                "SELECT a & b, c | d, e # f;",
                { "a&b", "c|d", "e^f" },
            },
            {
                "SELECT 76 << 14, 76.31 << 14, 3 << 9, 5 << 7, 760 >> 14, 30000 >> 9, 159 >> 7, -97845198764363672415796583254123645 >> 100;",
                { "1245184", "1245184", "1536", "640", "0", "58", "1", "-77187" },
            },
            {
                "SELECT a << b, c >> d;",
                { "a<<b", "c>>d" },
            },
            {
                "SELECT a || b, c || d || e || f, 'lit' || g, h || 'lit',"
                    " i || 'par' || 'tial', 'st' || 'art' || j, k || 'mid' || 'dle' || l,"
                    " m || 304 || 'n' || 10.5 || n, 'con' || 'cat' || ' to ' || 'literal';",
                { "String.concat(a,b)", "String.concat(c,d,e,f)", "String.concat(\"lit\",g)", "String.concat(h,\"lit\")",
                  "String.concat(i,\"partial\")", "String.concat(\"start\",j)", "String.concat(k,\"middle\",l)",
                  "String.concat(m,\"304n10.5\",n)", "\"concat to literal\"" },
            },
            {
                "SELECT 'this string' ~ 'matches that string?', 'this string' !~ 'matches that string?', a ~ b, c !~ d;",
                { "false", "true", "new RegExp(b).test(a)", "!new RegExp(d).test(c)" },
            },
        };
        for(auto const & e : other_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_additive, "other-expression.pbql"));
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

    CATCH_START_SECTION("expression: matching (BETWEEN, IN, LIKE, ILIKE)")
    {
        struct postfix_t
        {
            char const *        f_matching = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const other_expressions[] =
        {
            {
                "SELECT 3 BETWEEN -10 AND +10;",
                { "true" },
            },
            {
                "SELECT -3 BETWEEN 0 AND '+10';",
                { "false" },
            },
            {
                "SELECT 30 BETWEEN '-10' AND 10;",
                { "false" },
            },
            {
                "SELECT 3.0 BETWEEN -'3.1' AND +3.1;",
                { "true" },
            },
            {
                "SELECT 3.0 NOT BETWEEN -'3.1' AND +3.1;",
                { "false" },
            },
            {
                "SELECT -3.3 BETWEEN '-3.2' AND +5;",
                { "false" },
            },
            {
                "SELECT 7.5 BETWEEN -5 AND 5.5;",
                { "false" },
            },
            {
                "SELECT 'hello' BETWEEN 'kitty' AND 'world';",
                { "false" },
            },
            {
                "SELECT 'kitty' BETWEEN 'hello' AND 'world';",
                { "true" },
            },
            {
                "SELECT 'hello' NOT BETWEEN 'kitty' AND 'world';",
                { "true" },
            },
            {
                "SELECT 'kitty' NOT BETWEEN 'hello' AND 'world';",
                { "false" },
            },
            {
                "SELECT null BETWEEN 0 AND 100;",
                { "null" },
            },
            {
                "SELECT true BETWEEN false AND true;",
                { "true" },
            },
            {
                "SELECT true BETWEEN true AND false;",
                { "false" },
            },
            {
                "SELECT true BETWEEN true AND true;",
                { "true" },
            },
            {
                "SELECT true BETWEEN false AND false;",
                { "false" },
            },
            {
                "SELECT false BETWEEN false AND true;",
                { "true" },
            },
            {
                "SELECT false BETWEEN true AND false;",
                { "false" },
            },
            {
                "SELECT false BETWEEN true AND true;",
                { "false" },
            },
            {
                "SELECT false BETWEEN false AND false;",
                { "true" },
            },
            {
                "SELECT a BETWEEN b AND c;",
                { "(_t1=a,_t1>=b&&_t1<=c)" },
            },
            {
                "SELECT a NOT BETWEEN b AND c;",
                { "!(_t1=a,_t1>=b&&_t1<=c)" },
            },
            {
                "SELECT 'hello world' LIKE '%world%', 'Hello World' ILIKE '%HELLO%',"
                      " 'hello world' LIKE '%world', 'Hello World' ILIKE 'HELLO%',"
                      " 'hello world' LIKE 'world%', 'Hello World' ILIKE '%HELLO',"
                      " 'hello world' NOT LIKE '%world%', 'Hello World' NOT ILIKE '%HELLO%',"
                      " 'hello world' NOT LIKE '%world', 'Hello World' NOT ILIKE 'HELLO%',"
                      " 'hello world' NOT LIKE 'world%', 'Hello World' NOT ILIKE '%HELLO';",
                {
                    "true", "true", "true", "true", "false", "false",
                    "false", "false", "false", "false", "true", "true",
                },
            },
            {
                "SELECT a LIKE b, c ILIKE d, e NOT LIKE f, g NOT ILIKE h;",
                {
                    "new RegExp(b).test(a)", "new RegExp(d,\"i\").test(c)",
                    "!new RegExp(f).test(e)", "!new RegExp(h,\"i\").test(g)",
                },
            },
            {
                "SELECT a LIKE '%word%', b ILIKE '%WORD%', c NOT LIKE '%word%', d NOT ILIKE '%WORD%';",
                {
                    "new RegExp(\"^.*word.*$\").test(a)", "new RegExp(\"^.*WORD.*$\",\"i\").test(b)",
                    "!new RegExp(\"^.*word.*$\").test(c)", "!new RegExp(\"^.*WORD.*$\",\"i\").test(d)",
                },
            },
        };
        for(auto const & e : other_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_matching, "matching-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_matching << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
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

    CATCH_START_SECTION("expression: comparison")
    {
        struct postfix_t
        {
            char const *        f_comparison = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const other_expressions[] =
        {
            {
                "SELECT NULL < 8, NULL <= 7.3, NULL = 'string', NULL > NULL, NULL >= True, NULL <> False,"
                      " 3 < NULL, 7.3 <= NULL, 'string' = NULL, NULL > NULL, True >= NULL, False <> NULL;",
                {
                    "null", "null", "null", "null", "null", "null",
                    "null", "null", "null", "null", "null", "null",
                },
            },
            {
                "SELECT 3 < 8, 7 <= 7, 4 = 4, 9 > 7, 6 >= 6, 1 <> 9,"
                      " '3' < '18', '7' <= '7', '4' = '04', '19' > '7', '6' >= '6', '1' <> '9',"
                      " 3.5 < 8.2, 7.4 <= 7.4, 4.5 = 4.5, 9.2 > 7.01, 6.3 >= 6.2, 1.9 <> 9.1,"
                      " '3.5' < '8.2', '7.4' <= '7.4', '4.5' = '4.5', '11.2' > '7.01', '6.3' >= '6.2', '1.9' <> '9.1';",
                {
                    "true", "true", "true", "true", "true", "true",
                    "true", "true", "true", "true", "true", "true",
                    "true", "true", "true", "true", "true", "true",
                    "true", "true", "true", "true", "true", "true",
                },
            },
            {
                "SELECT true < false, true <= true, false = false, true > true, false >= true, true <> true;",
                {
                    "false", "true", "true", "false", "false", "false",
                },
            },
            {
                "SELECT 'hello' < 'world', 'hello' <= 'kitty', 'kitty' = 'food', 'orange' > 'violet', 'toy' >= 'brick', 'thick' <> 'thin';",
                {
                    "true", "true", "false", "false", "true", "true",
                },
            },
            {
                "SELECT a < b, c <= d, e = f, g > h, i >= j, k <> l;",
                { "a<b", "c<=d", "e==f", "g>h", "i>=j", "k!=l" },
            },
        };
        for(auto const & e : other_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_comparison, "comparison-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_comparison << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
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

    CATCH_START_SECTION("expression: expr IS ...")
    {
        struct postfix_t
        {
            char const *        f_is = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const other_expressions[] =
        {
            {
                "SELECT 77 IS TRUE, 'string' IS TRUE, 3.9813 IS TRUE, TRUE IS TRUE, FALSE IS TRUE, NULL IS TRUE;",
                {
                    "false", "false", "false", "true", "false", "false",
                },
            },
            {
                "SELECT 77 IS FALSE, 'string' IS FALSE, 3.9813 IS FALSE, TRUE IS FALSE, FALSE IS FALSE, NULL IS FALSE;",
                {
                    "false", "false", "false", "false", "true", "false",
                },
            },
            {
                "SELECT 77 IS NULL, 'string' IS NULL, 3.9813 IS NULL, TRUE IS NULL, FALSE IS NULL, NULL IS NULL;",
                {
                    "false", "false", "false", "false", "false", "true",
                },
            },
            {
                "SELECT a IS TRUE, b IS NOT TRUE, c IS FALSE, d IS NOT FALSE, e IS NULL, f IS NOT NULL;",
                {
                    "a", "!b", "!c", "d", "e==null", "f!=null",
                },
            },
        };
        for(auto const & e : other_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_is, "comparison-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_is << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
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

    CATCH_START_SECTION("expression: NOT expr")
    {
        struct postfix_t
        {
            char const *        f_not = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const other_expressions[] =
        {
            {
                "SELECT NOT TRUE, NOT FALSE, NOT NOT TRUE, NOT NOT FALSE;",
                {
                    "false", "true", "true", "false",
                },
            },
            {
                "SELECT NOT 'TRUE', NOT 'FalsE', NOT NOT 'tru', NOT NOT 'f';",
                {
                    "false", "true", "true", "false",
                },
            },
            {
                "SELECT NOT 0, NOT 1, NOT NOT 3, NOT NOT 5.05, NOT NOT 0, NOT NOT 0.0;",
                {
                    "true", "false", "true", "true", "false", "false",
                },
            },
            {
                "SELECT NOT a, NOT NOT b, NOT NOT NOT c, NOT NOT NOT NOT d;",
                {
                    "!a", "!!b", "!c", "!!d",
                },
            },
        };
        for(auto const & e : other_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_not, "comparison-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_not << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
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

    CATCH_START_SECTION("expression: expr AND expr")
    {
        struct postfix_t
        {
            char const *        f_and = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const other_expressions[] =
        {
            {
                "SELECT TRUE AND TRUE, TRUE AND FALSE, FALSE AND TRUE, FALSE AND FALSE;",
                {
                    "true", "false", "false", "false",
                },
            },
            {
                "SELECT a AND b, c AND TRUE, TRUE AND d, e AND FALSE, FALSE AND f;",
                {
                    "a&&b", "c", "d", "false", "false",
                },
            },
        };
        for(auto const & e : other_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_and, "comparison-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_and << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
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

    CATCH_START_SECTION("expression: expr OR expr")
    {
        struct postfix_t
        {
            char const *        f_or = nullptr;
            std::vector<std::string>
                                f_expected = std::vector<std::string>();
        };
        postfix_t const other_expressions[] =
        {
            {
                "SELECT TRUE OR TRUE, TRUE OR FALSE, FALSE OR TRUE, FALSE OR FALSE;",
                {
                    "true", "true", "true", "false",
                },
            },
            {
                "SELECT a OR b, c OR TRUE, TRUE OR d, e OR FALSE, FALSE OR f;",
                {
                    "a||b", "true", "true", "e", "f",
                },
            },
        };
        for(auto const & e : other_expressions)
        {
SNAP_LOG_WARNING << "[early] got command for expression! [" << e.f_or << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_or, "comparison-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

SNAP_LOG_WARNING << "got command for expression! [" << e.f_or << "] max=" << e.f_expected.size() << SNAP_LOG_SEND;
            CATCH_REQUIRE(commands.size() == 1);

            // BEGIN
            CATCH_REQUIRE(commands[0]->get_command() == prinbee::pbql::command_t::COMMAND_SELECT);
            // SCHEMA/DATA
            std::size_t const max(e.f_expected.size());
            CATCH_REQUIRE(max <= prinbee::pbql::MAX_EXPRESSIONS);
            for(std::size_t idx(0); idx < max; ++idx)
            {
SNAP_LOG_WARNING << "compare #" << idx << SNAP_LOG_SEND;
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
                "SELECT @5 AS pos, @-6 AS neg, Sign(+32), SiGn(-9), siGN(0);",
                { "5", "6", "1", "-1", "0" },
                { "pos", "neg", "__col3", "__col4", "__col5" },
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
                "SELECT Abs(45.3) - 9.1, Abs(-99) + 3;",
                { "36.2", "102" },
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
                "SELECT SqRt(121.0), sqrt(a) + b, CbRt(1331), cbRT(c) - d;",
                { "11.0", "Math.sqrt(a)+b", "11.0", "Math.cbrt(c)-d" },
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
            { // two identifiers one after the other when the first is not a type
                "SELECT Column AS c1, demonstration AS c2, idea as c3, fork AS c4,"
                      " REACT AS C5, SMall As c6, tuition AS C7, urN aS c8, violet as c9"
                    ";",
                { "column", "demonstration", "idea", "fork",
                  "react", "small", "tuition", "urn", "violet" },
                { "c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", "c9" },
            },
        };
        for(auto const & e : function_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_function, "function-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));
            prinbee::pbql::command::vector_t const & commands(parser->parse());

//SNAP_LOG_WARNING << "got command for expression! [" << e.f_function << "]" << SNAP_LOG_SEND;
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
                          "prinbee_exception: postfix-expression.pbql:1:20: a type name was expected after the '::' operator, not INTEGER."));
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
                              "prinbee_exception: postfix-expression.pbql:1:20: "
                              "a type name was expected after the '::' operator, not IDENTIFIER \""
                            + snapdev::to_lower(std::string(n))
                            + "\"."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: unknown UNSIGNED integer type after scope")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name::UNSIGNED NUMBER;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:29: "
                          "expected an integer name to follow the UNSIGNED word (not 'NUMBER')."));
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
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name::DOUBLE 3.1415926;", "postfix-expression.pbql"));
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
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name::DOUBLE 'PRECISION';", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:20: expected"
                          " DOUBLE to be followed by the word PRECISION."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: type is UNSIGNED <int name>, not UNSIGNED 42")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT table_name::UNSIGNED 42;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:29: expected an integer name to follow the UNSIGNED word (not a INTEGER)."));
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
                          "prinbee_exception: postfix-expression.pbql:1:29: expected an integer name to follow the UNSIGNED word (not a STRING)."));
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

    CATCH_START_SECTION("expression_error: function name not an identifier (integer)")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT 45(11);", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:11: unexpected opening parenthesis ('(') after token INTEGER."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: function name not an identifier (string)")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT 'tan'(3.14159);", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:14: unexpected opening parenthesis ('(') after token STRING."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: function name not an identifier (true)")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT true(3.14159);", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:13: unexpected opening parenthesis ('(') after token TRUE."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: function name not an identifier (false)")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT false(3.14159);", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:14: unexpected opening parenthesis ('(') after token FALSE."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: double 'precision'() expected an identifier")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT double 'precision'(308);", "cast-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: cast-expression.pbql:1:8: expected DOUBLE to be followed by the word PRECISION."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: atan() called with no parameters")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT atan();", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:14: expected 1 or 2 parameters to ATAN(), found 0 instead."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: atan() called with 3 parameters")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT atan(x, y, z);", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:21: expected 1 or 2 parameters to ATAN(), found 3 instead."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: unknown functions (to test the 'false' statements in all cases)")
    {
        struct function_t
        {
            char const *        f_function = nullptr;
            char const *        f_error_msg = nullptr;
        };
        function_t const function_expressions[] =
        {
            {
                "SELECT algebra(15.03);",
                "prinbee_exception: function-expression.pbql:1:16: unknown function ALGEBRA().",
            },
            {
                "SELECT Brake(15.03);",
                "prinbee_exception: function-expression.pbql:1:14: unknown function BRAKE().",
            },
            {
                "SELECT COLUMNS(15.03);",
                "prinbee_exception: function-expression.pbql:1:16: unknown function COLUMNS().",
            },
            {
                "SELECT Edge_Case('car');",
                "prinbee_exception: function-expression.pbql:1:18: unknown function EDGE_CASE().",
            },
            {
                "SELECT FractioN(15.03);",
                "prinbee_exception: function-expression.pbql:1:17: unknown function FRACTION().",
            },
            {
                "SELECT HelloWorld(15.03);",
                "prinbee_exception: function-expression.pbql:1:19: unknown function HELLOWORLD().",
            },
            {
                "SELECT IS_Red(15.03);",
                "prinbee_exception: function-expression.pbql:1:15: unknown function IS_RED().",
            },
            {
                "SELECT Logarithm(15.03);",
                "prinbee_exception: function-expression.pbql:1:18: unknown function LOGARITHM().",
            },
            {
                "SELECT multi(15.03);",
                "prinbee_exception: function-expression.pbql:1:14: unknown function MULTI().",
            },
            {
                "SELECT price(15.03);",
                "prinbee_exception: function-expression.pbql:1:14: unknown function PRICE().",
            },
            {
                "SELECT Random_Chart(15.03);",
                "prinbee_exception: function-expression.pbql:1:21: unknown function RANDOM_CHART().",
            },
            {
                "SELECT STRING(15.03);",
                "prinbee_exception: function-expression.pbql:1:15: unknown function STRING().",
            },
            {
                "SELECT ToDay(15.03);",
                "prinbee_exception: function-expression.pbql:1:14: unknown function TODAY().",
            },
            {
                "SELECT Orange(15.03);",
                "prinbee_exception: function-expression.pbql:1:15: unknown function ORANGE().",
            },
        };
        for(auto const & e : function_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_function, "function-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                      parser->parse()
                    , prinbee::type_not_found
                    , Catch::Matchers::ExceptionMessage(e.f_error_msg));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: cast() with missing closing parenthesis")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT BIGINT(expression;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:26: type casting used '(' so a ')' was expected to end the casting expression."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: func() with missing closing parenthesis")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT ABS(expression;", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:23: expected ')' to end the list of parameters in a function call; not ';'."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: ABS(a, b) fails since it accepts only one parameter")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT ABS(a, b);", "postfix-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: postfix-expression.pbql:1:17: ABS() expected 1 parameter, found 2 instead."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: [NOT] IN not yet implemented")
    {
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT a IN b;", "postfix-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                      parser->parse()
                    , prinbee::not_yet_implemented
                    , Catch::Matchers::ExceptionMessage(
                              "not_yet_implemented: [NOT] IN is not yet implemented."));
        }

        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT a NOT IN b;", "postfix-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                      parser->parse()
                    , prinbee::not_yet_implemented
                    , Catch::Matchers::ExceptionMessage(
                              "not_yet_implemented: [NOT] IN is not yet implemented."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: a NOT 3|b is not valid")
    {
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT a NOT 3;", "matching-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                      parser->parse()
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: matching-expression.pbql:1:14: expected NOT to be followed by BETWEEN, IN, LIKE, ILIKE, or SIMILAR TO."));
        }

        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT a NOT b;", "matching-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                      parser->parse()
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: matching-expression.pbql:1:14: expected NOT to be followed by BETWEEN, IN, LIKE, ILIKE, or SIMILAR TO."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: BETWEEN missing AND")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT a BETWEEN b c;", "matching-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: matching-expression.pbql:1:20: expected AND between the lower and higher bounds of [NOT] BETWEEN operator."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: x IS <not identifier>")
    {
        struct function_t
        {
            char const *        f_is = nullptr;
            char const *        f_error_msg = nullptr;
        };
        function_t const function_expressions[] =
        {
            {
                "SELECT 77 IS 33;",
                "prinbee_exception: is-expression.pbql:1:14: expected one of TRUE, FALSE, NULL or DISTINCT after IS, not INTEGER.",
            },
            {
                "SELECT 77 IS NOT 33;",
                "prinbee_exception: is-expression.pbql:1:18: expected one of TRUE, FALSE, NULL or DISTINCT after IS, not INTEGER.",
            },
            {
                "SELECT 77 IS 'string';",
                "prinbee_exception: is-expression.pbql:1:14: expected one of TRUE, FALSE, NULL or DISTINCT after IS, not STRING.",
            },
            {
                "SELECT 77 IS NOT 'string';",
                "prinbee_exception: is-expression.pbql:1:18: expected one of TRUE, FALSE, NULL or DISTINCT after IS, not STRING.",
            },
            {
                "SELECT 77 IS 701.394;",
                "prinbee_exception: is-expression.pbql:1:14: expected one of TRUE, FALSE, NULL or DISTINCT after IS, not FLOATING_POINT.",
            },
            {
                "SELECT 77 IS NOT 701.394;",
                "prinbee_exception: is-expression.pbql:1:18: expected one of TRUE, FALSE, NULL or DISTINCT after IS, not FLOATING_POINT.",
            },
        };
        for(auto const & e : function_expressions)
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>(e.f_is, "is-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

SNAP_LOG_WARNING << "parsing [" << e.f_is << "]" << SNAP_LOG_SEND;
            CATCH_REQUIRE_THROWS_MATCHES(
                      parser->parse()
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(e.f_error_msg));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: x IS UNKNOWN")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT x IS UNKNOWN;", "is-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: is-expression.pbql:1:13: expected one of TRUE, FALSE, NULL or DISTINCT after IS, not UNKNOWN."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: x IS DISTINCT TO (a, b, c) instead of FROM")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT x IS DISTINCT TO (a, b, c);", "is-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: is-expression.pbql:1:22: expected FROM after IS [NOT] DISTINCT."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: x IS DISTINCT FROM (a, b, c) not yet implemented...")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT x IS DISTINCT FROM (a, b, c);", "is-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::not_yet_implemented
                , Catch::Matchers::ExceptionMessage(
                          "not_yet_implemented: IS [NOT] DISTINCT FROM is not yet implemented."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: empty string is not a number (5 + '')")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT 5 + '';", "empty-string-expression.pbql"));
        prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

        CATCH_REQUIRE_THROWS_MATCHES(
                  parser->parse()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: empty-string-expression.pbql:1:15:"
                          " the + and - binary operators expect numbers as input."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("expression_error: some strings are not numbers (+'str')")
    {
        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT + '';", "number-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                      parser->parse()
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: number-expression.pbql:1:13:"
                              " string \"\" cannot be converted to a number."));
        }

        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT + '55a';", "number-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                      parser->parse()
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: number-expression.pbql:1:16:"
                              " string \"55a\" cannot be converted to a number."));
        }

        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT + '+';", "number-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                      parser->parse()
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: number-expression.pbql:1:14:"
                              " string \"+\" cannot be converted to a number."));
        }

        {
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(std::make_shared<prinbee::pbql::input>("SELECT + '-';", "number-expression.pbql"));
            prinbee::pbql::parser::pointer_t parser(std::make_shared<prinbee::pbql::parser>(lexer));

            CATCH_REQUIRE_THROWS_MATCHES(
                      parser->parse()
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: number-expression.pbql:1:14:"
                              " string \"-\" cannot be converted to a number."));
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
