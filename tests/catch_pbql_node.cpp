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
#include    <prinbee/pbql/node.h>

#include    <prinbee/exception.h>


// self
//
#include    "catch_main.h"


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/floating_point_to_string.h>


// last include
//
#include    <snapdev/poison.h>



namespace
{





} // no name namespace



CATCH_TEST_CASE("node", "[node][pbql]")
{
    CATCH_START_SECTION("node: verify defaults")
    {
        prinbee::pbql::location l;
        std::string const filename(SNAP_CATCH2_NAMESPACE::random_string(1, 25));
        l.set_filename(filename);

        unsigned int line(0);
        SNAP_CATCH2_NAMESPACE::random(line);
        line %= 100;
        for(unsigned int idx(0); idx < line; ++idx)
        {
            l.next_line();
        }
        ++line;

        unsigned int column(0);
        SNAP_CATCH2_NAMESPACE::random(column);
        column %= 90;
        for(unsigned int idx(0); idx < column; ++idx)
        {
            l.next_column();
        }
        ++column;

        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));

        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);

        prinbee::pbql::location const & loc(n->get_location());
        CATCH_REQUIRE(loc.get_filename() == filename);
        CATCH_REQUIRE(loc.get_column() == static_cast<int>(column));
        CATCH_REQUIRE(loc.get_line() == static_cast<int>(line));

        CATCH_REQUIRE(n->get_string() == std::string());
        CATCH_REQUIRE(n->get_integer() == prinbee::int512_t());
        CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(n->get_floating_point(), 0.0L, 0.0L));
        CATCH_REQUIRE(n->get_parent() == nullptr);
        CATCH_REQUIRE(n->get_children_size() == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: verify string")
    {
        prinbee::pbql::location l;
        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));

        for(int i(0); i < 10; ++i)
        {
            std::string const identifier(SNAP_CATCH2_NAMESPACE::random_string(1, 25));
            n->set_string(identifier);
            CATCH_REQUIRE(n->get_string() == identifier);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: verify integer")
    {
        prinbee::pbql::location l;
        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));

        for(int i(0); i < 10; ++i)
        {
            prinbee::int512_t a;
            SNAP_CATCH2_NAMESPACE::rand512(a);
            n->set_integer(a);
            CATCH_REQUIRE(n->get_integer() == a);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: verify floating point")
    {
        prinbee::pbql::location l;
        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));

        for(int i(0); i < 10; ++i)
        {
            double const a(drand48());
            n->set_floating_point(a);
            CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(n->get_floating_point(), static_cast<long double>(a), 0.0L));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: verify tree")
    {
        prinbee::pbql::location l;

        // prepare nodes
        //
        prinbee::pbql::node::pointer_t root(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
        prinbee::pbql::node::pointer_t i32(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
        i32->set_integer(32);
        prinbee::pbql::node::pointer_t i54(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
        i54->set_integer(54);
        prinbee::pbql::node::pointer_t plus(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_PLUS, l));

        // build tree
        //
        root->insert_child(-1, plus);
        plus->insert_child(-1, i54);
        plus->insert_child(0, i32);    // using the index, we can insert before/after another item

        CATCH_REQUIRE(root->get_children_size() == 1);
        CATCH_REQUIRE(root->get_child(0) == plus);
        CATCH_REQUIRE(plus->get_children_size() == 2);
        CATCH_REQUIRE(plus->get_child(0) == i32);
        CATCH_REQUIRE(plus->get_child(0)->get_integer() == 32UL);
        CATCH_REQUIRE(plus->get_child(1) == i54);
        CATCH_REQUIRE(plus->get_child(1)->get_integer() == 54UL);

        prinbee::pbql::node::pointer_t i1067(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
        i1067->set_integer(1067);
        plus->set_child(0, i1067);    // using the index, we can insert before/after another item

        CATCH_REQUIRE(plus->get_children_size() == 2);
        CATCH_REQUIRE(plus->get_child(0) == i1067);
        CATCH_REQUIRE(plus->get_child(0)->get_integer() == 1067UL);
        CATCH_REQUIRE(plus->get_child(1) == i54);
        CATCH_REQUIRE(plus->get_child(1)->get_integer() == 54UL);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: check node names")
    {
        struct token_name_t
        {
            prinbee::pbql::token_t const    f_token = prinbee::pbql::token_t::TOKEN_UNKNOWN;
            char const * const              f_name = nullptr;
        };
        token_name_t token_names[] =
        {
            { prinbee::pbql::token_t::TOKEN_EOF, "EOF" },
            { prinbee::pbql::token_t::TOKEN_UNKNOWN, "UNKNOWN" },
            { prinbee::pbql::token_t::TOKEN_BITWISE_XOR, "#" },
            { prinbee::pbql::token_t::TOKEN_MODULO, "%" },
            { prinbee::pbql::token_t::TOKEN_BITWISE_AND, "&" },
            { prinbee::pbql::token_t::TOKEN_OPEN_PARENTHESIS, "(" },
            { prinbee::pbql::token_t::TOKEN_CLOSE_PARENTHESIS, ")" },
            { prinbee::pbql::token_t::TOKEN_MULTIPLY, "*" },
            { prinbee::pbql::token_t::TOKEN_PLUS, "+" },
            { prinbee::pbql::token_t::TOKEN_COMMA, "," },
            { prinbee::pbql::token_t::TOKEN_MINUS, "-" },
            { prinbee::pbql::token_t::TOKEN_PERIOD, "." },
            { prinbee::pbql::token_t::TOKEN_DIVIDE, "/" },
            { prinbee::pbql::token_t::TOKEN_COLON, ":" },
            { prinbee::pbql::token_t::TOKEN_SEMI_COLON, ";" },
            { prinbee::pbql::token_t::TOKEN_LESS, "<" },
            { prinbee::pbql::token_t::TOKEN_EQUAL, "=" },
            { prinbee::pbql::token_t::TOKEN_GREATER, ">" },
            { prinbee::pbql::token_t::TOKEN_ABSOLUTE_VALUE, "@" },
            { prinbee::pbql::token_t::TOKEN_OPEN_BRACKET, "[" },
            { prinbee::pbql::token_t::TOKEN_CLOSE_BRACKET, "]" },
            { prinbee::pbql::token_t::TOKEN_POWER, "^" },
            { prinbee::pbql::token_t::TOKEN_BITWISE_OR, "|" },
            { prinbee::pbql::token_t::TOKEN_REGULAR_EXPRESSION, "~" },
            { prinbee::pbql::token_t::TOKEN_IDENTIFIER, "IDENTIFIER" },
            { prinbee::pbql::token_t::TOKEN_STRING, "STRING" },
            { prinbee::pbql::token_t::TOKEN_INTEGER, "INTEGER" },
            { prinbee::pbql::token_t::TOKEN_FLOATING_POINT, "FLOATING_POINT" },
            { prinbee::pbql::token_t::TOKEN_NOT_EQUAL, "NOT_EQUAL" },
            { prinbee::pbql::token_t::TOKEN_LESS_EQUAL, "LESS_EQUAL" },
            { prinbee::pbql::token_t::TOKEN_GREATER_EQUAL, "GREATER_EQUAL" },
            { prinbee::pbql::token_t::TOKEN_SQUARE_ROOT, "SQUARE_ROOT" },
            { prinbee::pbql::token_t::TOKEN_CUBE_ROOT, "CUBE_ROOT" },
            { prinbee::pbql::token_t::TOKEN_SCOPE, "SCOPE" },
            { prinbee::pbql::token_t::TOKEN_SHIFT_LEFT, "SHIFT_LEFT" },
            { prinbee::pbql::token_t::TOKEN_SHIFT_RIGHT, "SHIFT_RIGHT" },
            { prinbee::pbql::token_t::TOKEN_STRING_CONCAT, "STRING_CONCAT" },
            { prinbee::pbql::token_t::TOKEN_ALL_FIELDS, "ALL_FIELDS" },
            { prinbee::pbql::token_t::TOKEN_AT, "AT" },
            { prinbee::pbql::token_t::TOKEN_BETWEEN, "BETWEEN" },
            { prinbee::pbql::token_t::TOKEN_CAST, "CAST" },
            { prinbee::pbql::token_t::TOKEN_FALSE, "FALSE" },
            { prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, "FUNCTION_CALL" },
            { prinbee::pbql::token_t::TOKEN_ILIKE, "ILIKE" },
            { prinbee::pbql::token_t::TOKEN_LIKE, "LIKE" },
            { prinbee::pbql::token_t::TOKEN_LIST, "LIST" },
            { prinbee::pbql::token_t::TOKEN_LOGICAL_OR, "LOGICAL_OR" },
            { prinbee::pbql::token_t::TOKEN_LOGICAL_AND, "LOGICAL_AND" },
            { prinbee::pbql::token_t::TOKEN_LOGICAL_NOT, "LOGICAL_NOT" },
            { prinbee::pbql::token_t::TOKEN_NULL, "NULL" },
            { prinbee::pbql::token_t::TOKEN_SIMILAR, "SIMILAR" },
            { prinbee::pbql::token_t::TOKEN_TRUE, "TRUE" },
            { prinbee::pbql::token_t::TOKEN_BOOLEAN, "BOOLEAN" },
            { prinbee::pbql::token_t::TOKEN_NUMBER, "NUMBER" },
        };

        std::string previous;
        for(auto const & t : token_names)
        {
            std::string const s(prinbee::pbql::to_string(t.f_token, false));
            if(s.empty())
            {
                SNAP_LOG_FATAL
                    << "token number "
                    << static_cast<int>(t.f_token)
                    << " is not defined in prinbee::pbql::to_string() (previous token was: "
                    << (previous.empty() ? "<no previous token>" : previous)
                    << ")."
                    << SNAP_LOG_SEND;
            }
            CATCH_REQUIRE_FALSE(s.empty());    // token missing?
            CATCH_REQUIRE(s == t.f_name);
            if(s.length() == 1)
            {
                CATCH_REQUIRE(prinbee::pbql::to_string(t.f_token, true) == "'" + s + "'");
                CATCH_REQUIRE(prinbee::pbql::to_string(t.f_token) == "'" + s + "'");
            }
            previous = s;
        }

        // also test the two "not a token" enums
        {
            std::string p(prinbee::pbql::to_string(prinbee::pbql::token_t::TOKEN_other));
            CATCH_REQUIRE(p.empty());
            p = prinbee::pbql::to_string(prinbee::pbql::token_t::TOKEN_max, false);
            CATCH_REQUIRE(p.empty());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: constructor & literals")
    {
        struct token_name_t
        {
            prinbee::pbql::token_t const    f_token = prinbee::pbql::token_t::TOKEN_UNKNOWN;
            bool                            f_literal = false;
        };
        token_name_t token_literals[] =
        {
            { prinbee::pbql::token_t::TOKEN_EOF,                false },
            { prinbee::pbql::token_t::TOKEN_BITWISE_XOR,        false },
            { prinbee::pbql::token_t::TOKEN_MODULO,             false },
            { prinbee::pbql::token_t::TOKEN_BITWISE_AND,        false },
            { prinbee::pbql::token_t::TOKEN_OPEN_PARENTHESIS,   false },
            { prinbee::pbql::token_t::TOKEN_CLOSE_PARENTHESIS,  false },
            { prinbee::pbql::token_t::TOKEN_MULTIPLY,           false },
            { prinbee::pbql::token_t::TOKEN_PLUS,               false },
            { prinbee::pbql::token_t::TOKEN_COMMA,              false },
            { prinbee::pbql::token_t::TOKEN_MINUS,              false },
            { prinbee::pbql::token_t::TOKEN_PERIOD,             false },
            { prinbee::pbql::token_t::TOKEN_DIVIDE,             false },
            { prinbee::pbql::token_t::TOKEN_COLON,              false },
            { prinbee::pbql::token_t::TOKEN_SEMI_COLON,         false },
            { prinbee::pbql::token_t::TOKEN_LESS,               false },
            { prinbee::pbql::token_t::TOKEN_EQUAL,              false },
            { prinbee::pbql::token_t::TOKEN_GREATER,            false },
            { prinbee::pbql::token_t::TOKEN_ABSOLUTE_VALUE,     false },
            { prinbee::pbql::token_t::TOKEN_OPEN_BRACKET,       false },
            { prinbee::pbql::token_t::TOKEN_CLOSE_BRACKET,      false },
            { prinbee::pbql::token_t::TOKEN_POWER,              false },
            { prinbee::pbql::token_t::TOKEN_BITWISE_OR,         false },
            { prinbee::pbql::token_t::TOKEN_REGULAR_EXPRESSION, false },
            { prinbee::pbql::token_t::TOKEN_IDENTIFIER,         false },
            { prinbee::pbql::token_t::TOKEN_STRING,             true },
            { prinbee::pbql::token_t::TOKEN_INTEGER,            true },
            { prinbee::pbql::token_t::TOKEN_FLOATING_POINT,     true },
            { prinbee::pbql::token_t::TOKEN_NOT_EQUAL,          false },
            { prinbee::pbql::token_t::TOKEN_LESS_EQUAL,         false },
            { prinbee::pbql::token_t::TOKEN_GREATER_EQUAL,      false },
            { prinbee::pbql::token_t::TOKEN_SQUARE_ROOT,        false },
            { prinbee::pbql::token_t::TOKEN_CUBE_ROOT,          false },
            { prinbee::pbql::token_t::TOKEN_SCOPE,              false },
            { prinbee::pbql::token_t::TOKEN_SHIFT_LEFT,         false },
            { prinbee::pbql::token_t::TOKEN_SHIFT_RIGHT,        false },
            { prinbee::pbql::token_t::TOKEN_STRING_CONCAT,      false },
            { prinbee::pbql::token_t::TOKEN_ALL_FIELDS,         false },
            { prinbee::pbql::token_t::TOKEN_AT,                 false },
            { prinbee::pbql::token_t::TOKEN_BETWEEN,            false },
            { prinbee::pbql::token_t::TOKEN_CAST,               false },
            { prinbee::pbql::token_t::TOKEN_FALSE,              true },
            { prinbee::pbql::token_t::TOKEN_FUNCTION_CALL,      false },
            { prinbee::pbql::token_t::TOKEN_ILIKE,              false },
            { prinbee::pbql::token_t::TOKEN_LIKE,               false },
            { prinbee::pbql::token_t::TOKEN_LIST,               false },
            { prinbee::pbql::token_t::TOKEN_LOGICAL_OR,         false },
            { prinbee::pbql::token_t::TOKEN_LOGICAL_AND,        false },
            { prinbee::pbql::token_t::TOKEN_LOGICAL_NOT,        false },
            { prinbee::pbql::token_t::TOKEN_NULL,               true },
            { prinbee::pbql::token_t::TOKEN_SIMILAR,            false },
            { prinbee::pbql::token_t::TOKEN_TRUE,               true },
        };

        for(auto const & t : token_literals)
        {
            //char const * const name(prinbee::pbql::to_string(t.f_token));
            prinbee::pbql::location l;
            prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(t.f_token, l));
            if(t.f_literal)
            {
                CATCH_REQUIRE(n->is_literal());
                CATCH_REQUIRE_FALSE(n->is_literal(prinbee::pbql::token_t::TOKEN_EOF));
                switch(t.f_token)
                {
                case prinbee::pbql::token_t::TOKEN_STRING:
                    CATCH_REQUIRE(n->is_literal(t.f_token));
                    n->set_string("this is the string being tested");
                    n->set_integer(1018);
                    n->set_floating_point(3.14159);
                    CATCH_REQUIRE(n->get_string_auto_convert() == "this is the string being tested");
                    break;

                case prinbee::pbql::token_t::TOKEN_INTEGER:
                    CATCH_REQUIRE(n->is_literal(t.f_token));
                    n->set_string("this string must be ignored");
                    n->set_integer(7081);
                    n->set_floating_point(1.141);
                    CATCH_REQUIRE(n->get_string_auto_convert() == "7081");
                    break;

                case prinbee::pbql::token_t::TOKEN_FLOATING_POINT:
                    CATCH_REQUIRE(n->is_literal(t.f_token));
                    n->set_string("ignore this with floating points");
                    n->set_integer(19362);
                    n->set_floating_point(101.994);
                    CATCH_REQUIRE(n->get_string_auto_convert() == "101.994");
                    break;

                case prinbee::pbql::token_t::TOKEN_TRUE:
                    CATCH_REQUIRE(n->is_literal(prinbee::pbql::token_t::TOKEN_BOOLEAN));
                    n->set_string("strings are not Boolean");
                    n->set_integer(320);
                    n->set_floating_point(91.321);
                    CATCH_REQUIRE(n->get_string_auto_convert() == "true");
                    break;

                case prinbee::pbql::token_t::TOKEN_FALSE:
                    CATCH_REQUIRE(n->is_literal(prinbee::pbql::token_t::TOKEN_BOOLEAN));
                    n->set_string("strings are still not Boolean");
                    n->set_integer(60320);
                    n->set_floating_point(3291.02501);
                    CATCH_REQUIRE(n->get_string_auto_convert() == "false");
                    break;

                case prinbee::pbql::token_t::TOKEN_NULL:
                    CATCH_REQUIRE(n->is_literal(t.f_token));
                    n->set_string("strings are still not NULL");
                    n->set_integer(90021);
                    n->set_floating_point(412.10328);
                    CATCH_REQUIRE(n->get_string_auto_convert() == "null");
                    break;

                default:
                    CATCH_FAIL();

                }
            }
            else
            {
                CATCH_REQUIRE_FALSE(n->is_literal());
                CATCH_REQUIRE_FALSE(n->is_literal(prinbee::pbql::token_t::TOKEN_EOF));
                CATCH_REQUIRE_FALSE(n->is_literal(t.f_token));
                CATCH_REQUIRE_THROWS_MATCHES(
                          n->get_string_auto_convert()
                        , prinbee::logic_error
                        , Catch::Matchers::ExceptionMessage(
                                  "logic_error: node is not a literal and it cannot be converted to a string."));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: to_as2js()")
    {
        constexpr std::int64_t const value_parent_integer(987);
        constexpr std::int64_t const value_left_integer(1191);
        constexpr std::int64_t const value_right_integer(6731);
        constexpr std::int64_t const value_extra_integer(92426);
        constexpr double const value_parent_double(0.0238);
        constexpr double const value_left_double(9.3321);
        constexpr double const value_right_double(11.65103);
        constexpr double const value_extra_double(151.5931);
        std::string const value_parent_string("the parent string value");
        std::string const value_left_string("the left side string value");
        std::string const value_right_string("the right side string value");
        std::string const value_extra_string("the extra string value");
        std::string const value_parent_identifier("top_identifier");
        std::string const value_left_identifier("left_identifier");
        std::string const value_right_identifier("right_identifier");
        std::string const value_extra_identifier("extra_identifier");
        struct expression_t
        {
            prinbee::pbql::token_t const    f_parent = prinbee::pbql::token_t::TOKEN_UNKNOWN;
            prinbee::pbql::token_t const    f_left = prinbee::pbql::token_t::TOKEN_UNKNOWN;
            prinbee::pbql::token_t const    f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN;
            prinbee::pbql::token_t const    f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN;
            std::string const               f_output = std::string();
        };
        expression_t const expressions[] =
        {
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_BITWISE_XOR,
                .f_left = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_right = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = std::to_string(value_left_integer) + '^' + std::to_string(value_right_integer),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_MODULO,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = snapdev::floating_point_to_string<double, char>(value_left_double)
                          + '%'
                          + snapdev::floating_point_to_string<double, char>(value_right_double),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_BITWISE_AND,
                .f_left = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_right = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = std::to_string(value_left_integer) + '&' + std::to_string(value_right_integer),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_MULTIPLY,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = snapdev::floating_point_to_string<double, char>(value_left_double)
                          + '*'
                          + snapdev::floating_point_to_string<double, char>(value_right_double),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_MINUS,
                .f_left = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = '-' + std::to_string(value_left_integer),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_MINUS,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = '-' + snapdev::floating_point_to_string<double, char>(value_left_double),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_PERIOD,
                .f_left = prinbee::pbql::token_t::TOKEN_IDENTIFIER,
                .f_right = prinbee::pbql::token_t::TOKEN_IDENTIFIER,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = value_left_identifier + '.' + value_right_identifier,
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_DIVIDE,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = snapdev::floating_point_to_string<double, char>(value_left_double)
                          + '/'
                          + snapdev::floating_point_to_string<double, char>(value_right_double),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_LESS,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = snapdev::floating_point_to_string<double, char>(value_left_double)
                          + '<'
                          + snapdev::floating_point_to_string<double, char>(value_right_double),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_EQUAL,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = snapdev::floating_point_to_string<double, char>(value_left_double)
                          + "=="
                          + snapdev::floating_point_to_string<double, char>(value_right_double),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_GREATER,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = snapdev::floating_point_to_string<double, char>(value_left_double)
                          + '>'
                          + snapdev::floating_point_to_string<double, char>(value_right_double),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_POWER,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = '('
                          + snapdev::floating_point_to_string<double, char>(value_left_double)
                          + "**"
                          + snapdev::floating_point_to_string<double, char>(value_right_double)
                          + ')',
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_BITWISE_OR,
                .f_left = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_right = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = std::to_string(value_left_integer) + '|' + std::to_string(value_right_integer),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_REGULAR_EXPRESSION,
                .f_left = prinbee::pbql::token_t::TOKEN_STRING,
                .f_right = prinbee::pbql::token_t::TOKEN_STRING,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = "new RegExp(\""
                          + value_right_string
                          + "\").test(\""
                          + value_left_string
                          + "\")",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_IDENTIFIER,
                .f_left = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = value_parent_identifier,
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_STRING,
                .f_left = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = '"' + value_parent_string + '"',
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_left = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = "987",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_left = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = "0.0238",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_NOT_EQUAL,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = snapdev::floating_point_to_string<double, char>(value_left_double)
                          + "!="
                          + snapdev::floating_point_to_string<double, char>(value_right_double),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_LESS_EQUAL,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = snapdev::floating_point_to_string<double, char>(value_left_double)
                          + "<="
                          + snapdev::floating_point_to_string<double, char>(value_right_double),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_GREATER_EQUAL,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = snapdev::floating_point_to_string<double, char>(value_left_double)
                          + ">="
                          + snapdev::floating_point_to_string<double, char>(value_right_double),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_SHIFT_LEFT,
                .f_left = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_right = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = std::to_string(value_left_integer)
                          + "<<"
                          + std::to_string(value_right_integer),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_SHIFT_RIGHT,
                .f_left = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_right = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = std::to_string(value_left_integer)
                          + ">>"
                          + std::to_string(value_right_integer),
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_ALL_FIELDS,
                .f_left = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = "ALL_FIELDS",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_AT,
                .f_left = prinbee::pbql::token_t::TOKEN_IDENTIFIER,
                .f_right = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = value_left_identifier + '[' + std::to_string(value_right_integer) + ']',
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_BETWEEN,
                .f_left = prinbee::pbql::token_t::TOKEN_IDENTIFIER,
                .f_right = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_extra = prinbee::pbql::token_t::TOKEN_INTEGER,
                .f_output = "(_t1="
                          + value_left_identifier
                          + ",_t1>="
                          + std::to_string(value_right_integer)
                          + "&&_t1<="
                          + std::to_string(value_extra_integer)
                          + ')',
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_BETWEEN,
                .f_left = prinbee::pbql::token_t::TOKEN_IDENTIFIER,
                .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_extra = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_output = "(_t1="
                          + value_left_identifier
                          + ",_t1>="
                          + snapdev::floating_point_to_string<double, char>(value_right_double)
                          + "&&_t1<="
                          + snapdev::floating_point_to_string<double, char>(value_extra_double)
                          + ')',
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_BETWEEN,
                .f_left = prinbee::pbql::token_t::TOKEN_IDENTIFIER,
                .f_right = prinbee::pbql::token_t::TOKEN_STRING,
                .f_extra = prinbee::pbql::token_t::TOKEN_STRING,
                .f_output = "(_t1="
                          + value_left_identifier
                          + ",_t1>=\""
                          + value_right_string
                          + "\"&&_t1<=\""
                          + value_extra_string
                          + "\")",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_CAST,
                .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = "new "
                          + value_parent_identifier
                          + '('
                          + snapdev::floating_point_to_string<double, char>(value_left_double)
                          + ')',
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_FALSE,
                .f_left = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = "false",
            },
            //{
            //    .f_parent = prinbee::pbql::token_t::TOKEN_FUNCTION_CALL,
            //    .f_left = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
            //    .f_right = prinbee::pbql::token_t::TOKEN_FLOATING_POINT,
            //    .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
            //    .f_output = snapdev::floating_point_to_string<double, char>(value_left_double)
            //              + "<="
            //              + snapdev::floating_point_to_string<double, char>(value_right_double),
            //},
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_LOGICAL_OR,
                .f_left = prinbee::pbql::token_t::TOKEN_TRUE,
                .f_right = prinbee::pbql::token_t::TOKEN_FALSE,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = "true||false",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_LOGICAL_AND,
                .f_left = prinbee::pbql::token_t::TOKEN_FALSE,
                .f_right = prinbee::pbql::token_t::TOKEN_TRUE,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = "false&&true",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_LOGICAL_NOT,
                .f_left = prinbee::pbql::token_t::TOKEN_FALSE,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = "!false",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_NULL,
                .f_left = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = "null",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_TRUE,
                .f_left = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_right = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = "true",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_ILIKE,
                .f_left = prinbee::pbql::token_t::TOKEN_STRING,
                .f_right = prinbee::pbql::token_t::TOKEN_STRING,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = '/'
                          + value_right_string
                          + "/i.test(\""
                          + value_left_string
                          + "\")",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_LIKE,
                .f_left = prinbee::pbql::token_t::TOKEN_STRING,
                .f_right = prinbee::pbql::token_t::TOKEN_STRING,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = '/'
                          + value_right_string
                          + "/.test(\""
                          + value_left_string
                          + "\")",
            },
            {
                .f_parent = prinbee::pbql::token_t::TOKEN_SIMILAR,
                .f_left = prinbee::pbql::token_t::TOKEN_STRING,
                .f_right = prinbee::pbql::token_t::TOKEN_STRING,
                .f_extra = prinbee::pbql::token_t::TOKEN_UNKNOWN,
                .f_output = '/'
                          + value_right_string
                          + "/.test(\""
                          + value_left_string
                          + "\")",
            },
        };

        for(auto const & e : expressions)
        {
            prinbee::pbql::location l;

            prinbee::pbql::node::pointer_t parent(std::make_shared<prinbee::pbql::node>(e.f_parent, l));
            parent->set_integer(value_parent_integer);
            parent->set_floating_point(value_parent_double);
            if(e.f_parent == prinbee::pbql::token_t::TOKEN_IDENTIFIER
            || e.f_parent == prinbee::pbql::token_t::TOKEN_CAST)
            {
                parent->set_string(value_parent_identifier);
            }
            else
            {
                parent->set_string(value_parent_string);
            }

            if(e.f_left != prinbee::pbql::token_t::TOKEN_UNKNOWN)
            {
                prinbee::pbql::node::pointer_t left(std::make_shared<prinbee::pbql::node>(e.f_left, l));
                left->set_integer(value_left_integer);
                left->set_floating_point(value_left_double);
                if(e.f_left == prinbee::pbql::token_t::TOKEN_IDENTIFIER)
                {
                    left->set_string(value_left_identifier);
                }
                else
                {
                    left->set_string(value_left_string);
                }

                parent->insert_child(-1, left);

                if(e.f_right != prinbee::pbql::token_t::TOKEN_UNKNOWN)
                {
                    prinbee::pbql::node::pointer_t right(std::make_shared<prinbee::pbql::node>(e.f_right, l));
                    right->set_integer(value_right_integer);
                    right->set_floating_point(value_right_double);
                    if(e.f_right == prinbee::pbql::token_t::TOKEN_IDENTIFIER)
                    {
                        right->set_string(value_right_identifier);
                    }
                    else
                    {
                        right->set_string(value_right_string);
                    }

                    parent->insert_child(-1, right);

                    if(e.f_extra != prinbee::pbql::token_t::TOKEN_UNKNOWN)
                    {
                        prinbee::pbql::node::pointer_t extra(std::make_shared<prinbee::pbql::node>(e.f_extra, l));
                        extra->set_integer(value_extra_integer);
                        extra->set_floating_point(value_extra_double);
                        if(e.f_extra == prinbee::pbql::token_t::TOKEN_IDENTIFIER)
                        {
                            right->set_string(value_extra_identifier);
                        }
                        else
                        {
                            extra->set_string(value_extra_string);
                        }

                        parent->insert_child(-1, extra);
                    }
                }
                else if(e.f_extra != prinbee::pbql::token_t::TOKEN_UNKNOWN)
                {
                    // you need a right to have an extra
                    //
                    CATCH_FAIL();
                }
            }
            else if(e.f_right != prinbee::pbql::token_t::TOKEN_UNKNOWN)
            {
                // you need a left to have a right
                //
                CATCH_FAIL();
            }
            std::string const result(parent->to_as2js());
            CATCH_REQUIRE(result == e.f_output);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: string with escape codes to_as2js()")
    {
        struct string_expression_t
        {
            char const *        f_in = nullptr;
            char const *        f_out = nullptr;
        };
        string_expression_t const strings[] =
        {
            {
                .f_in = "test",
                .f_out = "\"test\"",
            },
            {
                .f_in = "double \" quote",
                .f_out = "\"double \\\" quote\"",
            },
            {
                .f_in = "\back",
                .f_out = "\"\\back\"",
            },
            {
                .f_in = "\forward",
                .f_out = "\"\\forward\"",
            },
            {
                .f_in = "\newline",
                .f_out = "\"\\newline\"",
            },
            {
                .f_in = "\return",
                .f_out = "\"\\return\"",
            },
            {
                .f_in = "\tab",
                .f_out = "\"\\tab\"",
            },
            {
                .f_in = "\vertical",
                .f_out = "\"\\vertical\"",
            },
            { // kept as is
                .f_in = "\001 Ctrl-A",
                .f_out = "\"\001 Ctrl-A\"",
            },
        };

        for(auto const & s : strings)
        {
            prinbee::pbql::location l;

            prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            n->set_string(s.f_in);
            std::string const result(n->to_as2js());
            CATCH_REQUIRE(result == s.f_out);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: function calls 0 to 10 parameters")
    {
        prinbee::pbql::location l;

        // 0 parameters
        {
            prinbee::pbql::node::pointer_t f(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, l));
            f->set_string("myFunc");
            prinbee::pbql::node::pointer_t list(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIST, l));
            f->insert_child(-1, list);
            std::string const result(f->to_as2js());
            CATCH_REQUIRE(result == "myFunc()");
        }

        // 1 parameter
        {
            prinbee::pbql::node::pointer_t f(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, l));
            f->set_string("sin");
            prinbee::pbql::node::pointer_t list(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIST, l));
            f->insert_child(-1, list);
            prinbee::pbql::node::pointer_t p1(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p1->set_floating_point(3.14159);
            list->insert_child(-1, p1);
            std::string const result(f->to_as2js());
            CATCH_REQUIRE(result == "sin(3.14159)");
        }

        // 2 parameters
        {
            prinbee::pbql::node::pointer_t f(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, l));
            f->set_string("Math.atan2");
            prinbee::pbql::node::pointer_t list(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIST, l));
            f->insert_child(-1, list);
            prinbee::pbql::node::pointer_t p1(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p1->set_floating_point(0.56172);
            list->insert_child(-1, p1);
            prinbee::pbql::node::pointer_t p2(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p2->set_floating_point(0.29819);
            list->insert_child(-1, p2);
            std::string const result(f->to_as2js());
            CATCH_REQUIRE(result == "Math.atan2(0.56172,0.29819)");
        }

        // 3 parameters
        {
            prinbee::pbql::node::pointer_t f(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, l));
            f->set_string("String.concat");
            prinbee::pbql::node::pointer_t list(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIST, l));
            f->insert_child(-1, list);
            prinbee::pbql::node::pointer_t p1(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            p1->set_string("prefix");
            list->insert_child(-1, p1);
            prinbee::pbql::node::pointer_t p2(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p2->set_floating_point(9180.21911);
            list->insert_child(-1, p2);
            prinbee::pbql::node::pointer_t p3(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
            p3->set_integer(290119);
            list->insert_child(-1, p3);
            std::string const result(f->to_as2js());
            CATCH_REQUIRE(result == "String.concat(\"prefix\",9180.21911,290119)");
        }

        // 4 parameters
        {
            prinbee::pbql::node::pointer_t f(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, l));
            f->set_string("hisFunc");
            prinbee::pbql::node::pointer_t list(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIST, l));
            f->insert_child(-1, list);
            prinbee::pbql::node::pointer_t p1(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            p1->set_string("prefix");
            list->insert_child(-1, p1);
            prinbee::pbql::node::pointer_t p2(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            p2->set_string("left");
            list->insert_child(-1, p2);
            prinbee::pbql::node::pointer_t p3(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            p3->set_string("right");
            list->insert_child(-1, p3);
            prinbee::pbql::node::pointer_t p4(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            p4->set_string("suffix");
            list->insert_child(-1, p4);
            std::string const result(f->to_as2js());
            CATCH_REQUIRE(result == "hisFunc(\"prefix\",\"left\",\"right\",\"suffix\")");
        }

        // 5 parameters
        {
            prinbee::pbql::node::pointer_t f(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, l));
            f->set_string("Math.min");
            prinbee::pbql::node::pointer_t list(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIST, l));
            f->insert_child(-1, list);
            prinbee::pbql::node::pointer_t p1(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
            p1->set_integer(55);
            list->insert_child(-1, p1);
            prinbee::pbql::node::pointer_t p2(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
            p2->set_integer(101);
            list->insert_child(-1, p2);
            prinbee::pbql::node::pointer_t p3(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
            p3->set_integer(-67);
            list->insert_child(-1, p3);
            prinbee::pbql::node::pointer_t p4(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
            p4->set_integer(31);
            list->insert_child(-1, p4);
            prinbee::pbql::node::pointer_t p5(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
            p5->set_integer(-96);
            list->insert_child(-1, p5);
            std::string const result(f->to_as2js());
            CATCH_REQUIRE(result == "Math.min(55,101,-67,31,-96)");
        }

        // 6 parameters
        {
            prinbee::pbql::node::pointer_t f(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, l));
            f->set_string("Math.max");
            prinbee::pbql::node::pointer_t list(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIST, l));
            f->insert_child(-1, list);
            prinbee::pbql::node::pointer_t p1(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p1->set_floating_point(5.5);
            list->insert_child(-1, p1);
            prinbee::pbql::node::pointer_t p2(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p2->set_floating_point(10.21);
            list->insert_child(-1, p2);
            prinbee::pbql::node::pointer_t p3(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p3->set_floating_point(-6.7);
            list->insert_child(-1, p3);
            prinbee::pbql::node::pointer_t p4(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p4->set_floating_point(3.1);
            list->insert_child(-1, p4);
            prinbee::pbql::node::pointer_t p5(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p5->set_floating_point(-0.96);
            list->insert_child(-1, p5);
            prinbee::pbql::node::pointer_t p6(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p6->set_floating_point(96.689);
            list->insert_child(-1, p6);
            std::string const result(f->to_as2js());
            CATCH_REQUIRE(result == "Math.max(5.5,10.21,-6.7,3.1,-0.96,96.689)");
        }

        // 7 parameters
        {
            prinbee::pbql::node::pointer_t f(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, l));
            f->set_string("yourFunction");
            prinbee::pbql::node::pointer_t list(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIST, l));
            f->insert_child(-1, list);
            prinbee::pbql::node::pointer_t p1(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
            p1->set_integer(159);
            list->insert_child(-1, p1);
            prinbee::pbql::node::pointer_t p2(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p2->set_floating_point(10.21);
            list->insert_child(-1, p2);
            prinbee::pbql::node::pointer_t p3(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p3->set_floating_point(-6.7);
            list->insert_child(-1, p3);
            prinbee::pbql::node::pointer_t p4(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            p4->set_string("this string");
            list->insert_child(-1, p4);
            prinbee::pbql::node::pointer_t p5(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p5->set_floating_point(-0.96);
            list->insert_child(-1, p5);
            prinbee::pbql::node::pointer_t p6(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p6->set_floating_point(96.689);
            list->insert_child(-1, p6);
            prinbee::pbql::node::pointer_t p7(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_TRUE, l));
            list->insert_child(-1, p7);
            std::string const result(f->to_as2js());
            CATCH_REQUIRE(result == "yourFunction(159,10.21,-6.7,\"this string\",-0.96,96.689,true)");
        }

        // 8 parameters
        {
            prinbee::pbql::node::pointer_t f(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, l));
            f->set_string("getProperties");
            prinbee::pbql::node::pointer_t list(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIST, l));
            f->insert_child(-1, list);
            prinbee::pbql::node::pointer_t p1(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            p1->set_string("x");
            list->insert_child(-1, p1);
            prinbee::pbql::node::pointer_t p2(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
            p2->set_integer(1012);
            list->insert_child(-1, p2);
            prinbee::pbql::node::pointer_t p3(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FALSE, l));
            list->insert_child(-1, p3);
            prinbee::pbql::node::pointer_t p4(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p4->set_floating_point(3.1);
            list->insert_child(-1, p4);
            prinbee::pbql::node::pointer_t p5(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_TRUE, l));
            list->insert_child(-1, p5);
            prinbee::pbql::node::pointer_t p6(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p6->set_floating_point(96.689);
            list->insert_child(-1, p6);
            prinbee::pbql::node::pointer_t p7(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            p7->set_string("one before last");
            list->insert_child(-1, p7);
            prinbee::pbql::node::pointer_t p8(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p8->set_floating_point(0.002);
            list->insert_child(-1, p8);
            std::string const result(f->to_as2js());
            CATCH_REQUIRE(result == "getProperties(\"x\",1012,false,3.1,true,96.689,\"one before last\",0.002)");
        }

        // 9 parameters
        {
            prinbee::pbql::node::pointer_t f(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, l));
            f->set_string("Math.average");
            prinbee::pbql::node::pointer_t list(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIST, l));
            f->insert_child(-1, list);
            prinbee::pbql::node::pointer_t p1(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p1->set_floating_point(501.3);
            list->insert_child(-1, p1);
            prinbee::pbql::node::pointer_t p2(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p2->set_floating_point(1.012);
            list->insert_child(-1, p2);
            prinbee::pbql::node::pointer_t p3(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p3->set_floating_point(7.902);
            list->insert_child(-1, p3);
            prinbee::pbql::node::pointer_t p4(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p4->set_floating_point(3.1);
            list->insert_child(-1, p4);
            prinbee::pbql::node::pointer_t p5(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p5->set_floating_point(907.231);
            list->insert_child(-1, p5);
            prinbee::pbql::node::pointer_t p6(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p6->set_floating_point(96.689);
            list->insert_child(-1, p6);
            prinbee::pbql::node::pointer_t p7(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p7->set_floating_point(1.0216);
            list->insert_child(-1, p7);
            prinbee::pbql::node::pointer_t p8(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p8->set_floating_point(0.002);
            list->insert_child(-1, p8);
            prinbee::pbql::node::pointer_t p9(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p9->set_floating_point(0.202);
            list->insert_child(-1, p9);
            std::string const result(f->to_as2js());
            CATCH_REQUIRE(result == "Math.average(501.3,1.012,7.902,3.1,907.231,96.689,1.0216,0.002,0.202)");
        }

        // 10 parameters
        {
            prinbee::pbql::node::pointer_t f(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FUNCTION_CALL, l));
            f->set_string("Math.sum");
            prinbee::pbql::node::pointer_t list(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIST, l));
            f->insert_child(-1, list);
            prinbee::pbql::node::pointer_t p1(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p1->set_floating_point(501.3);
            list->insert_child(-1, p1);
            prinbee::pbql::node::pointer_t p2(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p2->set_floating_point(1.012);
            list->insert_child(-1, p2);
            prinbee::pbql::node::pointer_t p3(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p3->set_floating_point(7.902);
            list->insert_child(-1, p3);
            prinbee::pbql::node::pointer_t p4(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p4->set_floating_point(3.1);
            list->insert_child(-1, p4);
            prinbee::pbql::node::pointer_t p5(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p5->set_floating_point(907.231);
            list->insert_child(-1, p5);
            prinbee::pbql::node::pointer_t p6(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p6->set_floating_point(96.689);
            list->insert_child(-1, p6);
            prinbee::pbql::node::pointer_t p7(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p7->set_floating_point(1.0216);
            list->insert_child(-1, p7);
            prinbee::pbql::node::pointer_t p8(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p8->set_floating_point(0.002);
            list->insert_child(-1, p8);
            prinbee::pbql::node::pointer_t p9(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p9->set_floating_point(0.202);
            list->insert_child(-1, p9);
            prinbee::pbql::node::pointer_t p10(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_FLOATING_POINT, l));
            p10->set_floating_point(0.202);
            list->insert_child(-1, p10);
            std::string const result(f->to_as2js());
            CATCH_REQUIRE(result == "Math.sum(501.3,1.012,7.902,3.1,907.231,96.689,1.0216,0.002,0.202,0.202)");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: LIKE, ILIKE, SIMILAR to regex")
    {
        prinbee::pbql::location l;

        // make sure all the characters except '%' are properly escaped (LIKE)
        {
            prinbee::pbql::node::pointer_t like(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_LIKE, l));
            prinbee::pbql::node::pointer_t value(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            value->set_string("value being checked using ILIKE");
            like->insert_child(-1, value);
            prinbee::pbql::node::pointer_t pattern(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            pattern->set_string("/(LIKE+ | p_a_t_t_e_r_n? \\with* % and [nothing] {el,se}.)/");
            like->insert_child(-1, pattern);
            std::string const result(like->to_as2js());
            CATCH_REQUIRE(result == "/\\/\\(LIKE\\+ \\| p_a_t_t_e_r_n\\? \\\\with\\* .* and \\[nothing\\] \\{el,se\\}\\.\\)\\//.test(\"value being checked using ILIKE\")");
        }

        // make sure all the characters except '%' are properly escaped (ILIKE)
        {
            prinbee::pbql::node::pointer_t ilike(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_ILIKE, l));
            prinbee::pbql::node::pointer_t value(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            value->set_string("value being checked using ILIKE");
            ilike->insert_child(-1, value);
            prinbee::pbql::node::pointer_t pattern(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            pattern->set_string("/(LIKE+ | p_a_t_t_e_r_n? \\with* % and [nothing] {el,se}.)/");
            ilike->insert_child(-1, pattern);
            std::string const result(ilike->to_as2js());
            CATCH_REQUIRE(result == "/\\/\\(LIKE\\+ \\| p_a_t_t_e_r_n\\? \\\\with\\* .* and \\[nothing\\] \\{el,se\\}\\.\\)\\//i.test(\"value being checked using ILIKE\")");
        }

        // make sure all the special characters properly converted (SIMILAR)
        {
            prinbee::pbql::node::pointer_t similar(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_SIMILAR, l));
            prinbee::pbql::node::pointer_t value(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            value->set_string("value being checked using SIMILAR");
            similar->insert_child(-1, value);
            prinbee::pbql::node::pointer_t pattern(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
            pattern->set_string("(plus)+ (asterisk)* any:% [a-z0-9]? []]+ any+repeat:_* (one{1} | two{5,} | three{3,9})?");
            similar->insert_child(-1, pattern);
            std::string const result(similar->to_as2js());
            CATCH_REQUIRE(result == "/(plus)+ (asterisk)* any:.* [a-z0-9]? []]+ any+repeat:.* (one{1} | two{5,} | three{3,9})?/.test(\"value being checked using SIMILAR\")");
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("node_error", "[node][pbql][error]")
{
    CATCH_START_SECTION("node_error: invalid token")
    {
        prinbee::pbql::token_t const invalid_token[] = {
            prinbee::pbql::token_t::TOKEN_UNKNOWN,
            prinbee::pbql::token_t::TOKEN_other,
            prinbee::pbql::token_t::TOKEN_max,
        };

        prinbee::pbql::location l;
        for(std::size_t idx(0); idx < std::size(invalid_token); ++idx)
        {
            CATCH_REQUIRE_THROWS_MATCHES(
                      std::make_shared<prinbee::pbql::node>(invalid_token[idx], l)
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: node created with an invalid token ("
                            + std::to_string(static_cast<int>(invalid_token[idx]))
                            + ")."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node_error: child not found")
    {
        prinbee::pbql::location l;
        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
        CATCH_REQUIRE_THROWS_MATCHES(
                  n->get_child(0)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                          "out_of_range: child 0 does not exist."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node_error: insert child at wrong position")
    {
        prinbee::pbql::location l;
        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
        prinbee::pbql::node::pointer_t c(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
        CATCH_REQUIRE_THROWS_MATCHES(
                  n->insert_child(3, c)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                          "out_of_range: child 3 does not exist."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node_error: set child at wrong position")
    {
        prinbee::pbql::location l;
        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
        prinbee::pbql::node::pointer_t c1(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
        n->insert_child(-1, c1);
        prinbee::pbql::node::pointer_t c2(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
        n->set_child(0, c2);

        for(int i(1); i < 10; ++i)
        {
            prinbee::pbql::node::pointer_t c3(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
            CATCH_REQUIRE_THROWS_MATCHES(
                      n->set_child(i, c3)
                    , prinbee::out_of_range
                    , Catch::Matchers::ExceptionMessage(
                              "out_of_range: child "
                            + std::to_string(i)
                            + " does not exist, it cannot be replaced."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node_error: tokens that cannot be converted to as2js script")
    {
        struct unexpected_token_t
        {
            prinbee::pbql::token_t      f_token = prinbee::pbql::token_t::TOKEN_UNKNOWN;
            char const *                f_name = nullptr;
        };
        unexpected_token_t unexpected_tokens[] =
        {
            {
                .f_token = prinbee::pbql::token_t::TOKEN_EOF,
                .f_name = "EOF",
            },
            {
                .f_token = prinbee::pbql::token_t::TOKEN_OPEN_PARENTHESIS,
                .f_name = "(",
            },
            {
                .f_token = prinbee::pbql::token_t::TOKEN_CLOSE_PARENTHESIS,
                .f_name = ")",
            },
            {
                .f_token = prinbee::pbql::token_t::TOKEN_COMMA,
                .f_name = ",",
            },
            {
                .f_token = prinbee::pbql::token_t::TOKEN_COLON,
                .f_name = ":",
            },
            {
                .f_token = prinbee::pbql::token_t::TOKEN_SEMI_COLON,
                .f_name = ";",
            },
            {
                .f_token = prinbee::pbql::token_t::TOKEN_ABSOLUTE_VALUE,
                .f_name = "@",
            },
            {
                .f_token = prinbee::pbql::token_t::TOKEN_OPEN_BRACKET,
                .f_name = "[",
            },
            {
                .f_token = prinbee::pbql::token_t::TOKEN_CLOSE_BRACKET,
                .f_name = "]",
            },
            {
                .f_token = prinbee::pbql::token_t::TOKEN_SCOPE,
                .f_name = "SCOPE",
            },
            {
                .f_token = prinbee::pbql::token_t::TOKEN_STRING_CONCAT,
                .f_name = "STRING_CONCAT",
            },
            {
                .f_token = prinbee::pbql::token_t::TOKEN_LIST,
                .f_name = "LIST",
            },
        };
        for(auto const & u : unexpected_tokens)
        {
            prinbee::pbql::location l;
            prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(u.f_token, l));
            CATCH_REQUIRE_THROWS_MATCHES(
                      n->to_as2js()
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: node token type cannot be converted to as2js script ("
                            + (u.f_name[1] == '\0' ? std::string(1, '\'') + u.f_name + '\'' : std::string(u.f_name))
                            + ")."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: SIMILAR with invalid patterns")
    {
        prinbee::pbql::location l;

        // |, *, +, ? cannot be at the start of a pattern
        {
            char const repeat[] = { '|', '*', '+', '?' };
            for(auto const r : repeat)
            {
                prinbee::pbql::node::pointer_t similar(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_SIMILAR, l));
                prinbee::pbql::node::pointer_t value(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
                value->set_string("ignored");
                similar->insert_child(-1, value);
                prinbee::pbql::node::pointer_t pattern(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
                std::string p;
                p += r;
                p += " and some other stuff";
                pattern->set_string(p);
                similar->insert_child(-1, pattern);

                CATCH_REQUIRE_THROWS_MATCHES(
                          similar->to_as2js()
                        , prinbee::invalid_token
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: 1:1: SIMILAR pattern characters '|', '*', '+', '?' cannot appear at the start of a pattern."));
            }
        }

        // '{...}' cannot be at the start of a pattern
        {
            char const * bad_repeat[] = {
                "{3} exact repeat",
                "{3,} three or more",
                "{3,9} three to nine",
            };
            for(auto const r : bad_repeat)
            {
                prinbee::pbql::node::pointer_t similar(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_SIMILAR, l));
                prinbee::pbql::node::pointer_t value(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
                value->set_string("ignored");
                similar->insert_child(-1, value);
                prinbee::pbql::node::pointer_t pattern(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
                pattern->set_string(r);
                similar->insert_child(-1, pattern);

                CATCH_REQUIRE_THROWS_MATCHES(
                          similar->to_as2js()
                        , prinbee::invalid_token
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: 1:1: SIMILAR pattern character '{' cannot appear at the start of a pattern."));
            }
        }

        // '{...}', '(...)', '[...]' not closed
        {
            struct not_closed_t
            {
                char const *        f_bad_pattern = nullptr;
                char                f_missing_close = '?';
            };
            not_closed_t not_closed[] = {
                { "foo{", '}' },
                { "foo{3 oops", '}' },
                { "foo{3, oh!", '}' },
                { "foo{3,9 duh", '}' },
                { "(", ')' },
                { "(group all of this", ')' },
                { "[a-z and more", ']' },
                { "[", ']' },
                { "[]a-z and more", ']' },
            };
            for(auto const r : not_closed)
            {
                prinbee::pbql::node::pointer_t similar(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_SIMILAR, l));
                prinbee::pbql::node::pointer_t value(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
                value->set_string("ignored");
                similar->insert_child(-1, value);
                prinbee::pbql::node::pointer_t pattern(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
                pattern->set_string(r.f_bad_pattern);
                similar->insert_child(-1, pattern);

                CATCH_REQUIRE_THROWS_MATCHES(
                          similar->to_as2js()
                        , prinbee::invalid_token
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: 1:1: SIMILAR pattern missing closing bracket ('"
                                + std::string(1, r.f_missing_close)
                                + "' not found or there were no characters within those brackets)."));
            }
        }

        // '{}', '()' empty
        {
            struct not_closed_t
            {
                char const *        f_bad_pattern = nullptr;
                char                f_missing_close = '?';
            };
            not_closed_t not_closed[] = {
                { "foo{} empty?", '}' },
                { "() empty?", ')' },
            };
            for(auto const r : not_closed)
            {
                prinbee::pbql::node::pointer_t similar(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_SIMILAR, l));
                prinbee::pbql::node::pointer_t value(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
                value->set_string("ignored");
                similar->insert_child(-1, value);
                prinbee::pbql::node::pointer_t pattern(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_STRING, l));
                pattern->set_string(r.f_bad_pattern);
                similar->insert_child(-1, pattern);

                CATCH_REQUIRE_THROWS_MATCHES(
                          similar->to_as2js()
                        , prinbee::invalid_token
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: 1:1: SIMILAR found empty pattern between brackets ('"
                                + std::string(1, r.f_missing_close)
                                + "')."));
            }
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
