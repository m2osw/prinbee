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
 * \brief Handle a block structure.
 *
 * Each block contains a structure. The very first four bytes are always the
 * magic characters which define the type of the block. The remained of the
 * block is a _lose_ structure which very often changes in size because it
 * includes parameters such as a string or an array.
 *
 * Also in most cases arrays are also themselvess _lose_ structures (a few
 * are just numbers such as column ids or block references.)
 *
 * IMPORTANT: The types defined here are also the types that we accept in
 * a user table. Here we define structures and later tables.
 */

// C++
//
#include    <cstdint>
#include    <initializer_list>



namespace prinbee
{



struct uint512_t;

struct int512_t
{
                                    int512_t();
                                    int512_t(int512_t const & rhs);
                                    int512_t(uint512_t const & rhs);
                                    int512_t(std::initializer_list<std::uint64_t> rhs);

    int512_t &                      operator = (int512_t const & rhs) = default;

    bool                            is_positive() const { return f_high_value >= 0; }
    bool                            is_negative() const { return f_high_value < 0; }
    int512_t                        abs() const { if(f_high_value < 0) return -*this; else return *this; }
    int                             compare(int512_t const & rhs) const;

    std::size_t                     bit_size() const;

    int512_t                        operator - () const;
    int512_t &                      operator += (int512_t const & rhs);
    int512_t &                      operator -= (int512_t const & rhs);

    bool                            operator == (int512_t const & rhs) const;
    bool                            operator == (int64_t rhs) const;
    bool                            operator != (int512_t const & rhs) const;
    bool                            operator != (int64_t rhs) const;
    bool                            operator < (int512_t const & rhs) const;
    bool                            operator <= (int512_t const & rhs) const;
    bool                            operator > (int512_t const & rhs) const;
    bool                            operator >= (int512_t const & rhs) const;

    std::uint64_t                   f_value[7] = { 0 };
    std::int64_t                    f_high_value = 0;
};



} // namespace prinbee
// vim: ts=4 sw=4 et
