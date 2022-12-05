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
#include    <snapdev/math.h>
#include    <snapdev/ostream_int128.h>


// C++
//
#include    <fstream>


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
    prinbee::uint512_t b;
    b.f_value[7] = 0x8000000000000000ULL;
    for(;; b >>= 1)
    {
        if((n & b) != 0)
        {
            break;
        }
    }
    if(b.is_zero())
    {
        return "0";
    }
    std::string result;
    while(!b.is_zero())
    {
        result += (n & b) != 0 ? '1' : '0';
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
                ss << "0X" << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0x" << std::hex << i;
                uint64_t const c(prinbee::convert_to_uint(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0X" << std::hex << i;
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
                ss << "0x" << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0X" << std::hex << std::uppercase << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0x" << std::hex << i;
                uint64_t const c(prinbee::convert_to_int(decorate(ss.str()), 8));
                CATCH_REQUIRE(c == i);
            }

            {
                std::stringstream ss;
                ss << "0X" << std::hex << i;
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
//buffer_t string_to_typed_buffer(struct_type_t type, std::string const & value);
//std::string typed_buffer_to_string(struct_type_t type, buffer_t value, int base);
    CATCH_START_SECTION("convert_buffer: string -> bits8")
    {
        for(int i(0); i < 256; ++i)
        {
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS8, buffer, 2));

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
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
                }
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
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
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
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT8, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT8, buffer, 16));

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
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
                }
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
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
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
                    ss << "0b" << to_binary(i);
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
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT16, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT16, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> bits32")
    {
        for(int j(0); j < 100; ++j)
        {
            std::uint32_t const i(rand() ^ (rand() << 16));
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
                }
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
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
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
            std::uint32_t const i(rand() ^ (rand() << 16));
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
                }
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
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT32, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT32, buffer, 16));

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
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
                }
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
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
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
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
                }
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
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT64, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT64, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_buffer: string -> oid")
    {
        for(int j(0); j < 100; ++j)
        {
            std::uint64_t const i(SNAP_CATCH2_NAMESPACE::rand64());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
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
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
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
            std::uint64_t const i(SNAP_CATCH2_NAMESPACE::rand64());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
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
                    ss << std::hex << std::uppercase << "0x" << i;
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
            unsigned __int128 const i(SNAP_CATCH2_NAMESPACE::rand128());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS128, buffer, 2));

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
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
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
            unsigned __int128 const i(SNAP_CATCH2_NAMESPACE::rand128());
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
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
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT128, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT128, buffer, 16));

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
            SNAP_CATCH2_NAMESPACE::rand512(i);
            i.f_value[4] = 0;
            i.f_value[5] = 0;
            i.f_value[6] = 0;
            i.f_value[7] = 0;
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
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
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
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
            SNAP_CATCH2_NAMESPACE::rand512(i);
            i.f_value[4] = 0;
            i.f_value[5] = 0;
            i.f_value[6] = 0;
            i.f_value[7] = 0;
            {
                std::stringstream ss;
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << "0b" << to_binary(i);
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
                if(i == 0)
                {
                    ss << '0';
                }
                else
                {
                    ss << std::hex << std::uppercase << "0x" << i;
                }
                prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT256, ss.str()));
                std::string const back(prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT256, buffer, 16));

                CATCH_REQUIRE(ss.str() == back);
            }
        }
    }
    CATCH_END_SECTION()

// struct_type_t::STRUCT_TYPE_BITS256:
// struct_type_t::STRUCT_TYPE_UINT256:
// struct_type_t::STRUCT_TYPE_BITS512:
// struct_type_t::STRUCT_TYPE_UINT512:
// struct_type_t::STRUCT_TYPE_INT8:
// struct_type_t::STRUCT_TYPE_INT16:
// struct_type_t::STRUCT_TYPE_INT32:
// struct_type_t::STRUCT_TYPE_INT64:
// struct_type_t::STRUCT_TYPE_INT128:
// struct_type_t::STRUCT_TYPE_INT256:
// struct_type_t::STRUCT_TYPE_INT512:
// struct_type_t::STRUCT_TYPE_FLOAT32:
// struct_type_t::STRUCT_TYPE_FLOAT64:
// struct_type_t::STRUCT_TYPE_FLOAT128:
// struct_type_t::STRUCT_TYPE_VERSION:
// struct_type_t::STRUCT_TYPE_TIME:
// struct_type_t::STRUCT_TYPE_MSTIME:
// struct_type_t::STRUCT_TYPE_USTIME:
// struct_type_t::STRUCT_TYPE_P8STRING:
// struct_type_t::STRUCT_TYPE_P16STRING:
// struct_type_t::STRUCT_TYPE_P32STRING:
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

    CATCH_START_SECTION("convert_errors: number too large for type")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS8, "256")
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: number \"256\" too large for an 8 bit value."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: unknown base")
    {
        prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_BITS8, "100"));
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS8, buffer, 12)
                , prinbee::conversion_unavailable
                , Catch::Matchers::ExceptionMessage("prinbee_exception: base 12 not supported."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("convert_errors: mismatch -> value too large")
    {
        prinbee::buffer_t const buffer(prinbee::string_to_typed_buffer(prinbee::struct_type_t::STRUCT_TYPE_UINT16, "256"));
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::typed_buffer_to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS8, buffer, 10)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: value too large (16) for this field (max: 8)."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
