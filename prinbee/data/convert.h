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
#pragma once


/** \file
 * \brief Convert XML file data.
 *
 * The convert code is used to transform data from text to binary and vice
 * versa.
 */

// self
//
#include    "prinbee/data/structure.h"



namespace prinbee
{



enum class unit_t
{
    UNIT_NONE,
    UNIT_SIZE
};


std::int64_t convert_to_int(std::string const & value, std::size_t max_size, unit_t unit = unit_t::UNIT_NONE);
std::uint64_t convert_to_uint(std::string const & value, std::size_t max_size, unit_t unit = unit_t::UNIT_NONE);

buffer_t string_to_typed_buffer(struct_type_t type, std::string const & value, std::size_t size = 0);
std::string typed_buffer_to_string(struct_type_t type, buffer_t const & value, int base = 10);



} // namespace prinbee
// vim: ts=4 sw=4 et
