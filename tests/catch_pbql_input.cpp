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

// self
//
#include    "catch_main.h"


// prinbee
//
#include    <prinbee/pbql/input.h>

#include    <prinbee/exception.h>


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



char const * g_create_secure_table = "#!/usr/bin/pbql\n"
              "CREATE SECURE TABLE users (\n"
              "  name TEXT,\n"
              "  password TEXT,\n"
              "  email TEXT,\n"
              "  PRIMARY KEY (name),\n"
              ") WITH (REPLICATION = 3);\n";



} // no name namespace



CATCH_TEST_CASE("input", "[input] [pbql]")
{
    CATCH_START_SECTION("input: verify a script")
    {
        prinbee::pbql::input in(g_create_secure_table, "./my_script.pbql");

        prinbee::pbql::location const & l(in.get_location());
        CATCH_REQUIRE(l.get_filename() == "./my_script.pbql");
        CATCH_REQUIRE(l.get_column() == 1);
        CATCH_REQUIRE(l.get_line() == 1);

        int line(1);
        int column(1);
        std::size_t const max(strlen(g_create_secure_table));
        for(std::size_t idx(0); idx < max; ++idx)
        {
            char32_t const c(in.getc());
            CATCH_REQUIRE(c != libutf8::EOS);
            CATCH_REQUIRE(c == static_cast<char32_t>(g_create_secure_table[idx]));

            if(c == '\n')
            {
                ++line;
                column = 1;
            }
            else
            {
                ++column;
            }
            CATCH_REQUIRE(l.get_filename() == "./my_script.pbql");
            CATCH_REQUIRE(l.get_column() == column);
            CATCH_REQUIRE(l.get_line() == line);

            in.ungetc(c);
            CATCH_REQUIRE(in.getc() == static_cast<char32_t>(g_create_secure_table[idx]));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("input: test 1 & 2 & 3 ungetc()")
    {
        // one unget
        {
            prinbee::pbql::input in("a<b", "./my_script.pbql");
            char32_t c(in.getc());
            CATCH_REQUIRE(c == 'a');
            c = in.getc();
            CATCH_REQUIRE(c == '<');
            c = in.getc();
            CATCH_REQUIRE(c == 'b');
            // it's not '=' or '>'
            in.ungetc(c);
            in.ungetc(libutf8::EOS); // no effect
            c = in.getc();
            CATCH_REQUIRE(c == 'b');
            c = in.getc();
            CATCH_REQUIRE(c == libutf8::EOS);
        }

        // two ungets
        {
            prinbee::pbql::input in("a.b", "./my_script.pbql");
            char32_t c(in.getc());
            CATCH_REQUIRE(c == 'a');
            c = in.getc();
            CATCH_REQUIRE(c == '.');
            in.ungetc(libutf8::EOS); // no effect
            in.ungetc('>');
            in.ungetc(libutf8::EOS); // no effect
            in.ungetc('-');
            in.ungetc(libutf8::EOS); // no effect
            c = in.getc();
            CATCH_REQUIRE(c == '-');
            c = in.getc();
            CATCH_REQUIRE(c == '>');
            c = in.getc();
            CATCH_REQUIRE(c == 'b');
            c = in.getc();
            CATCH_REQUIRE(c == libutf8::EOS);
        }

        // three ungets
        {
            prinbee::pbql::input in("a~b", "./my_script.pbql");
            char32_t c(in.getc());
            CATCH_REQUIRE(c == 'a');
            c = in.getc();
            CATCH_REQUIRE(c == '~');
            in.ungetc('=');
            in.ungetc(libutf8::EOS); // no effect
            in.ungetc('~');
            in.ungetc(libutf8::EOS); // no effect
            in.ungetc('!');
            c = in.getc();
            CATCH_REQUIRE(c == '!');
            c = in.getc();
            CATCH_REQUIRE(c == '~');
            c = in.getc();
            CATCH_REQUIRE(c == '=');
            c = in.getc();
            CATCH_REQUIRE(c == 'b');
            c = in.getc();
            CATCH_REQUIRE(c == libutf8::EOS);
            in.ungetc(libutf8::EOS); // no effect
            c = in.getc();
            CATCH_REQUIRE(c == libutf8::EOS);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("input: three new lines")
    {
        prinbee::pbql::input in("1\r2\n3\r\n*\n", "./my_script.pbql");

        prinbee::pbql::location const & l(in.get_location());
        CATCH_REQUIRE(l.get_filename() == "./my_script.pbql");
        CATCH_REQUIRE(l.get_column() == 1);
        CATCH_REQUIRE(l.get_line() == 1);

        {
            char32_t const c(in.getc());
            CATCH_REQUIRE(c == '1');
            CATCH_REQUIRE(l.get_column() == 2);
            CATCH_REQUIRE(l.get_line() == 1);
        }

        {
            char32_t const c(in.getc());
            CATCH_REQUIRE(c == '\n');
            CATCH_REQUIRE(l.get_column() == 1);
            CATCH_REQUIRE(l.get_line() == 2);
        }

        {
            char32_t const c(in.getc());
            CATCH_REQUIRE(c == '2');
            CATCH_REQUIRE(l.get_column() == 2);
            CATCH_REQUIRE(l.get_line() == 2);
        }

        {
            char32_t const c(in.getc());
            CATCH_REQUIRE(c == '\n');
            CATCH_REQUIRE(l.get_column() == 1);
            CATCH_REQUIRE(l.get_line() == 3);
        }

        {
            char32_t const c(in.getc());
            CATCH_REQUIRE(c == '3');
            CATCH_REQUIRE(l.get_column() == 2);
            CATCH_REQUIRE(l.get_line() == 3);
        }

        {
            char32_t const c(in.getc());
            CATCH_REQUIRE(c == '\n');
            CATCH_REQUIRE(l.get_column() == 1);
            CATCH_REQUIRE(l.get_line() == 4);
        }

        {
            char32_t const c(in.getc());
            CATCH_REQUIRE(c == '*');
            CATCH_REQUIRE(l.get_column() == 2);
            CATCH_REQUIRE(l.get_line() == 4);
        }

        {
            char32_t const c(in.getc());
            CATCH_REQUIRE(c == '\n');
            CATCH_REQUIRE(l.get_column() == 1);
            CATCH_REQUIRE(l.get_line() == 5);
        }

        {
            char32_t const c(in.getc());
            CATCH_REQUIRE(c == libutf8::EOS);
            CATCH_REQUIRE(l.get_column() == 1);
            CATCH_REQUIRE(l.get_line() == 5);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("input: create from file")
    {
        std::string const filename(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/create_from_file.pbql");
        {
            std::ofstream out(filename);
            out << g_create_secure_table;
        }

        prinbee::pbql::input::pointer_t in(prinbee::pbql::create_input(filename));

        prinbee::pbql::location const & l(in->get_location());
        CATCH_REQUIRE(l.get_filename() == filename);
        CATCH_REQUIRE(l.get_column() == 1);
        CATCH_REQUIRE(l.get_line() == 1);

        int line(1);
        int column(1);
        std::size_t const max(strlen(g_create_secure_table));
        for(std::size_t idx(0); idx < max; ++idx)
        {
            char32_t const c(in->getc());
            CATCH_REQUIRE(c != libutf8::EOS);
            CATCH_REQUIRE(c == static_cast<char32_t>(g_create_secure_table[idx]));

            if(c == '\n')
            {
                ++line;
                column = 1;
            }
            else
            {
                ++column;
            }
            CATCH_REQUIRE(l.get_filename() == filename);
            CATCH_REQUIRE(l.get_column() == column);
            CATCH_REQUIRE(l.get_line() == line);
        }

        {
            char32_t const c(in->getc());
            CATCH_REQUIRE(c == libutf8::EOS);
            CATCH_REQUIRE(l.get_column() == column);
            CATCH_REQUIRE(l.get_line() == line);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("input_error", "[input] [pbql] [error]")
{
    CATCH_START_SECTION("input: too many ungetc()")
    {
        prinbee::pbql::input in("#!/usr/bin/pbql -r\n", "./my_script.pbql");
        CATCH_REQUIRE(in.getc() == '#');

        in.ungetc('1');
        in.ungetc('2');
        in.ungetc('3');

        CATCH_REQUIRE_THROWS_MATCHES(
                  in.ungetc('*')
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                          "out_of_range: ungetc() called too many times."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("input: file not found")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::pbql::create_input("unknown.file")
                , prinbee::file_not_found
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: could not read \"unknown.file\"."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
