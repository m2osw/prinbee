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
#include    <prinbee/network/binary_message.h>
#include    <prinbee/network/crc16.h>



// advgetopt
//
//#include    <advgetopt/options.h>


// snapdev
//
//#include    <snapdev/enum_class_math.h>
#include    <snapdev/raii_generic_deleter.h>


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


CATCH_TEST_CASE("network_crc16", "[crc16][valid][invalid]")
{
    CATCH_START_SECTION("network_crc16: verify empty buffer")
    {
        std::vector<std::uint8_t> data;
        std::uint16_t const crc16(prinbee::crc16_compute(data.data(), data.size()));
        CATCH_REQUIRE(crc16 == 0);
        data.push_back(0);
        data.push_back(0);
        CATCH_REQUIRE(prinbee::crc16_compute(data.data(), data.size()) == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("network_crc16: verify negation")
    {
        std::size_t const size(rand() % 64536 + 1024);
        std::vector<std::uint8_t> data(size);
        for(std::size_t i(0); i < size; ++i)
        {
            data[i] = rand();
        }
        std::uint16_t const crc16(prinbee::crc16_compute(data.data(), data.size()));

        // test against all values
        //
        data.push_back(0);
        data.push_back(0);
        for(std::uint32_t check(0); check < 0x10000; ++check)
        {
            data[data.size() - 2] = check;
            data[data.size() - 1] = check >> 8;
            if(check == crc16)
            {
                // only one that works
                //
                CATCH_REQUIRE(prinbee::crc16_compute(data.data(), data.size()) == 0);
            }
            else
            {
                CATCH_REQUIRE_FALSE(prinbee::crc16_compute(data.data(), data.size()) == 0);
            }
        }
    }
    CATCH_END_SECTION()

    // this doesn't work--the CRC16 has to be at the end
    //CATCH_START_SECTION("network_crc16: verify negation inserting CRC16 at the start")
    //{
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


CATCH_TEST_CASE("network_message", "[message][valid]")
{
    CATCH_START_SECTION("network_message: verify name")
    {
        prinbee::message_name_t one(prinbee::create_message_name("1"));
        char const * p1(reinterpret_cast<char const *>(&one));
        CATCH_REQUIRE(p1[0] == '1');
        CATCH_REQUIRE(p1[1] == '\0');
        CATCH_REQUIRE(p1[2] == '\0');
        CATCH_REQUIRE(p1[3] == '\0');

        prinbee::message_name_t two(prinbee::create_message_name("!?"));
        char const * p2(reinterpret_cast<char const *>(&two));
        CATCH_REQUIRE(p2[0] == '!');
        CATCH_REQUIRE(p2[1] == '?');
        CATCH_REQUIRE(p2[2] == '\0');
        CATCH_REQUIRE(p2[3] == '\0');

        prinbee::message_name_t abc(prinbee::create_message_name("ABC"));
        char const * p3(reinterpret_cast<char const *>(&abc));
        CATCH_REQUIRE(p3[0] == 'A');
        CATCH_REQUIRE(p3[1] == 'B');
        CATCH_REQUIRE(p3[2] == 'C');
        CATCH_REQUIRE(p3[3] == '\0');

        prinbee::message_name_t name(prinbee::create_message_name("NAME"));
        char const * p4(reinterpret_cast<char const *>(&name));
        CATCH_REQUIRE(p4[0] == 'N');
        CATCH_REQUIRE(p4[1] == 'A');
        CATCH_REQUIRE(p4[2] == 'M');
        CATCH_REQUIRE(p4[3] == 'E');
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("network_message: check defaults")
    {
        prinbee::binary_message msg;

        CATCH_REQUIRE(msg.get_name() == prinbee::g_message_unknown);

        CATCH_REQUIRE_FALSE(msg.has_pointer());

        std::size_t size(0);
        CATCH_REQUIRE_FALSE(msg.get_data_pointer(size));
        CATCH_REQUIRE(size == 0);

        CATCH_REQUIRE(msg.get_data().empty());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("network_message: check name")
    {
        prinbee::binary_message msg;

        CATCH_REQUIRE(msg.get_name() == prinbee::g_message_unknown);

        for(int i(0); i < 100; ++i)
        {
            std::stringstream ss;
            ss << "i" << i;
            msg.set_name(prinbee::create_message_name(ss.str().c_str()));

            char name[4] = {};
            int p(0);
            name[p++] = 'i';
            if(i >= 10)
            {
                name[p++] = i / 10 + '0';
            }
            name[p++] = i % 10 + '0';
            prinbee::message_name_t expected = *reinterpret_cast<std::uint32_t *>(name);
            CATCH_REQUIRE(msg.get_name() == expected);
        }

        std::size_t size(0);
        CATCH_REQUIRE_FALSE(msg.get_data_pointer(size));
        CATCH_REQUIRE(size == 0);

        CATCH_REQUIRE(msg.get_data().empty());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("network_message: check pointer")
    {
        prinbee::binary_message msg;

        CATCH_REQUIRE_FALSE(msg.has_pointer());

        std::size_t const size(rand() % 1000 + 10);
        void * ptr(malloc(size));
        CATCH_REQUIRE(ptr != nullptr);

        // we're responsible for that pointer...
        // (this is ugly, but that way we avoid one copy per message, some
        // of which are really large)
        //
        snapdev::raii_buffer_t auto_free(reinterpret_cast<char *>(ptr));

        msg.set_data_by_pointer(ptr, size);

        CATCH_REQUIRE(msg.has_pointer());

        std::size_t sz(0);
        CATCH_REQUIRE(msg.get_data_pointer(sz) == ptr);
        CATCH_REQUIRE(sz == size);

        // if we have a pointer, there is no data buffer
        //
        CATCH_REQUIRE(msg.get_data().size() == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("network_message: check data")
    {
        prinbee::binary_message msg;

        CATCH_REQUIRE_FALSE(msg.has_pointer());

        std::size_t const size(rand() % 1000 + 10);
        void * ptr(malloc(size));
        CATCH_REQUIRE(ptr != nullptr);

        // we're responsible for that pointer...
        // (this is ugly, but that way we avoid one copy per message, some
        // of which are really large)
        //
        snapdev::raii_buffer_t auto_free(reinterpret_cast<char *>(ptr));

        msg.set_data(ptr, size);

        std::vector<std::uint8_t> data(msg.get_data());
        CATCH_REQUIRE(data.size() == size);
        CATCH_REQUIRE(memcmp(data.data(), ptr, size) == 0);

        // if we a buffer, there is no pointer
        //
        CATCH_REQUIRE_FALSE(msg.has_pointer());
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("network_message_invalid", "[message][invalid]")
{
    CATCH_START_SECTION("network_message_invalid: the nullptr string is not a valid name")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::create_message_name(nullptr)
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: name cannot be null."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("network_message: the empty string is not a valid name")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::create_message_name("")
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: name cannot be empty."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("network_message: too many characters")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::create_message_name("ELEPHANT")
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: name cannot be more than 4 characters."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
