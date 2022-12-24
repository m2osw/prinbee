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
#pragma once


/** \file
 * \brief Utility functions.
 *
 * A few utility functions used within prinbee.
 */

// C++
//
#include    <cstdint>
#include    <string>



namespace prinbee
{



constexpr std::uint64_t round_down(std::uint64_t value, std::uint64_t multiple)
{
    return value - value % multiple;
}


constexpr std::uint64_t round_up(std::uint64_t value, std::uint64_t multiple)
{
    std::uint64_t const adjusted(value + multiple - 1);
    return round_down(adjusted, multiple);
}


constexpr std::uint64_t divide_rounded_up(std::uint64_t value, std::uint64_t multiple)
{
    return (value + multiple - 1) / multiple;
}


bool        validate_name(std::string const & name, size_t const max_length = 255);


} // namespace prinbee
// vim: ts=4 sw=4 et
