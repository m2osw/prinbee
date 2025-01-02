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
#include    <prinbee/pbql/lexer.h>

#include    <prinbee/exception.h>


// self
//
#include    "catch_main.h"


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
//#include    <snapdev/hexadecimal_string.h>
//#include    <snapdev/math.h>
//#include    <snapdev/ostream_int128.h>


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



CATCH_TEST_CASE("lexer", "[lexer] [pbql]")
{
    CATCH_START_SECTION("lexer: verify tokens")
    {
        class script_t
        {
        public:
            struct value_t
            {
                std::string     f_string = std::string();
                //uint512_t       f_integer = uint512_t(); -- in this test, we do not want to verify super large numbers
                std::int64_t    f_integer = 0;
                long double     f_floating_point = 0.0;
            };

            void add_line(prinbee::pbql::token_t token, std::string const & s)
            {
                append_to_script(s);
                f_tokens.push_back(token);
                f_values.push_back(value_t());
            }

            void add_line(prinbee::pbql::token_t token, std::string const & s, std::string string)
            {
                append_to_script(s);
                f_tokens.push_back(token);
                value_t v;
                v.f_string = string;
                f_values.push_back(v);
            }

            void add_line(prinbee::pbql::token_t token, std::string const & s, std::int64_t integer)
            {
                append_to_script(s);
                value_t v;
                f_tokens.push_back(token);
                v.f_integer = integer;
                f_values.push_back(v);
            }

            void add_line(prinbee::pbql::token_t token, std::string const & s, long double floating_point)
            {
                append_to_script(s);
                f_tokens.push_back(token);
                value_t v;
                v.f_floating_point = floating_point;
                f_values.push_back(v);
            }

            void tokenize() const
            {
                std::string const filename("./lexer-tokens.pbql");
                prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(f_script, filename));
                prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
                lexer->set_input(in);

                CATCH_REQUIRE(f_tokens.size() == f_values.size());
                for(std::size_t idx(0); idx < f_tokens.size(); ++idx)
                {
                    prinbee::pbql::node::pointer_t n(lexer->get_next_token());
                    CATCH_REQUIRE(n->get_token() == f_tokens[idx]);

                    prinbee::pbql::location const & loc(n->get_location());
                    CATCH_REQUIRE(loc.get_filename() == filename);
                    CATCH_REQUIRE(loc.get_column() == 1); // poor test for columns...
                    CATCH_REQUIRE(loc.get_line() == static_cast<int>(idx + 1));

                    CATCH_REQUIRE(n->get_string() == f_values[idx].f_string);
                    CATCH_REQUIRE(n->get_integer() == f_values[idx].f_integer);
                    bool const float_nearly_equal(SNAP_CATCH2_NAMESPACE::nearly_equal(n->get_floating_point(), f_values[idx].f_floating_point, 0.0L));
                    if(!float_nearly_equal)
                    {
                        SNAP_LOG_FATAL
                            << "floating points are not equal "
                            << n->get_floating_point()
                            << " vs "
                            << f_values[idx].f_floating_point
                            << SNAP_LOG_SEND;
                    }
                    CATCH_REQUIRE(float_nearly_equal);
                    CATCH_REQUIRE(n->get_parent() == nullptr);
                    CATCH_REQUIRE(n->get_children_size() == 0);
                }

                // after that we always get an end of file token
                //
                for(std::size_t count(0); count < 10; ++count)
                {
                    prinbee::pbql::node::pointer_t n(lexer->get_next_token());
                    CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_EOF);

                    prinbee::pbql::location const & loc(n->get_location());
                    CATCH_REQUIRE(loc.get_filename() == filename);
                    CATCH_REQUIRE(loc.get_column() == 1);
                    CATCH_REQUIRE(loc.get_line() == static_cast<int>(f_tokens.size() + 1)); // EOF ends up on the last line + 1

                    CATCH_REQUIRE(n->get_string() == std::string());
                    CATCH_REQUIRE(n->get_integer() == 0);
                    CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(n->get_floating_point(), 0.0L, 0.0L));
                    CATCH_REQUIRE(n->get_parent() == nullptr);
                    CATCH_REQUIRE(n->get_children_size() == 0);
                }
            }

        private:
            void append_to_script(std::string const & s)
            {
                f_script += s;
                if((rand() & 1) == 0)
                {
                    f_script += '\r';
                }
                f_script += '\n';
            }

            std::string                         f_script = std::string();
            std::vector<prinbee::pbql::token_t> f_tokens = std::vector<prinbee::pbql::token_t>();
            std::vector<value_t>                f_values = std::vector<value_t>();
        };

        script_t script;

        // WARNING: the '#' on the very first line / column is a special case
        //          so try something else first
        //
        script.add_line(prinbee::pbql::token_t::TOKEN_MODULO,             "%");
        script.add_line(prinbee::pbql::token_t::TOKEN_BITWISE_XOR,        "#");
        script.add_line(prinbee::pbql::token_t::TOKEN_BITWISE_AND,        "&");
        script.add_line(prinbee::pbql::token_t::TOKEN_OPEN_PARENTHESIS,   "(");
        script.add_line(prinbee::pbql::token_t::TOKEN_CLOSE_PARENTHESIS,  ")");
        script.add_line(prinbee::pbql::token_t::TOKEN_MULTIPLY,           "*");
        script.add_line(prinbee::pbql::token_t::TOKEN_PLUS,               "+");
        script.add_line(prinbee::pbql::token_t::TOKEN_COMMA,              ",");
        script.add_line(prinbee::pbql::token_t::TOKEN_MINUS,              "-");
        script.add_line(prinbee::pbql::token_t::TOKEN_DIVIDE,             "/");
        script.add_line(prinbee::pbql::token_t::TOKEN_SEMI_COLON,         ";");
        script.add_line(prinbee::pbql::token_t::TOKEN_EQUAL,              "=");
        script.add_line(prinbee::pbql::token_t::TOKEN_ABSOLUTE_VALUE,     "@");
        script.add_line(prinbee::pbql::token_t::TOKEN_POWER,              "^");
        script.add_line(prinbee::pbql::token_t::TOKEN_BITWISE_OR,         "|");
        script.add_line(prinbee::pbql::token_t::TOKEN_REGULAR_EXPRESSION, "~");
        script.add_line(prinbee::pbql::token_t::TOKEN_IDENTIFIER,         "identifier", "identifier");
        script.add_line(prinbee::pbql::token_t::TOKEN_IDENTIFIER,         "CAPS",       "CAPS");
        script.add_line(prinbee::pbql::token_t::TOKEN_IDENTIFIER,         "_123",       "_123");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING,             "\'string\'", "string");
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "123",        123L);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "0911",       911L);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "0xa9d1b1f",      0xa9d1b1fL);
        script.add_line(prinbee::pbql::token_t::TOKEN_FLOATING_POINT,     "5.12309",    5.12309L);
        script.add_line(prinbee::pbql::token_t::TOKEN_FLOATING_POINT,     "5.12309E3",  5123.09L);
        script.add_line(prinbee::pbql::token_t::TOKEN_FLOATING_POINT,     "7.83213e+3", 7832.13L);
        script.add_line(prinbee::pbql::token_t::TOKEN_FLOATING_POINT,     "7841.93e-3", 7.84193L);
        script.add_line(prinbee::pbql::token_t::TOKEN_NOT_EQUAL,          "<>");
        script.add_line(prinbee::pbql::token_t::TOKEN_LESS,               "<");
        script.add_line(prinbee::pbql::token_t::TOKEN_LESS_EQUAL,         "<=");
        script.add_line(prinbee::pbql::token_t::TOKEN_GREATER,            ">");
        script.add_line(prinbee::pbql::token_t::TOKEN_GREATER_EQUAL,      ">=");
        script.add_line(prinbee::pbql::token_t::TOKEN_SQUARE_ROOT,        "|/");
        script.add_line(prinbee::pbql::token_t::TOKEN_CUBE_ROOT,          "||/");
        script.add_line(prinbee::pbql::token_t::TOKEN_SHIFT_LEFT,         "<<");
        script.add_line(prinbee::pbql::token_t::TOKEN_SHIFT_RIGHT,        ">>");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING_CONCAT,      "||");

        script.tokenize();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: hash comment at top of file (1 line)")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("# hash at the start is viewed as a comment!\nthis is not", "./lexer-hash-comment.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        prinbee::pbql::node::pointer_t n(lexer->get_next_token());
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "this");
        prinbee::pbql::location const & loc(n->get_location());
        CATCH_REQUIRE(loc.get_line() == 2);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "is");

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "not");

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_EOF);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: hash comment at top of file (3 lines)")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(
                "#!/usr/bin/pbql -e\n"
                "# initialization script for website tables\n"
                "# and some default system data\n"
                "CREATE TABLE magic;", "./lexer-hash-comment.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        prinbee::pbql::node::pointer_t n(lexer->get_next_token());
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "CREATE");
        prinbee::pbql::location const * loc(&n->get_location());
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 2); // this is a known bug... the getc()+ungetc() generate a location bug

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "TABLE");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 8);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "magic");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 14);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_SEMI_COLON);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 20); // same bug as above, we've read the ';' then did an ungetc() which does not correct the column

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_EOF);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: dash-dash comment")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(
              "CREATE TABLE test ( -- list of columns below\n"
              "  name TEXT,\n"
              "  -- the name above should be limited in length\r\n"
              "  email TEXT, -- email should include an '@' character\n"
              "  address TEXT,\n"
              "-- comment from the start of the line\r"
              "  age INTEGER\n"
              ");\n"
              "-- vim: comment\n"
            , "./lexer-hash-comment.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        // "CREATE TABLE test ("
        prinbee::pbql::node::pointer_t n(lexer->get_next_token());
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "CREATE");
        prinbee::pbql::location const * loc(&n->get_location());
        CATCH_REQUIRE(loc->get_line() == 1);
        CATCH_REQUIRE(loc->get_column() == 1);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "TABLE");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 1);
        CATCH_REQUIRE(loc->get_column() == 8);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "test");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 1);
        CATCH_REQUIRE(loc->get_column() == 14);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_OPEN_PARENTHESIS);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 1);
        CATCH_REQUIRE(loc->get_column() == 19);

        // "name TEXT,"
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "name");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 2);
        CATCH_REQUIRE(loc->get_column() == 3);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "TEXT");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 2);
        CATCH_REQUIRE(loc->get_column() == 8);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_COMMA);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 2);
        CATCH_REQUIRE(loc->get_column() == 13);

        // "email TEXT,"
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "email");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 3);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "TEXT");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 9);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_COMMA);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 14);

        // "address TEXT,"
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "address");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 5);
        CATCH_REQUIRE(loc->get_column() == 3);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "TEXT");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 5);
        CATCH_REQUIRE(loc->get_column() == 11);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_COMMA);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 5);
        CATCH_REQUIRE(loc->get_column() == 16);

        // "age INTEGER"
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "age");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 7);
        CATCH_REQUIRE(loc->get_column() == 3);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "INTEGER");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 7);
        CATCH_REQUIRE(loc->get_column() == 7);

        // ");"
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_CLOSE_PARENTHESIS);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 8);
        CATCH_REQUIRE(loc->get_column() == 1);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_SEMI_COLON);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 8);
        CATCH_REQUIRE(loc->get_column() == 2);

        // EOF
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_EOF);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("lexer_error", "[lexer] [pbql] [error]")
{
    CATCH_START_SECTION("lexer: missing input")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                          "logic_error: input missing."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: invalid string (EOF)")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("'string not ended", "./lexer-bad-string.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: unclosed string."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: invalid string (\\n)")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("'string\ncut 1'", "./lexer-bad-string.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: string cannot include a newline or carriage return character."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: invalid string (\\r)")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("'string\rcut 2'", "./lexer-bad-string.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: string cannot include a newline or carriage return character."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: invalid string (\\r\\n)")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("'string\rcut 3'", "./lexer-bad-string.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: string cannot include a newline or carriage return character."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: invalid floating point")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("7041.03e", "./lexer-bad-string.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: invalid floating point number (7041.03e)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: empty hexadecimal number")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("0x", "./lexer-bad-string.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: hexadecimal number needs at least one digit after the \"0x\"."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: unsupported characters")
    {
        char const unsupported_characters[] = {
            '`',
            '!',
            '$',
            '{',
            '}',
            '[',
            ']',
            ':',
            '"',
            '?',
        };

        for(std::size_t idx(0); idx < std::size(unsupported_characters); ++idx)
        {
            char const buf[2] = {
                unsupported_characters[idx],
                '\0',
            };
            prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(buf, "./lexer-bad-character.pbql"));
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(in);

            CATCH_REQUIRE_THROWS_MATCHES(
                      lexer->get_next_token()
                    , prinbee::unexpected_token
                    , Catch::Matchers::ExceptionMessage(
                              std::string("prinbee_exception: unexpected token (")
                            + buf
                            + ")."));
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
