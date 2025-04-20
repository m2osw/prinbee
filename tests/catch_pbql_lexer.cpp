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
#include    <prinbee/pbql/lexer.h>

#include    <prinbee/exception.h>


// self
//
#include    "catch_main.h"


// libuf8
//
#include    <libutf8/libutf8.h>


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
#include    <bitset>
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
        script.add_line(prinbee::pbql::token_t::TOKEN_COLON,              ":");
        script.add_line(prinbee::pbql::token_t::TOKEN_SEMI_COLON,         ";");
        script.add_line(prinbee::pbql::token_t::TOKEN_EQUAL,              "=");
        script.add_line(prinbee::pbql::token_t::TOKEN_ABSOLUTE_VALUE,     "@");
        script.add_line(prinbee::pbql::token_t::TOKEN_POWER,              "^");
        script.add_line(prinbee::pbql::token_t::TOKEN_BITWISE_OR,         "|");
        script.add_line(prinbee::pbql::token_t::TOKEN_REGULAR_EXPRESSION, "~");
        script.add_line(prinbee::pbql::token_t::TOKEN_IDENTIFIER,         "identifier",          "identifier");
        script.add_line(prinbee::pbql::token_t::TOKEN_IDENTIFIER,         "CAPS",                "CAPS");
        script.add_line(prinbee::pbql::token_t::TOKEN_IDENTIFIER,         "_123",                "_123");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING,             "'\\no e\\fect'",      "\\no e\\fect");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING,             "e'string\\n'",        "string\n");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING,             "E'string\\r'",        "string\r");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING,             "e'\\b\\f\\n\\r\\t'",  "\b\f\n\r\t");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING,             "e'\\a\\g\\m\\s\\\\'", "agms\\");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING,             "e'\\58only 5'",       "\058only 5");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING,             "e'\\339only 33'",     "\339only 33");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING,             "e'\\xfg only f'",     "\xfg only f");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING,             "e'\\xf: only f'",     "\xf: only f");
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "123",                 123L);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "0b11001010",          0xCAL);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "0B11001010",          0xCAL);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "b'11001010'",         0xCAL);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "B'11011110'",         0xDEL);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "0711",                711L); // this is not octal in SQL
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "0o345",               0345L);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "0O346",               0346L);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "o'365'",              0365L);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "O'645'",              0645L);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "0xa9d1b1f",           0xa9d1b1fL);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "0Xa3d1f1c",           0xa3d1f1cL);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "x'a9d3b3f'",          0xa9d3b3fL);
        script.add_line(prinbee::pbql::token_t::TOKEN_INTEGER,            "X'a9d9d1f'",          0xa9d9d1fL);
        script.add_line(prinbee::pbql::token_t::TOKEN_FLOATING_POINT,     "5.12309",             5.12309L);
        script.add_line(prinbee::pbql::token_t::TOKEN_FLOATING_POINT,     "5.12309E3",           5123.09L);
        script.add_line(prinbee::pbql::token_t::TOKEN_FLOATING_POINT,     "7.83213e+3",          7832.13L);
        script.add_line(prinbee::pbql::token_t::TOKEN_FLOATING_POINT,     "7841.93e-3",          7.84193L);
        script.add_line(prinbee::pbql::token_t::TOKEN_NOT_EQUAL,          "<>");
        script.add_line(prinbee::pbql::token_t::TOKEN_LESS,               "<");
        script.add_line(prinbee::pbql::token_t::TOKEN_LESS_EQUAL,         "<=");
        script.add_line(prinbee::pbql::token_t::TOKEN_GREATER,            ">");
        script.add_line(prinbee::pbql::token_t::TOKEN_GREATER_EQUAL,      ">=");
        script.add_line(prinbee::pbql::token_t::TOKEN_SQUARE_ROOT,        "|/");
        script.add_line(prinbee::pbql::token_t::TOKEN_CUBE_ROOT,          "||/");
        script.add_line(prinbee::pbql::token_t::TOKEN_SCOPE,              "::");
        script.add_line(prinbee::pbql::token_t::TOKEN_SHIFT_LEFT,         "<<");
        script.add_line(prinbee::pbql::token_t::TOKEN_SHIFT_RIGHT,        ">>");
        script.add_line(prinbee::pbql::token_t::TOKEN_STRING_CONCAT,      "||");

        script.tokenize();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: binary 0 to 255")
    {
        for(int v(0); v < 256; ++v)
        {
            std::bitset<8> binary(v);
            std::stringstream ss;
            ss << "0b" << binary;

            prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(ss.str(), "./lexer-binary.pbql"));
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(in);

            prinbee::pbql::node::pointer_t n(lexer->get_next_token());
            CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_INTEGER);
            CATCH_REQUIRE(n->get_integer() == v);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: octal characters 1 to 255")
    {
        for(int c(1); c < 256; ++c)
        {
            int const zeroes(c < 8 ? rand() % 3 : (c < 64 ? rand() % 2 : 0));
            std::stringstream ss;
            ss << (rand() & 1 ? 'e' : 'E')
               << "'\\"
               << std::oct
               << std::string(zeroes, '0')
               << c
               << '\'';

            prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(ss.str(), "./lexer-octal-char.pbql"));
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(in);

            prinbee::pbql::node::pointer_t n(lexer->get_next_token());
            CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_STRING);
            if(c < 0x80)
            {
                CATCH_REQUIRE(n->get_string() == std::string(1, c));
            }
            else
            {
                std::string const expected(libutf8::to_u8string(static_cast<char32_t>(c)));
                CATCH_REQUIRE(n->get_string() == expected);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: hexadecimal characters 1 to 255")
    {
        for(int c(1); c < 256; ++c)
        {
            std::stringstream ss;
            if((rand() & 1) != 0)
            {
                ss << std::uppercase;
            }
            ss << (rand() & 1 ? 'e' : 'E')
               << "'\\"
               << (rand() & 1 ? 'x' : 'X')
               << std::hex
               << (c < 16 ? ((rand() & 1) != 0 ? "" : "0") : "")
               << c
               << '\'';

            prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(ss.str(), "./lexer-hexadecimal-char.pbql"));
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(in);

            prinbee::pbql::node::pointer_t n(lexer->get_next_token());
            CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_STRING);
            if(c < 0x80)
            {
                CATCH_REQUIRE(n->get_string() == std::string(1, c));
            }
            else
            {
                std::string const expected(libutf8::to_u8string(static_cast<char32_t>(c)));
                CATCH_REQUIRE(n->get_string() == expected);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: 4 digits unicode characters")
    {
        for(int c(1); c < 0x10000; ++c)
        {
            // skip surrogates
            //
            if(c == 0xD800)
            {
                c = 0xE000;
            }
            std::stringstream ss;
            if((rand() & 1) != 0)
            {
                ss << std::uppercase;
            }
            ss << (rand() & 1 ? 'e' : 'E')
               << "'\\u"
               << std::hex
               << std::setfill('0')
               << std::setw(4)
               << c
               << '\'';

            prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(ss.str(), "./lexer-plane0-unicode-char.pbql"));
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(in);

            prinbee::pbql::node::pointer_t n(lexer->get_next_token());
            CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_STRING);
            if(c < 0x80)
            {
                CATCH_REQUIRE(n->get_string() == std::string(1, c));
            }
            else
            {
                std::string const expected(libutf8::to_u8string(static_cast<char32_t>(c)));
                CATCH_REQUIRE(n->get_string() == expected);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: 8 digits unicode characters")
    {
        for(int count(0); count < 1000; ++count)
        {
            int const c(SNAP_CATCH2_NAMESPACE::random_char(SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_UNICODE));
            std::stringstream ss;
            if((rand() & 1) != 0)
            {
                ss << std::uppercase;
            }
            ss << (rand() & 1 ? 'e' : 'E')
               << "'\\U"
               << std::hex
               << std::setfill('0')
               << std::setw(8)
               << c
               << '\'';

            prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(ss.str(), "./lexer-any-unicode-char.pbql"));
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(in);

            prinbee::pbql::node::pointer_t n(lexer->get_next_token());
            CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_STRING);
            if(c < 0x80)
            {
                CATCH_REQUIRE(n->get_string() == std::string(1, c));
            }
            else
            {
                std::string const expected(libutf8::to_u8string(static_cast<char32_t>(c)));
                CATCH_REQUIRE(n->get_string() == expected);
            }
        }
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
                "CREATE TABLE /* C-like comment */ magic;", "./lexer-hash-comment.pbql"));
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
        CATCH_REQUIRE(loc->get_column() == 35);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_SEMI_COLON);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 41); // same bug as above, we've read the ';' then did an ungetc() which does not correct the column

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_EOF);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer: dash-dash comment")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(
              "/* copyright notices\n"
              " * often go here\n"
              " */\n"
              "CREATE TABLE test ( -- list of columns below\n"
              "  name TEXT,\n"
              "  -- the name above should be limited in length\r\n"
              "  email TEXT, -- email should include an '@' character\n"
              "  address TEXT,\n"
              "-- comment from the start of the line\r"
              "  age INTEGER /* and C-like /* comments can be */ nested */\n"
              "); /*** multi-asterisks ***/\n"
              "-- vim: comment\n"
            , "./lexer-hash-comment.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        // "CREATE TABLE test ("
        prinbee::pbql::node::pointer_t n(lexer->get_next_token());
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "CREATE");
        prinbee::pbql::location const * loc(&n->get_location());
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 1);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "TABLE");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 8);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "test");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 14);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_OPEN_PARENTHESIS);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 4);
        CATCH_REQUIRE(loc->get_column() == 19);

        // "name TEXT,"
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "name");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 5);
        CATCH_REQUIRE(loc->get_column() == 3);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "TEXT");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 5);
        CATCH_REQUIRE(loc->get_column() == 8);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_COMMA);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 5);
        CATCH_REQUIRE(loc->get_column() == 13);

        // "email TEXT,"
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "email");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 7);
        CATCH_REQUIRE(loc->get_column() == 3);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "TEXT");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 7);
        CATCH_REQUIRE(loc->get_column() == 9);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_COMMA);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 7);
        CATCH_REQUIRE(loc->get_column() == 14);

        // "address TEXT,"
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "address");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 8);
        CATCH_REQUIRE(loc->get_column() == 3);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "TEXT");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 8);
        CATCH_REQUIRE(loc->get_column() == 11);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_COMMA);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 8);
        CATCH_REQUIRE(loc->get_column() == 16);

        // "age INTEGER"
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "age");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 10);
        CATCH_REQUIRE(loc->get_column() == 3);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);
        CATCH_REQUIRE(n->get_string() == "INTEGER");
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 10);
        CATCH_REQUIRE(loc->get_column() == 7);

        // ");"
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_CLOSE_PARENTHESIS);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 11);
        CATCH_REQUIRE(loc->get_column() == 1);

        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_SEMI_COLON);
        loc = &n->get_location();
        CATCH_REQUIRE(loc->get_line() == 11);
        CATCH_REQUIRE(loc->get_column() == 2);

        // EOF
        n = lexer->get_next_token();
        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_EOF);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("lexer_error", "[lexer] [pbql] [error]")
{
    CATCH_START_SECTION("lexer_error: missing input")
    {
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                          "logic_error: input missing."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: invalid string (EOF)")
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

    CATCH_START_SECTION("lexer_error: invalid string (\\n)")
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

    CATCH_START_SECTION("lexer_error: invalid string (\\r)")
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

    CATCH_START_SECTION("lexer_error: invalid string (\\r\\n)")
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

    CATCH_START_SECTION("lexer_error: invalid string (escaped characters)")
    {
        struct invalid_escape_t
        {
            char const * const      f_invalid_sequence = nullptr;
            int                     f_count = 0;
            int                     f_expected_count = 0;
        };
        constexpr invalid_escape_t const invalid_escapes[] =
        {
            { "\\xvoid",        0, 2 },
            { "\\uvoid",        0, 4 },
            { "\\u1",           1, 4 },
            { "\\u21",          2, 4 },
            { "\\u311",         3, 4 },
            { "\\Uvoid",        0, 8 },
            { "\\U1",           1, 8 },
            { "\\U21",          2, 8 },
            { "\\U311",         3, 8 },
            { "\\U4111",        4, 8 },
            { "\\U51111",       5, 8 },
            { "\\U611111",      6, 8 },
            { "\\U7111111",     7, 8 },
        };
        for(auto const & e : invalid_escapes)
        {
            std::string str("e'str: ");
            str += e.f_invalid_sequence;
            str += '\'';
            prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(str, "./lexer-bad-escape-sequence.pbql"));
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(in);

            CATCH_REQUIRE_THROWS_MATCHES(
                      lexer->get_next_token()
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: lexer::get_next_token() -- escape sequence needed "
                            + std::to_string(e.f_expected_count)
                            + " digits; found "
                            + std::to_string(e.f_count)
                            + " instead."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: \\0 is not allowed in strings")
    {
        struct invalid_escape_t
        {
            char const * const      f_invalid_sequence = nullptr;
        };
        constexpr invalid_escape_t const invalid_escapes[] =
        {
            { "octal null \\0 -- size of 1"         },
            { "octal null \\00 -- size of 2"        },
            { "octal null \\000 -- size of 3"       },
            { "hexadecimal null \\x0 --size of 1"   },
            { "hexadecimal null \\x00 --size of 2"  },
            { "unicode \\u0000 -- size of 4"        },
            { "unicode \\U00000000 -- size of 8"    },
        };
        for(auto const & e : invalid_escapes)
        {
            std::string str("e'str: ");
            str += e.f_invalid_sequence;
            str += '\'';
            prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(str, "./lexer-bad-null-character.pbql"));
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(in);

            CATCH_REQUIRE_THROWS_MATCHES(
                      lexer->get_next_token()
                    , prinbee::unexpected_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: lexer::get_next_token() -- the NULL character is not allowed in strings."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: missing */")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("/* C-Like comment must end with '*' and '/'", "./lexer-bad-c-comment.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: end of script reached within a C-like comment (i.e. '*/' not found; depth: 1)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: invalid floating point")
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

    CATCH_START_SECTION("lexer_error: empty binary number")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("0b", "./lexer-bad-binary.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: a binary number needs at least one digit."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: binary does not accept 8 or 9")
    {
        for(int digit(2); digit < 10; ++digit)
        {
            std::string const bin("0b" + std::to_string(digit));
            prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(bin, "./lexer-bad-binary.pbql"));
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(in);

            CATCH_REQUIRE_THROWS_MATCHES(
                      lexer->get_next_token()
                    , prinbee::invalid_number
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: a binary number only supports binary digits (0 and 1)."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: binary string not ending with quote")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("b'101 missing closing quote", "./lexer-bad-binary.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: a binary string needs to end with a quote (')."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: empty octal number")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("0o", "./lexer-bad-octal.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: an octal number needs at least one digit after the \"0o\"."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: octal does not accept 8 or 9")
    {
        char const * const bad_octal[] = {
            "0o8",
            "0o9",
        };
        for(int idx(0); idx < 2; ++idx)
        {
            prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(bad_octal[idx], "./lexer-bad-octal.pbql"));
            prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
            lexer->set_input(in);

            CATCH_REQUIRE_THROWS_MATCHES(
                      lexer->get_next_token()
                    , prinbee::invalid_number
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: an octal number cannot include digits 8 or 9."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: octal string not ending with quote")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("o'123 missing closing quote", "./lexer-bad-octal.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: an octal string needs to end with a quote (')."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: empty hexadecimal number")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("0x", "./lexer-bad-hexadecimal.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: a hexadecimal number needs at least one digit after the \"0x\"."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: hexadecimal string not ending with quote")
    {
        prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>("x'f1a3 missing closing quote", "./lexer-bad-hexadecimal.pbql"));
        prinbee::pbql::lexer::pointer_t lexer(std::make_shared<prinbee::pbql::lexer>());
        lexer->set_input(in);

        CATCH_REQUIRE_THROWS_MATCHES(
                  lexer->get_next_token()
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: a hexadecimal string needs to end with a quote (')."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("lexer_error: unsupported characters")
    {
        char const unsupported_characters[] = {
            '`',
            '!',
            '$',
            '{',
            '}',
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
