// Copyright (c) 2006-2024  Made to Order Software Corp.  All Rights Reserved
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
#pragma once

// snapcatch2
//
#include    <catch2/snapcatch2.hpp>


// prinbee
//
#include    <prinbee/bigint/uint512.h>


// snaplogger
//
#include    <snaplogger/snapcatch2.hpp>


// C++
//
#include    <string>
#include    <cstring>
#include    <cstdlib>
#include    <iostream>



namespace SNAP_CATCH2_NAMESPACE
{



std::string setup_context(std::string const & path, std::vector<std::string> const & xmls);


inline char32_t rand_char(bool full_range = false)
{
    // -1 so we can avoid '\0' which in most cases is not useful
    //
    char32_t const max((full_range ? 0x0110000 : 0x0010000) - (0xE000 - 0xD800) - 1);

    char32_t const wc(((rand() << 16) ^ rand()) % max + 1);

    // skip the surrogates for the larger characters
    //
    return wc >= 0xD800 ?  wc + (0xE000 - 0xD800) : wc;
}


inline std::string rand_string(int len = rand() % 200 + 10)
{
    std::string result;
    for(int i(0); i < len; ++i)
    {
        result += rand() % 26 + 'a';
    }
    return result;
}


inline std::uint32_t rand32()
{
    return rand() ^ (rand() << 16);
}


inline std::uint64_t rand64()
{
    std::uint64_t const a(rand());
    std::uint64_t const b(rand());
    std::uint64_t const c(rand());
    std::uint64_t const d(rand());
    return a ^ (b << 16) ^ (c << 32) ^ (d << 48);
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
inline unsigned __int128 rand128()
{
    unsigned __int128 const a(rand());
    unsigned __int128 const b(rand());
    unsigned __int128 const c(rand());
    unsigned __int128 const d(rand());
    unsigned __int128 const e(rand());
    unsigned __int128 const f(rand());
    unsigned __int128 const g(rand());
    unsigned __int128 const h(rand());
    return (a <<  0) ^ (b << 16) ^ (c << 32) ^ (d <<  48)
         ^ (e << 64) ^ (f << 80) ^ (g << 96) ^ (h << 112);
}
#pragma GCC diagnostic pop


inline void rand512(prinbee::uint512_t & a)
{
    for(std::size_t i(0); i < 8; ++i)
    {
        a.f_value[i] = SNAP_CATCH2_NAMESPACE::rand64();
    }
}


inline void rand512(prinbee::int512_t & a)
{
    for(std::size_t i(0); i < 7; ++i)
    {
        a.f_value[i] = SNAP_CATCH2_NAMESPACE::rand64();
    }
    a.f_high_value = SNAP_CATCH2_NAMESPACE::rand64();
}



} // namespace SNAP_CATCH2_NAMESPACE
// vim: ts=4 sw=4 et
