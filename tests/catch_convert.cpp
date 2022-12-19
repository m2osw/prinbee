// Copyright (c) 2019-2022  Made to Order Software Corp.  All Rights Reserved
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
#include    <prinbee/exception.h>
#include    <prinbee/data/convert.h>


// snapdev
//
#include    <snapdev/hexadecimal_string.h>
#include    <snapdev/math.h>
#include    <snapdev/ostream_int128.h>


// C++
//
#include    <fstream>
#include    <iomanip>


// C
//
#include    <sys/stat.h>
#include    <sys/types.h>


// last include
//
#include    <snapdev/poison.h>


namespace
{


std::string decorate(std::string s)
{
    if(s[0] != '-'
    && (rand() % 3) == 0)
    {
        s = '+' + s;
    }
    while((rand() & 1) != 0)
    {
        s = ' ' + s;
    }
    while((rand() & 1) != 0)
    {
        s += ' ';
    }
    return s;
}


template<typename T>
std::string to_binary(T const n)
{
    std::stringstream ss;
    ss << std::bitset<sizeof(T) * 8>(n).to_string(); // fun but adds leading '0's
    std::string result(ss.str());
    std::string::size_type const pos(result.find('1'));
    if(pos == std::string::npos)
    {
        return "0";
    }
    return result.substr(pos);
}


template<>
std::string to_binary(prinbee::uint512_t const n)
{
    if(n.is_zero())
    {
        return "0";
    }
    prinbee::uint512_t b;
    b.f_value[7] = 0x8000000000000000ULL;
    for(;; b >>= 1)
    {
        if((n & b) != 0)
        {
            break;
        }
    }
    std::string result;
    while(!b.is_zero())
    {
        result += (n & b) != 0 ? '1' : '0';
        b >>= 1;
    }
    return result;
}


template<>
std::string to_binary(prinbee::int512_t const n)
{
    if(n.is_zero())
    {
        return "0";
    }
    prinbee::uint512_t a;
    if(n < 0)
    {
        a = -n;
    }
    else
    {
        a = n;
    }
    prinbee::uint512_t b;
    b.f_value[7] = 0x8000000000000000ULL;
    for(;; b >>= 1)
    {
        if((a & b) != 0)
        {
            break;
        }
    }
    std::string result;
    while(!b.is_zero())
    {
        result += (a & b) != 0 ? '1' : '0';
        b >>= 1;
    }
    return result;
}


} // no name namespace



