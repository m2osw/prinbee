// Copyright (c) 2025  Made to Order Software Corp.  All Rights Reserved
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
#include    <prinbee/network/crc16.h>


// advgetopt
//
//#include    <advgetopt/options.h>


// snapdev
//
//#include    <snapdev/enum_class_math.h>
//#include    <snapdev/not_reached.h>


// C++
//
//#include    <iomanip>


// last include
//
#include    <snapdev/poison.h>



namespace
{




}
// no name namespace


CATCH_TEST_CASE("network_crc16", "[crc16][valid]")
{
    CATCH_START_SECTION("network_crc16: verify negation after")
    {
        // there isn't much we can do here, we verify that the computation
        // including the result is zero...
        //
        std::size_t const size(rand() % 64536 + 1024);
        std::vector<std::uint8_t> data(size);
        for(std::size_t i(0); i < size; ++i)
        {
            data[i] = rand();
        }
        std::uint16_t const crc16(prinbee::crc16_compute(data.data(), data.size()));
        data.push_back(crc16);
        data.push_back(crc16 >> 8);
        CATCH_REQUIRE(prinbee::crc16_compute(data.data(), data.size()) == 0);
    }
    CATCH_END_SECTION()

    // this doesn't work--it has to be at the end
    //CATCH_START_SECTION("network_crc16: verify negation before")
    //{
    //    // there isn't much we can do here, we verify that the computation
    //    // including the result is zero...
    //    //
    //    std::size_t const size(rand() % 64536 + 1024);
    //    std::vector<std::uint8_t> data(size);
    //    for(std::size_t i(0); i < size; ++i)
    //    {
    //        data[i] = rand();
    //    }
    //    std::uint16_t const crc16(prinbee::crc16_compute(data.data(), data.size()));
    //    data.insert(data.begin(), crc16);
    //    data.insert(data.begin(), crc16 >> 8);
    //    CATCH_REQUIRE(prinbee::crc16_compute(data.data(), data.size()) == 0);
    //}
    //CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
