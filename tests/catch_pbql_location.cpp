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
#include    <prinbee/pbql/location.h>



// self
//
#include    "catch_main.h"


// snaplogger
//
#include    <snaplogger/message.h>


// last include
//
#include    <snapdev/poison.h>



CATCH_TEST_CASE("location", "[location] [pbql]")
{
    CATCH_START_SECTION("location: verify defaults")
    {
        prinbee::pbql::location l;

        CATCH_REQUIRE(l.get_filename() == std::string());
        CATCH_REQUIRE(l.get_column() == 1);
        CATCH_REQUIRE(l.get_line() == 1);

        CATCH_REQUIRE(l.get_location() == "1:1: ");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("location: verify filename")
    {
        prinbee::pbql::location l;

        CATCH_REQUIRE(l.get_filename() == std::string());
        for(int i(0); i < 10; ++i)
        {
            std::string const filename(SNAP_CATCH2_NAMESPACE::random_string(1, 25));
            l.set_filename(filename);
            CATCH_REQUIRE(l.get_filename() == filename);
            CATCH_REQUIRE(l.get_location() == filename + ":1:1: ");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("location: verify columns & lines")
    {
        for(int i(0); i < 10; ++i)
        {
            prinbee::pbql::location l;
            std::string const filename(SNAP_CATCH2_NAMESPACE::random_string(1, 25));
            l.set_filename(filename);

            unsigned int lines(0);
            SNAP_CATCH2_NAMESPACE::random(lines);
            lines = (lines % 1000) + 10;
            for(unsigned int y(0); y < lines; ++y)
            {
                unsigned int columns(0);
                SNAP_CATCH2_NAMESPACE::random(columns);
                columns = (columns % 1000) + 10;
                for(unsigned int x(0); x < columns; ++x)
                {
                    l.next_column();
                    CATCH_REQUIRE(l.get_column() == static_cast<int>(x + 2));

                    CATCH_REQUIRE(l.get_location() == filename + ":" + std::to_string(y + 1) + ":" + std::to_string(x + 2) + ": ");
                }
                l.next_line();
                CATCH_REQUIRE(l.get_line() == static_cast<int>(y + 2));
                CATCH_REQUIRE(l.get_column() == 1);

                CATCH_REQUIRE(l.get_location() == filename + ":" + std::to_string(y + 2) + ":1: ");
            }
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