CATCH_TEST_CASE("convert_8bit", "[convert] [valid]")
{
    CATCH_START_SECTION("convert_8bit: uint8_t")
    {
        for(uint32_t i(0); i < (1ULL << 8); ++i)
        {
            {
                std::stringstream ss;
                ss << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0x" << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << std::uppercase << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << std::uppercase << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0" << std::oct << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            uint32_t v(i);
            std::string r;
            while(v != 0)
            {
                r += (v & 1) + '0';
                v >>= 1;
            }
            std::reverse(r.begin(), r.end());
            r = "0b" + r;
            {
                uint64_t const c(prinbee::convert_to_uint(decorate(r), 8));
                CATCH_REQUIRE(c == i);
            }

            r[1] &= 0x5F;
            {
                uint64_t const c(prinbee::convert_to_uint(decorate(r), 8));
                CATCH_REQUIRE(c == i);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_8bit: int8_t")
    {
        for(uint32_t i(0); i < (1ULL << 8); ++i)
        {
            {
                std::stringstream ss;
                ss << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << std::uppercase << i << '\'';
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << std::uppercase << i << '\'';
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << i << '\'';
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << i << '\'';
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0" << std::oct << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            uint32_t v(i);
            std::string r;
            while(v != 0)
            {
                r += (v & 1) + '0';
                v >>= 1;
            }
            std::reverse(r.begin(), r.end());
            r = "0b" + r;
            {
                uint64_t const c(prinbee::convert_to_int(decorate(r), 8));
                CATCH_REQUIRE(c == i);
            }

            r[1] &= 0x5F;
            {
                uint64_t const c(prinbee::convert_to_int(decorate(r), 8));
                CATCH_REQUIRE(c == i);
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("convert_16bit", "[convert] [valid]")
{
    CATCH_START_SECTION("convert_16bit: uint16_t")
    {
        for(uint32_t i(0); i < (1ULL << 16); i += rand() % 27)
        {
            {
                std::stringstream ss;
                ss << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0x" << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0X" << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0x" << std::hex << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0X" << std::hex << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << std::uppercase << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << std::uppercase << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0" << std::oct << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            uint32_t v(i);
            std::string r;
            while(v != 0)
            {
                r += (v & 1) + '0';
                v >>= 1;
            }
            std::reverse(r.begin(), r.end());
            r = "0b" + r;
            {
                uint64_t const c(prinbee::convert_to_uint(decorate(r), 16));
                CATCH_REQUIRE(c == i);
            }

            r[1] &= 0x5F;
            {
                uint64_t const c(prinbee::convert_to_uint(decorate(r), 16));
                CATCH_REQUIRE(c == i);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_16bit: int16_t")
    {
        for(uint32_t i(0); i < (1ULL << 16); i += rand() % 27)
        {
            {
                std::stringstream ss;
                ss << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0x" << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0X" << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0x" << std::hex << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0X" << std::hex << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << std::uppercase << i << '\'';
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << std::uppercase << i << '\'';
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << i << '\'';
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << i << '\'';
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0" << std::oct << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 16));
                CATCH_REQUIRE(c == i);
            }

            uint32_t v(i);
            std::string r;
            while(v != 0)
            {
                r += (v & 1) + '0';
                v >>= 1;
            }
            std::reverse(r.begin(), r.end());
            r = "0b" + r;
            {
                uint64_t const c(prinbee::convert_to_int(decorate(r), 16));
                CATCH_REQUIRE(c == i);
            }

            r[1] &= 0x5F;
            {
                uint64_t const c(prinbee::convert_to_int(decorate(r), 16));
                CATCH_REQUIRE(c == i);
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("convert_32bit", "[convert] [valid]")
{
    CATCH_START_SECTION("convert_32bit: uint32_t")
    {
        for(uint64_t i(0); i < (1ULL << 32); i += rand() % 60000)
        {
            {
                std::stringstream ss;
                ss << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0x" << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0X" << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0x" << std::hex << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0X" << std::hex << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << std::uppercase << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << std::uppercase << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << i << '\'';
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0" << std::oct << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            uint32_t v(i);
            std::string r;
            while(v != 0)
            {
                r += (v & 1) + '0';
                v >>= 1;
            }
            std::reverse(r.begin(), r.end());
            r = "0b" + r;
            {
                uint64_t const c(prinbee::convert_to_uint(decorate(r), 32));
                CATCH_REQUIRE(c == i);
            }

            r[1] &= 0x5F;
            {
                uint64_t const c(prinbee::convert_to_uint(decorate(r), 32));
                CATCH_REQUIRE(c == i);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_32bit: int32_t")
    {
        for(std::int64_t i(0); i < (1LL << 31); i += rand() % 60000)
        {
            {
                std::stringstream ss;
                ss << i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << -i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == -i);
            }

            {
                std::stringstream ss;
                ss << "0x" << std::hex << std::uppercase << i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "-0x" << std::hex << std::uppercase << i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == -i);
            }

            {
                std::stringstream ss;
                ss << "0X" << std::hex << std::uppercase << i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "-0X" << std::hex << std::uppercase << i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == -i);
            }

            {
                std::stringstream ss;
                ss << "0x" << std::hex << i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "-0x" << std::hex << i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == -i);
            }

            {
                std::stringstream ss;
                ss << "0X" << std::hex << i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "-0X" << std::hex << i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == -i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << std::uppercase << i << '\'';
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "-x'" << std::hex << std::uppercase << i << '\'';
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == -i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << std::uppercase << i << '\'';
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "-X'" << std::hex << std::uppercase << i << '\'';
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == -i);
            }

            {
                std::stringstream ss;
                ss << "x'" << std::hex << i << '\'';
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "-x'" << std::hex << i << '\'';
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == -i);
            }

            {
                std::stringstream ss;
                ss << "X'" << std::hex << i << '\'';
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "-X'" << std::hex << i << '\'';
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == -i);
            }

            {
                std::stringstream ss;
                ss << "0" << std::oct << i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "-0" << std::oct << i;
                std::int64_t const c(prinbee::convert_to_int(decorate(ss.str()), 32));
                CATCH_REQUIRE(c == -i);
            }

            std::int32_t v(i);
            CATCH_REQUIRE(v >= 0);
            std::string r;
            while(v != 0)
            {
                r += (v & 1) + '0';
                v >>= 1;
            }
            if(i == 0)
            {
                v += '0';
            }
            std::reverse(r.begin(), r.end());
            r = "0b" + r;
            {
                std::int64_t const c(prinbee::convert_to_int(decorate(r), 32));
                CATCH_REQUIRE(c == i);
            }

            r[1] &= 0x5F;
            {
                std::int64_t const c(prinbee::convert_to_int(decorate(r), 32));
                CATCH_REQUIRE(c == i);
            }

            r = '-' + r;
            {
                std::int64_t const c(prinbee::convert_to_int(decorate(r), 32));
                CATCH_REQUIRE(c == -i);
            }

            r[2] |= 0x20;
            {
                std::int64_t const c(prinbee::convert_to_int(decorate(r), 32));
                CATCH_REQUIRE(c == -i);
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("convert_size", "[convert] [size] [valid]")
{
    CATCH_START_SECTION("convert_size")
    {
        struct size_info
        {
            char const *    f_name = nullptr;
            int             f_power = 1;
            bool            f_1000 = true;
        };
        size_info si[] =
        {
            { "byte",     0 },
            { "bytes",    0 },

            { "kb",       1 },
            { "kib",      1, false },
            { "kibi",     1, false },
            { "kilo",     1 },
            // TODO: the "byte[s]" in the following are WRONG at this time...
            { "kb byte",  1 },
            { "kb bytes", 1 },
            { "kibbyte",  1, false },
            { "kibbytes", 1, false },

            { "mb",       2 },
            { "mebi",     2, false },
            { "mega",     2 },
            { "mib",      2, false },

            { "gb",       3 },
            { "gibi",     3, false },
            { "giga",     3 },
            { "gib",      3, false },

            { "tb",       4 },
            { "tebi",     4, false },
            { "tera",     4 },
            { "tib",      4, false },
        };
        for(auto const & s : si)
        {
            int count(rand() % 10);
            std::string const n(std::to_string(count));
            std::string unit(s.f_name);
            if((rand() & 1) != 0)
            {
                // make one character uppercase, which should have no effects
                //
                unit[rand() % unit.length()] &= 0x5F;
            }
            std::uint64_t const c(prinbee::convert_to_uint(n + ' ' + unit, 64, prinbee::unit_t::UNIT_SIZE));
            std::uint64_t const expected(snapdev::pow(s.f_1000 ? 1000ULL : 1024ULL, s.f_power) * count);
            CATCH_REQUIRE(c == expected);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("convert_buffer", "[convert] [size] [valid]")
{
    CATCH_START_SECTION("convert_buffer: string -> bits8")
    {
        for(int i(0); i < 256; ++i)
        {
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS8, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::showbase << std::oct << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS8, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS8, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS8, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> uint8")
    {
        for(int i(0); i < 256; ++i)
        {
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT8, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT8, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT8, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT8, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> int8")
    {
        for(int i(-128); i < 127; ++i)
        {
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else if(i > 0)
                {
                    ss << "0B" << to_binary(i);
                }
                else
                {
                    ss << "-0B" << to_binary(-i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT8, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else if(i > 0)
                {
                    ss << std::oct << std::showbase << i;
                }
                else
                {
                    ss << std::oct << std::showbase << '-' << -i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT8, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT8, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else if(i > 0)
                {
                    ss << std::showbase << std::hex << std::uppercase << i;
                }
                else
                {
                    ss << std::showbase << std::hex << std::uppercase << '-' << -i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT8, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> bits16")
    {
        for(int j(0); j < 100; ++j)
        {
            std::uint16_t const i(rand());
            {
                std::stringstream ss;
                ss << "0B" << to_binary(i);
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS16, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS16, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS16, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS16, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> uint16")
    {
        for(int j(0); j < 100; ++j)
        {
            std::uint16_t const i(rand());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT16, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT16, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT16, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT16, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> int16")
    {
        for(int j(0); j < 100; ++j)
        {
            std::int16_t const i(rand());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i > 0)
                {
                    ss << "0B" << to_binary(i);
                }
                else
                {
                    ss << "-0B" << to_binary(-i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT16, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << '-' << std::showbase << std::oct << -i;
                }
                else
                {
                    ss << std::showbase << std::oct << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT16, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT16, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << '-' << std::showbase << std::hex << std::uppercase << -i;
                }
                else
                {
                    ss << std::showbase << std::hex << std::uppercase << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT16, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> bits32")
    {
        for(int j(0); j < 100; ++j)
        {
            std::uint32_t const i(SNAP_CATCH2_NAMESPACE::rand32());
            {
                std::stringstream ss;
                ss << "0B" << to_binary(i);
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS32, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS32, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS32, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS32, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> uint32")
    {
        for(int j(0); j < 100; ++j)
        {
            std::uint32_t const i(SNAP_CATCH2_NAMESPACE::rand32());
            {
                std::stringstream ss;
                ss << "0B" << to_binary(i);
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT32, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT32, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT32, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT32, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> int32")
    {
        for(int j(0); j < 100; ++j)
        {
            std::int32_t const i(SNAP_CATCH2_NAMESPACE::rand32());
            {
                std::stringstream ss;
                if(i < 0)
                {
                    ss << "-0B" << to_binary(-i);
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT32, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << '-' << std::showbase << std::oct << -i;
                }
                else
                {
                    ss << std::showbase << std::oct << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT32, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT32, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else if(i < 0)
                {
                    ss << '-' << std::hex << std::uppercase << "0X" << -i;
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT32, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> bits64")
    {
        for(int j(0); j < 100; ++j)
        {
            std::uint64_t const i(SNAP_CATCH2_NAMESPACE::rand64());
            {
                std::stringstream ss;
                ss << "0B" << to_binary(i);
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS64, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS64, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS64, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS64, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> uint64")
    {
        for(int j(0); j < 100; ++j)
        {
            std::uint64_t const i(SNAP_CATCH2_NAMESPACE::rand64());
            {
                std::stringstream ss;
                ss << "0B" << to_binary(i);
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT64, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT64, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT64, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT64, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> int64")
    {
        for(int j(0); j < 100; ++j)
        {
            std::int64_t const i(j == 0 ? 0 : SNAP_CATCH2_NAMESPACE::rand64());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << "-0B" << to_binary(-i);
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT64, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << '-' << std::showbase << std::oct << -i;
                }
                else
                {
                    ss << std::showbase << std::oct << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT64, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT64, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << '-' << std::showbase << std::hex << std::uppercase << -i;
                }
                else
                {
                    ss << std::showbase << std::hex << std::uppercase << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT64, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> oid")
    {
        for(int j(0); j < 100; ++j)
        {
            std::uint64_t const i(j == 0 ? 0 : SNAP_CATCH2_NAMESPACE::rand64());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_OID, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_OID, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_OID, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_OID, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_OID, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_OID, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_OID, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_OID, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> reference")
    {
        for(int j(0); j < 100; ++j)
        {
            std::uint64_t const i(j == 0 ? 0 : SNAP_CATCH2_NAMESPACE::rand64());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> bits128")
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        for(int j(0); j < 100; ++j)
        {
            unsigned __int128 const i(j == 0 ? 0 : SNAP_CATCH2_NAMESPACE::rand128());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS128, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::showbase << std::oct << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS128, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS128, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS128, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
#pragma GCC diagnostic pop
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> uint128")
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        for(int j(0); j < 100; ++j)
        {
            unsigned __int128 const i(j == 0 ? 0 : SNAP_CATCH2_NAMESPACE::rand128());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT128, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT128, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT128, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT128, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
#pragma GCC diagnostic pop
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> int128")
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        for(int j(0); j < 100; ++j)
        {
            __int128 const i(j == 0 ? 0 : SNAP_CATCH2_NAMESPACE::rand128());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << "-0B" << to_binary(-i);
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT128, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << '-' << std::showbase << std::oct << -i;
                }
                else
                {
                    ss << std::showbase << std::oct << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT128, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT128, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i < 0)
                {
                    ss << '-' << std::showbase << std::hex << std::uppercase << -i;
                }
                else
                {
                    ss << std::showbase << std::hex << std::uppercase << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT128, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
#pragma GCC diagnostic pop
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> bits256")
    {
        for(int j(0); j < 100; ++j)
        {
            prinbee::uint512_t i;
            if(j != 0)
            {
                SNAP_CATCH2_NAMESPACE::rand512(i);
                i.f_value[4] = 0;
                i.f_value[5] = 0;
                i.f_value[6] = 0;
                i.f_value[7] = 0;
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS256, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS256, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS256, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS256, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> uint256")
    {
        for(int j(0); j < 100; ++j)
        {
            prinbee::uint512_t i;
            if(j != 0)
            {
                SNAP_CATCH2_NAMESPACE::rand512(i);
                i.f_value[4] = 0;
                i.f_value[5] = 0;
                i.f_value[6] = 0;
                i.f_value[7] = 0;
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT256, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT256, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT256, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT256, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> int256")
    {
        for(int j(0); j < 100; ++j)
        {
            prinbee::int512_t i;
            if(j != 0)
            {
                SNAP_CATCH2_NAMESPACE::rand512(i);
                if(i.f_value[3] & 0x8000000000000000UL)
                {
                    i.f_value[4] = 0xFFFFFFFFFFFFFFFF;
                    i.f_value[5] = 0xFFFFFFFFFFFFFFFF;
                    i.f_value[6] = 0xFFFFFFFFFFFFFFFF;
                    i.f_value[7] = 0xFFFFFFFFFFFFFFFF;
                }
                else
                {
                    i.f_value[4] = 0;
                    i.f_value[5] = 0;
                    i.f_value[6] = 0;
                    i.f_value[7] = 0;
                }
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << "-0B" << to_binary(-i);
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT256, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << '-' << std::showbase << std::oct << -i;
                }
                else
                {
                    ss << std::showbase << std::oct << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT256, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT256, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT256, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> bits512")
    {
        for(int j(0); j < 100; ++j)
        {
            prinbee::uint512_t i;
            SNAP_CATCH2_NAMESPACE::rand512(i);
            {
                std::stringstream ss;
                ss << "0B" << to_binary(i);
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS512, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS512, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS512, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << "0";
                }
                else
                {
                    ss << std::hex << std::uppercase << "0X" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS512, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> uint512")
    {
        for(int j(0); j < 100; ++j)
        {
            prinbee::uint512_t i;
            SNAP_CATCH2_NAMESPACE::rand512(i);
            {
                std::stringstream ss;
                ss << "0B" << to_binary(i);
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT512, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::oct << '0' << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT512, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT512, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT512, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> int512")
    {
        for(int j(0); j < 100; ++j)
        {
            prinbee::int512_t i;
            if(j != 0)
            {
                SNAP_CATCH2_NAMESPACE::rand512(i);
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << "-0B" << to_binary(-i);
                }
                else
                {
                    ss << "0B" << to_binary(i);
                }
                prinbee::buffer_t buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT512, buffer, 2));

                CATCH_REQUIRE(ss.str() == back);

                // the buffer can be shorten in this case which hits a specific
                // case when the value is negative; the following loop is used
                // to test all 63 bytes
                //
                if(i < 0)
                {
                    for(int pos(63); pos > 0; --pos)
                    {
                        buffer[pos] = 0xFF;
                        buffer[pos - 1] |= 0x80;

                        // the output of a negative number in binary actually
                        // outputs the positive version of the number with a
                        // '-' sign in front of it (since we deal primarily
                        // with small numbers, this means we often will have
                        // very small output in number of characters)
                        // so here I create a copy of buffer and compute
                        // the additive inverse
                        //
                        prinbee::buffer_t binary(buffer);
                        int carry(1);
                        for(int neg(0); neg < 64; ++neg) // WARNING: this assumes little endian
                        {
                            binary[neg] = ~binary[neg] + carry;
                            carry = binary[neg] == 0 && carry == 1 ? 1 : 0;
                        }
                        std::stringstream sn;
                        sn << "-0B";
                        bool found(false);
                        for(int byte(63); byte >= 0; --byte)
                        {
                            int bit(8);
                            while(bit > 0)
                            {
                                --bit;
                                if((binary[byte] & (1 << bit)) != 0)
                                {
                                    sn << '1';
                                    found = true;
                                }
                                else if(found)
                                {
                                    sn << '0';
                                }
                            }
                        }
                        if(!found) // if input is 0
                        {
                            sn << '0';
                        }
                        std::string const small(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT512, buffer, 2));
                        CATCH_REQUIRE(sn.str() == small);
                    }
                }
            }
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else if(i < 0)
                {
                    ss << '-' << std::oct << std::showbase << -i;
                }
                else
                {
                    ss << std::oct << std::showbase << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT512, buffer, 8));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::dec << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT512, buffer, 10));

                CATCH_REQUIRE(ss.str() == back);
            }
            {
                std::stringstream ss;
                ss << std::showbase << std::hex << std::uppercase << i;
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT512, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT512, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> float32")
    {
        for(int j(0); j < 100; ++j)
        {
            float const i(SNAP_CATCH2_NAMESPACE::rand32() / (SNAP_CATCH2_NAMESPACE::rand32() | 1));

            std::stringstream ss;
            ss << i;
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT32, std::string(rand() % 10, ' ') + ss.str() + std::string(rand() % 10, ' ')));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_FLOAT32, buffer, 2));

            CATCH_REQUIRE(ss.str() == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> float64")
    {
        for(int j(0); j < 100; ++j)
        {
            double const i(SNAP_CATCH2_NAMESPACE::rand64() / (SNAP_CATCH2_NAMESPACE::rand64() | 1));

            std::stringstream ss;
            ss << i;
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT64, std::string(rand() % 10, ' ') + ss.str() + std::string(rand() % 10, ' ')));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_FLOAT64, buffer, 2));

            CATCH_REQUIRE(ss.str() == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> float128")
    {
        for(int j(0); j < 100; ++j)
        {
            long double const i(SNAP_CATCH2_NAMESPACE::rand128() / (SNAP_CATCH2_NAMESPACE::rand128() | 1));

            std::stringstream ss;
            ss << i;
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT128, std::string(rand() % 10, ' ') + ss.str() + std::string(rand() % 10, ' ')));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_FLOAT128, buffer, 2));

            CATCH_REQUIRE(ss.str() == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> version")
    {
        for(int j(0); j < 100; ++j)
        {
            std::uint16_t vmajor(rand());
            std::uint16_t vminor(rand());

            std::stringstream ss;
            for(int i(rand() % 10 - 5); i > 0; --i)
            {
                ss << ' ';
            }
            if(rand() % 5 == 0)
            {
                ss << 'v';
            }
            ss << vmajor << '.' << vminor;
            for(int i(rand() % 10 - 5); i > 0; --i)
            {
                ss << ' ';
            }
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_VERSION, ss.str()));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_VERSION, buffer, 2));

            std::stringstream expect;
            expect << vmajor << '.' << vminor;
            CATCH_REQUIRE(expect.str() == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> time (seconds)")
    {
        for(int j(0); j < 25; ++j)
        {
            // negative numbers are not that useful to us at the moment
            // and very negative are not representing valid dates
            //
            // note: the 3,000 years is very approximative since I use 365
            //       days per year (to simplify); it still takes use close
            //       enough I think
            //
            constexpr time_t const three_thousand_years(3'000LL * 365LL * 86'400LL);
            time_t const d((SNAP_CATCH2_NAMESPACE::rand64() >> 1) % three_thousand_years);

            // we only output back to UTC so use UTC here
            //
            std::string cmd("date -u +%Y-%m-%dT%T -d @");
            cmd += std::to_string(d);
            FILE * p(popen(cmd.c_str(), "r"));
            CATCH_REQUIRE(p != nullptr);
            char buf[256] = {};
            std::size_t sz(fread(buf, 1, sizeof(buf), p));
            CATCH_REQUIRE(sz >= 1);
            CATCH_REQUIRE(sz < sizeof(buf));
            if(buf[sz - 1] == '\n')
            {
                --sz;
            }
            buf[sz] = '\0';
            CATCH_REQUIRE(pclose(p) == 0);

            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_TIME, buf));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_TIME, buffer, 2));

            // the prinbee always includes the timezone (+0000)
            //
            CATCH_REQUIRE(std::string(buf) + "+0000" == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> time (seconds + timezone)")
    {
        for(int j(0); j < 25; ++j)
        {
            // negative numbers are not that useful to us at the moment
            // and very negative are not representing valid dates
            //
            // note: the 3,000 years is very approximative since I use 365
            //       days per year (to simplify); it still takes use close
            //       enough I think
            //
            constexpr time_t const three_thousand_years(3'000LL * 365LL * 86'400LL);
            time_t const d((SNAP_CATCH2_NAMESPACE::rand64() >> 1) % three_thousand_years);

            // we only output back to UTC so use UTC here
            //
            std::string cmd("date -u +%Y-%m-%dT%T%z -d @");
            cmd += std::to_string(d);
            FILE * p(popen(cmd.c_str(), "r"));
            CATCH_REQUIRE(p != nullptr);
            char buf[256] = {};
            std::size_t sz(fread(buf, 1, sizeof(buf), p));
            CATCH_REQUIRE(sz >= 1);
            CATCH_REQUIRE(sz < sizeof(buf));
            if(buf[sz - 1] == '\n')
            {
                --sz;
            }
            buf[sz] = '\0';
            CATCH_REQUIRE(pclose(p) == 0);

            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_TIME, buf));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_TIME, buffer, 2));

            CATCH_REQUIRE(buf == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> time (milliseconds + timezone)")
    {
        for(int j(0); j < 10; ++j)
        {
            // negative numbers are not that useful to us at the moment
            // and very negative are not representing valid dates
            //
            // note: the 3,000 years is very approximative since I use 365
            //       days per year (to simplify); it still takes use close
            //       enough I think
            //
            constexpr time_t const three_thousand_years(3'000LL * 365LL * 86'400LL);
            time_t const d((SNAP_CATCH2_NAMESPACE::rand64() >> 1) % three_thousand_years);
            std::uint32_t const ms(rand() % 10); // 0 to 9

            // we only output back to UTC so use UTC here
            //
            std::string cmd("date -u +%Y-%m-%dT%T.");
            cmd += std::to_string(ms);
            cmd += "%z -d @";
            cmd += std::to_string(d);
            FILE * p(popen(cmd.c_str(), "r"));
            CATCH_REQUIRE(p != nullptr);
            char buf[256] = {};
            std::size_t sz(fread(buf, 1, sizeof(buf), p));
            CATCH_REQUIRE(sz >= 1);
            CATCH_REQUIRE(sz < sizeof(buf));
            if(buf[sz - 1] == '\n')
            {
                --sz;
            }
            buf[sz] = '\0';
            CATCH_REQUIRE(pclose(p) == 0);

            std::string mstime(buf);
            std::string::size_type const pos(mstime.find('+'));
            CATCH_REQUIRE(pos != std::string::npos);
            mstime =
                  mstime.substr(0, pos)
                + "00"
                + mstime.substr(pos);

            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_MSTIME, buf));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_MSTIME, buffer, 2));

            CATCH_REQUIRE(mstime == back);
        }
        for(int j(0); j < 10; ++j)
        {
            // negative numbers are not that useful to us at the moment
            // and very negative are not representing valid dates
            //
            // note: the 3,000 years is very approximative since I use 365
            //       days per year (to simplify); it still takes use close
            //       enough I think
            //
            constexpr time_t const three_thousand_years(3'000LL * 365LL * 86'400LL);
            time_t const d((SNAP_CATCH2_NAMESPACE::rand64() >> 1) % three_thousand_years);
            std::uint32_t const ms(rand() % (100 - 10) + 10); // 10 to 99

            // we only output back to UTC so use UTC here
            //
            std::string cmd("date -u +%Y-%m-%dT%T.");
            cmd += std::to_string(ms);
            cmd += "%z -d @";
            cmd += std::to_string(d);
            FILE * p(popen(cmd.c_str(), "r"));
            CATCH_REQUIRE(p != nullptr);
            char buf[256] = {};
            std::size_t sz(fread(buf, 1, sizeof(buf), p));
            CATCH_REQUIRE(sz >= 1);
            CATCH_REQUIRE(sz < sizeof(buf));
            if(buf[sz - 1] == '\n')
            {
                --sz;
            }
            buf[sz] = '\0';
            CATCH_REQUIRE(pclose(p) == 0);

            std::string mstime(buf);
            std::string::size_type const pos(mstime.find('+'));
            CATCH_REQUIRE(pos != std::string::npos);
            mstime =
                  mstime.substr(0, pos)
                + "0"
                + mstime.substr(pos);

            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_MSTIME, buf));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_MSTIME, buffer, 2));

            CATCH_REQUIRE(mstime == back);
        }
        for(int j(0); j < 10; ++j)
        {
            // negative numbers are not that useful to us at the moment
            // and very negative are not representing valid dates
            //
            // note: the 3,000 years is very approximative since I use 365
            //       days per year (to simplify); it still takes use close
            //       enough I think
            //
            constexpr time_t const three_thousand_years(3'000LL * 365LL * 86'400LL);
            time_t const d((SNAP_CATCH2_NAMESPACE::rand64() >> 1) % three_thousand_years);
            std::uint32_t const ms(rand() % 1'000);

            // we only output back to UTC so use UTC here
            //
            std::string cmd("date -u +%Y-%m-%dT%T.");
            if(ms < 10)
            {
                cmd += '0';
            }
            if(ms < 100)
            {
                cmd += '0';
            }
            cmd += std::to_string(ms);
            cmd += "%z -d @";
            cmd += std::to_string(d);
            FILE * p(popen(cmd.c_str(), "r"));
            CATCH_REQUIRE(p != nullptr);
            char buf[256] = {};
            std::size_t sz(fread(buf, 1, sizeof(buf), p));
            CATCH_REQUIRE(sz >= 1);
            CATCH_REQUIRE(sz < sizeof(buf));
            if(buf[sz - 1] == '\n')
            {
                --sz;
            }
            buf[sz] = '\0';
            CATCH_REQUIRE(pclose(p) == 0);

            std::string mstime(buf);
            int const extra_zeroes(rand() % 4);
            if(extra_zeroes > 0)
            {
                std::string::size_type const pos(mstime.find('+'));
                if(pos != std::string::npos)
                {
                    mstime =
                          mstime.substr(0, pos)
                        + std::string(extra_zeroes, '0')
                        + mstime.substr(pos + 1);
                }
            }

            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_MSTIME, mstime));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_MSTIME, buffer, 2));

            CATCH_REQUIRE(buf == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> time (microseconds + timezone)")
    {
        for(int j(0); j < 25; ++j)
        {
            // negative numbers are not that useful to us at the moment
            // and very negative are not representing valid dates
            //
            // note: the 3,000 years is very approximative since I use 365
            //       days per year (to simplify); it still takes use close
            //       enough I think
            //
            constexpr time_t const three_thousand_years(3'000LL * 365LL * 86'400LL);
            time_t const d((SNAP_CATCH2_NAMESPACE::rand64() >> 1) % three_thousand_years);
            std::uint32_t const us(SNAP_CATCH2_NAMESPACE::rand32() % 1'000'000LL);

            // we only output back to UTC so use UTC here
            //
            std::string cmd("date -u +%Y-%m-%dT%T.");
            if(us < 10)
            {
                cmd += '0';
            }
            if(us < 100)
            {
                cmd += '0';
            }
            if(us < 1'000)
            {
                cmd += '0';
            }
            if(us < 10'000)
            {
                cmd += '0';
            }
            if(us < 100'000)
            {
                cmd += '0';
            }
            cmd += std::to_string(us);
            cmd += "%z -d @";
            cmd += std::to_string(d);
            FILE * p(popen(cmd.c_str(), "r"));
            CATCH_REQUIRE(p != nullptr);
            char buf[256] = {};
            std::size_t sz(fread(buf, 1, sizeof(buf), p));
            CATCH_REQUIRE(sz >= 1);
            CATCH_REQUIRE(sz < sizeof(buf));
            if(buf[sz - 1] == '\n')
            {
                --sz;
            }
            buf[sz] = '\0';
            CATCH_REQUIRE(pclose(p) == 0);

            std::string ustime(buf);
            int const extra_zeroes(rand() % 4);
            if(extra_zeroes > 0)
            {
                std::string::size_type const pos(ustime.find('+'));
                if(pos != std::string::npos)
                {
                    ustime =
                          ustime.substr(0, pos)
                        + std::string(extra_zeroes, '0')
                        + ustime.substr(pos + 1);
                }
            }

            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_USTIME, ustime));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_USTIME, buffer, 2));

            CATCH_REQUIRE(buf == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> p8string")
    {
        for(int j(0); j < 256; ++j)
        {
            std::string const str(SNAP_CATCH2_NAMESPACE::rand_string(j));
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_P8STRING, str));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P8STRING, buffer, rand()));

            CATCH_REQUIRE(str == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> p16string")
    {
        for(int j(0); j < 10; ++j)
        {
            int const len(j == 0 ? 0 : (j == 1 ? 65'535 : rand() % 1'000));
            std::string const str(SNAP_CATCH2_NAMESPACE::rand_string(len));
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_P16STRING, str));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P16STRING, buffer, rand()));

            CATCH_REQUIRE(str == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> p32string")
    {
        for(int j(0); j < 10; ++j)
        {
            int const len(j == 0 ? 0 : (j == 1 ? 100'000 : rand() % 2'500));
            std::string const str(SNAP_CATCH2_NAMESPACE::rand_string(len));
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_P32STRING, str));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P32STRING, buffer, rand()));

            CATCH_REQUIRE(str == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> buffer8")
    {
        for(int j(0); j < 256; ++j)
        {
            prinbee::buffer_t buffer(j + 1);
            std::string str;
            buffer[0] = j;
            for(int i(1); i <= j; ++i)
            {
                buffer[i] = rand();
                str += snapdev::to_hex(buffer[i] >> 4);
                str += snapdev::to_hex(buffer[i] & 15);
            }

            prinbee::buffer_t const pbuffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BUFFER8, str));
            CATCH_REQUIRE(buffer == pbuffer);

            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER8, pbuffer, rand()));
            CATCH_REQUIRE(str == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> buffer16")
    {
        for(int j(0); j < 100; ++j)
        {
            std::size_t const size(
                    j == 0
                        ? 0
                        : j == 1
                            ? 65535
                            : rand() & 0xFFFF);

            prinbee::buffer_t buffer(size + 2);
            std::string str;
            buffer[0] = size;          // Little Endian specific
            buffer[1] = size >> 8;
            for(std::size_t i(2); i < size + 2; ++i)
            {
                buffer[i] = rand();
                str += snapdev::to_hex(buffer[i] >> 4);
                str += snapdev::to_hex(buffer[i] & 15);
            }

            prinbee::buffer_t const pbuffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BUFFER16, str));
            CATCH_REQUIRE(buffer == pbuffer);

            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER16, pbuffer, rand()));
            CATCH_REQUIRE(str == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> buffer32")
    {
        for(int j(0); j < 10; ++j)
        {
            std::size_t const size(j == 0 ? 0 : SNAP_CATCH2_NAMESPACE::rand32() % 100'000);

            prinbee::buffer_t buffer(size + 4);
            std::string str;
            buffer[0] = size;          // Little Endian specific
            buffer[1] = size >> 8;
            buffer[2] = size >> 16;
            buffer[3] = size >> 24;
            for(std::size_t i(4); i < size + 4; ++i)
            {
                buffer[i] = rand();
                str += snapdev::to_hex(buffer[i] >> 4);
                str += snapdev::to_hex(buffer[i] & 15);
            }

            prinbee::buffer_t const pbuffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BUFFER32, str));
            CATCH_REQUIRE(buffer == pbuffer);

            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER32, pbuffer, rand()));
            CATCH_REQUIRE(str == back);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("convert_errors", "[convert] [invalid]")
{
    CATCH_START_SECTION("convert_errors: too large")
    {
        for(int i(0); i < 61; ++i)
        {
            std::string s(std::to_string(1ULL << (i + 1)));

            CATCH_REQUIRE_THROWS_MATCHES(
                      prinbee::convert_to_int(s, i)
                    , prinbee::out_of_range
                    , Catch::Matchers::ExceptionMessage(
                              "out_of_range: number \""
                            + s
                            + "\" too large for a signed "
                            + std::to_string(i)
                            + " bit value."));

            CATCH_REQUIRE_THROWS_MATCHES(
                      prinbee::convert_to_uint(s, i)
                    , prinbee::out_of_range
                    , Catch::Matchers::ExceptionMessage(
                              "out_of_range: number \""
                            + s
                            + "\" too large for a signed "
                            + std::to_string(i)
                            + " bit value."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: negative uint")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::convert_to_uint("-64", 4)
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage("prinbee_exception: negative values are not accepted, \"-64\" is not valid."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: missing closing quote")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::convert_to_int("-X'64", 4)
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage("prinbee_exception: closing quote missing in \"-X'64\"."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: data when no unit is expected")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::convert_to_int("64 m", 120)
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage("prinbee_exception: could not convert number \"64 m\" to a valid uint512_t value (spurious data found after number)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: number too large for bits8 type")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS8, "256")
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: number \"256\" too large for an 8 bit value."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: number too large for uint8 type")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT8, "256")
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: number \"256\" too large for an 8 bit value."));

        prinbee::buffer_t buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT512, "256"));
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT8, buffer, 10)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: value too large (512 bits) for this field (max: 8 bits)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: number too large for int8 type")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT8, "256")
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: number \"256\" too large for a signed 8 bit value."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT8, "128")
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: number \"128\" too large for a signed 8 bit value."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT8, "-129")
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: number \"-129\" too large for a signed 8 bit value."));

        {
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT16, "-129"));
            CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT8, buffer, 10)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: value too large (16 bits) for this field (max: 8 bits)."));
        }

        // simple negative
        {
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT8, "-127"));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT8, buffer, 10));
            CATCH_REQUIRE("-127" == back);
        }

        // I have this one here because -(-128) = -128 in an 8 bit number
        // (and larger) and this could look like an overflow if not
        // properly handled
        {
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT8, "-128"));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT8, buffer, 10));
            CATCH_REQUIRE("-128" == back);
        }
        {
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT16, "-32768"));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT16, buffer, 10));
            CATCH_REQUIRE("-32768" == back);
        }
        {
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT32, "-2147483648"));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT32, buffer, 10));
            CATCH_REQUIRE("-2147483648" == back);
        }
        {
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT64, "-9223372036854775808"));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT64, buffer, 10));
            CATCH_REQUIRE("-9223372036854775808" == back);
        }
        {
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT128, "-170141183460469231731687303715884105728"));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT128, buffer, 10));
            CATCH_REQUIRE("-170141183460469231731687303715884105728" == back);
        }
        {
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT256, "-57896044618658097711785492504343953926634992332820282019728792003956564819968"));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT256, buffer, 10));
            CATCH_REQUIRE("-57896044618658097711785492504343953926634992332820282019728792003956564819968" == back);
        }
        {
            prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_INT512, "-6703903964971298549787012499102923063739682910296196688861780721860882015036773488400937149083451713845015929093243025426876941405973284973216824503042048"));
            std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_INT512, buffer, 10));
            CATCH_REQUIRE("-6703903964971298549787012499102923063739682910296196688861780721860882015036773488400937149083451713845015929093243025426876941405973284973216824503042048" == back);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: unknown base")
    {
        prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS8, "100"));
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS8, buffer, 37)
                , prinbee::conversion_unavailable
                , Catch::Matchers::ExceptionMessage("prinbee_exception: base 37 not supported."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: mismatch -> value too large")
    {
        prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT16, "256"));
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS8, buffer, 10)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: value too large (16 bits) for this field (max: 8 bits)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: version missing '.'")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_VERSION, "v3")
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage("prinbee_exception: version \"v3\" must include a period (.) between the major and minor numbers."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: version out of range")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_VERSION, "v300000.45")
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: one or both of the major or minor numbers from version \"v300000.45\" are too large for a version number (max. is 65535)."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_VERSION, "v300.450009")
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: one or both of the major or minor numbers from version \"v300.450009\" are too large for a version number (max. is 65535)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: buffer does not match version")
    {
        prinbee::buffer_t buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_VERSION, "v3.5"));
        buffer.push_back(1);
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_VERSION, buffer, 10)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: a buffer representing a version must be exactly 4 bytes, not 5."));

        buffer = prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_VERSION, "v5.6");
        buffer.pop_back();
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_VERSION, buffer, 10)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: a buffer representing a version must be exactly 4 bytes, not 3."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: floats out of range")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT32, "3.40282346638528859811704183485E+39")
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: floating point number \"3.40282346638528859811704183485E+39\" out of range."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT64, "1.79769313486231570814527423732E+309")
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: floating point number \"1.79769313486231570814527423732E+309\" out of range."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT128, "5.5E+16389")
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: floating point number \"5.5E+16389\" out of range."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: floats followed by spurious data")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT32, "3.14159k")
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage("prinbee_exception: floating point number \"3.14159k\" includes invalid characters."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT64, "3.14159k")
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage("prinbee_exception: floating point number \"3.14159k\" includes invalid characters."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT128, "3.14159k")
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage("prinbee_exception: floating point number \"3.14159k\" includes invalid characters."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: not floats")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT32, "bad float")
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage("prinbee_exception: floating point number \"bad float\" includes invalid characters."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT64, "another bad float")
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage("prinbee_exception: floating point number \"another bad float\" includes invalid characters."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_FLOAT128, "still a bad float")
                , prinbee::invalid_number
                , Catch::Matchers::ExceptionMessage("prinbee_exception: floating point number \"still a bad float\" includes invalid characters."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: float size mismatch")
    {
        for(std::size_t size(0); size < 20; ++size)
        {
            if(size != sizeof(float))
            {
                prinbee::buffer_t buffer(size);
                CATCH_REQUIRE_THROWS_MATCHES(
                          prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_FLOAT32, buffer)
                        , prinbee::out_of_range
                        , Catch::Matchers::ExceptionMessage("out_of_range: value buffer has an unexpected size ("
                                + std::to_string(size)
                                + ") for this field (expected floating point size: "
                                + std::to_string(sizeof(float))
                                + ")."));
            }
            if(size != sizeof(double))
            {
                prinbee::buffer_t buffer(size);
                CATCH_REQUIRE_THROWS_MATCHES(
                          prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_FLOAT64, buffer)
                        , prinbee::out_of_range
                        , Catch::Matchers::ExceptionMessage("out_of_range: value buffer has an unexpected size ("
                                + std::to_string(size)
                                + ") for this field (expected floating point size: "
                                + std::to_string(sizeof(double))
                                + ")."));
            }
            if(size != sizeof(long double))
            {
                prinbee::buffer_t buffer(size);
                CATCH_REQUIRE_THROWS_MATCHES(
                          prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_FLOAT128, buffer)
                        , prinbee::out_of_range
                        , Catch::Matchers::ExceptionMessage("out_of_range: value buffer has an unexpected size ("
                                + std::to_string(size)
                                + ") for this field (expected floating point size: "
                                + std::to_string(sizeof(long double))
                                + ")."));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: p8string too large")
    {
        for(int j(256); j < 300; ++j)
        {
            std::string const str(SNAP_CATCH2_NAMESPACE::rand_string(j));
            CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_P8STRING, str)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                      "out_of_range: string too long ("
                    + std::to_string(j)
                    + ") for this field (max: 255)."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: p16string too large")
    {
        for(int j(65536); j <= 65536 + 300; ++j)
        {
            std::string const str(SNAP_CATCH2_NAMESPACE::rand_string(j));
            CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_P16STRING, str)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                      "out_of_range: string too long ("
                    + std::to_string(j)
                    + ") for this field (max: 65535)."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: buffer8 too large")
    {
        for(int j(256); j < 300; ++j)
        {
            std::string str;
            str.reserve(j * 2);
            for(int i(0); i < j; ++i)
            {
                int const v(rand() & 0x0FF);
                str += snapdev::to_hex(v >> 4);
                str += snapdev::to_hex(v & 15);
            }

            CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BUFFER8, str)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                      "out_of_range: number of bytes in value is too large ("
                    + std::to_string(j)
                    + ") for a buffer8."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: buffer16 too large")
    {
        for(int j(65536); j <= 65536 + 300; ++j)
        {
            std::string str;
            str.reserve(j * 2);
            for(int i(0); i < j; ++i)
            {
                int const v(rand() & 0xFF);
                str += snapdev::to_hex(v >> 4);
                str += snapdev::to_hex(v & 15);
            }

            CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BUFFER16, str)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                      "out_of_range: number of bytes in value is too large ("
                    + std::to_string(j)
                    + ") for a buffer16."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: input too small for buffer size")
    {
        prinbee::buffer_t buffer;

        // empty
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER8, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-Buffer size (0, expected at least: 1)."));

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER16, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-Buffer size (0, expected at least: 2)."));

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER32, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-Buffer size (0, expected at least: 4)."));

        // size 1
        buffer.push_back(1);

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER16, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-Buffer size (1, expected at least: 2)."));

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER32, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-Buffer size (1, expected at least: 4)."));

        // size 2
        buffer.push_back(1);

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER32, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-Buffer size (2, expected at least: 4)."));

        // size 3
        buffer.push_back(1);

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER32, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-Buffer size (3, expected at least: 4)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: input too small for P-Buffer data")
    {
        prinbee::buffer_t buffer;

        // buffer8
        //
        buffer.push_back(5);
        buffer.push_back('1');
        buffer.push_back('2');
        buffer.push_back('3');
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER8, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer (size: 4 including 1 bytes for the size) too small for the requested number of bytes (6)."));

        // buffer16
        //
        buffer.clear();
        buffer.push_back(0);    // size 256 (little endian)
        buffer.push_back(1);
        buffer.push_back('1');
        buffer.push_back('2');
        buffer.push_back('3');
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER16, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer (size: 5 including 2 bytes for the size) too small for the requested number of bytes (258)."));

        // buffer32
        //
        buffer.clear();
        buffer.push_back(44);
        buffer.push_back(0);
        buffer.push_back(0);
        buffer.push_back(0);
        buffer.push_back('1');
        buffer.push_back('2');
        buffer.push_back('3');
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER32, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer (size: 7 including 4 bytes for the size) too small for the requested number of bytes (48)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: buffer too small for P-String size")
    {
        prinbee::buffer_t buffer;

        // empty
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P8STRING, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-String size (0, expected at least: 1)."));

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P16STRING, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-String size (0, expected at least: 2)."));

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P32STRING, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-String size (0, expected at least: 4)."));

        // size 1
        buffer.push_back(1);

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P16STRING, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-String size (1, expected at least: 2)."));

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P32STRING, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-String size (1, expected at least: 4)."));

        // size 2
        buffer.push_back(1);

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P32STRING, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-String size (2, expected at least: 4)."));

        // size 3
        buffer.push_back(1);

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P32STRING, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small to incorporate the P-String size (3, expected at least: 4)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: buffer too small for P-String data")
    {
        prinbee::buffer_t buffer;

        // P8 string
        //
        buffer.push_back(5);
        buffer.push_back('1');
        buffer.push_back('2');
        buffer.push_back('3');
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P8STRING, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small for the P-String characters (size: 5, character bytes in buffer: 3)."));

        // P16 string
        //
        buffer.clear();
        buffer.push_back(0);    // size 256 (little endian)
        buffer.push_back(1);
        buffer.push_back('1');
        buffer.push_back('2');
        buffer.push_back('3');
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P16STRING, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small for the P-String characters (size: 256, character bytes in buffer: 3)."));

        // P32 string
        //
        buffer.clear();
        buffer.push_back(44);
        buffer.push_back(0);
        buffer.push_back(0);
        buffer.push_back(0);
        buffer.push_back('1');
        buffer.push_back('2');
        buffer.push_back('3');
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_P32STRING, buffer)
            , prinbee::out_of_range
            , Catch::Matchers::ExceptionMessage(
                  "out_of_range: buffer too small for the P-String characters (size: 44, character bytes in buffer: 3)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: time with too many decimal")
    {
        for(int i(4); i <= 10; ++i)
        {
            constexpr time_t const three_thousand_years(3'000LL * 365LL * 86'400LL);
            time_t const d((SNAP_CATCH2_NAMESPACE::rand64() >> 1) % three_thousand_years);

            // we only output back to UTC so use UTC here
            //
            std::string cmd("date -u +%Y-%m-%dT%T.");
            for(int j(1); j < i; ++j)
            {
                cmd += std::to_string(rand() % 10);
            }
            cmd += std::to_string(rand() % 9 + 1);
            cmd += "%z -d @";
            cmd += std::to_string(d);
            FILE * p(popen(cmd.c_str(), "r"));
            CATCH_REQUIRE(p != nullptr);
            char buf[256] = {};
            std::size_t sz(fread(buf, 1, sizeof(buf), p));
            CATCH_REQUIRE(sz >= 1);
            CATCH_REQUIRE(sz < sizeof(buf));
            if(buf[sz - 1] == '\n')
            {
                --sz;
            }
            buf[sz] = '\0';
            CATCH_REQUIRE(pclose(p) == 0);

            std::string mstime(buf);
            std::string::size_type const pos(mstime.find('+'));
            CATCH_REQUIRE(pos != std::string::npos);

            CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_MSTIME, buf)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                      "out_of_range: time fraction is out of bounds in \""
                    + std::string(buf)
                    + "\" (expected 3 digits, found "
                    + std::to_string(i)
                    + ")."));

            if(i > 6)
            {
                CATCH_REQUIRE_THROWS_MATCHES(
                      prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_USTIME, buf)
                    , prinbee::out_of_range
                    , Catch::Matchers::ExceptionMessage(
                          "out_of_range: time fraction is out of bounds in \""
                        + std::string(buf)
                        + "\" (expected 6 digits, found "
                        + std::to_string(i)
                        + ")."));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: wrong buffer size for time")
    {
        prinbee::buffer_t buffer;
        for(int i(0); i < 10; ++i)
        {
            CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_TIME, buffer)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                      "out_of_range: buffer size is invalid for a time value (size: "
                    + std::to_string(buffer.size())
                    + ", expected size: 8)."));

            CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_MSTIME, buffer)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                      "out_of_range: buffer size is invalid for a time value (size: "
                    + std::to_string(buffer.size())
                    + ", expected size: 8)."));

            CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_USTIME, buffer)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                      "out_of_range: buffer size is invalid for a time value (size: "
                    + std::to_string(buffer.size())
                    + ", expected size: 8)."));

            buffer.push_back(rand());
            if(buffer.size() == sizeof(std::uint64_t))
            {
                buffer.push_back(rand());
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: unexpected structure type")
    {
        std::vector<prinbee::struct_type_t> unsupported_types{
            prinbee::struct_type_t::STRUCT_TYPE_ARRAY8,
            prinbee::struct_type_t::STRUCT_TYPE_ARRAY16,
            prinbee::struct_type_t::STRUCT_TYPE_ARRAY32,
            prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE,
            prinbee::struct_type_t::STRUCT_TYPE_END,
            prinbee::struct_type_t::STRUCT_TYPE_VOID,
            prinbee::struct_type_t::STRUCT_TYPE_RENAMED,
        };

        for(auto const t : unsupported_types)
        {
            CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(t, "ignored")
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                      "logic_error: unexpected structure type ("
                    + std::to_string(static_cast<int>(t))
                    + ") to convert a string to a buffer"));

            prinbee::buffer_t ignored;
            CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(t, ignored)
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                      "logic_error: unexpected structure type ("
                    + std::to_string(static_cast<int>(t))
                    + ") to convert a string to a buffer"));
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
