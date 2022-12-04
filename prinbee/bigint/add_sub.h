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
 * \brief Fast add/sub on very large numbers.
 *
 * These operations are implemented in assembly for amd64 processors. This
 * takes care of the carry/borrow in assembly so the add and subtract
 * operations are very fast.
 */

// C++
//
#include    <cstdint>
//#include    <initializer_list>



namespace prinbee
{



int                 add(std::uint64_t * dst, std::uint64_t const * src1, std::uint64_t const * src2, std::uint64_t count);
void                add128(std::uint64_t * dst, std::uint64_t const * src);
void                add256(std::uint64_t * dst, std::uint64_t const * src);
void                add512(std::uint64_t * dst, std::uint64_t const * src);

int                 sub(std::uint64_t * dst, std::uint64_t const * src1, std::uint64_t const * src2, std::uint64_t count);
void                sub128(std::uint64_t * dst, std::uint64_t const * src);
void                sub256(std::uint64_t * dst, std::uint64_t const * src);
void                sub512(std::uint64_t * dst, std::uint64_t const * src);



} // namespace prinbee
// vim: ts=4 sw=4 et
