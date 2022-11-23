// Copyright (c) 2006-2022  Made to Order Software Corp.  All Rights Reserved
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
#include <catch2/snapcatch2.hpp>


// C++
//
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>



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



} // namespace SNAP_CATCH2_NAMESPACE
// vim: ts=4 sw=4 et
