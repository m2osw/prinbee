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
 * \brief Handle a block structure.
 *
 * Each block contains a structure. The very first four bytes are always the
 * magic characters which define the type of the block. The remained of the
 * block is a _lose_ structure which very often changes in size because it
 * includes parameters such as a string or an array.
 *
 * Also in most cases arrays are also themselves _lose_ structures (a few
 * are just numbers such as column ids or block references.)
 *
 * IMPORTANT: The types defined here are also the types that we accept in
 * a user table. Here we define structures and later tables.
 */

// self
//
#include    <prinbee/bigint/int512.h>



namespace prinbee
{



struct uint512_t
{
                                    uint512_t();
                                    uint512_t(uint512_t const & rhs);
                                    uint512_t(int512_t const & rhs);
                                    uint512_t(std::initializer_list<std::uint64_t> rhs);
                                    uint512_t(std::uint64_t rhs);

    uint512_t &                     operator = (uint512_t const & rhs);
    uint512_t &                     operator = (int512_t const & rhs);

    bool                            is_positive() const { return true; }
    bool                            is_negative() const { return false; }

    std::size_t                     bit_size() const;
    void                            lsl(int count);
    void                            lsr(int count);
    uint512_t &                     zero();
    bool                            is_zero() const;
    int                             compare(uint512_t const & rhs) const;
    uint512_t &                     div(uint512_t const & rhs, uint512_t & remainder);
    std::string                     to_string(int base = 10, bool introducer = false, bool uppercase = false) const;

    uint512_t                       operator ~ () const;
    uint512_t                       operator - () const;
    uint512_t                       operator + (uint512_t const & rhs) const;
    uint512_t &                     operator += (uint512_t const & rhs);
    uint512_t &                     operator += (std::uint64_t const rhs);
    uint512_t                       operator - (uint512_t const & rhs) const;
    uint512_t &                     operator -= (uint512_t const & rhs);
    uint512_t &                     operator *= (uint512_t const & rhs);
    uint512_t &                     operator *= (std::uint64_t const rhs);
    uint512_t                       operator / (uint512_t const & rhs) const;
    uint512_t &                     operator /= (uint512_t const & rhs);
    uint512_t &                     operator /= (std::uint64_t const rhs);
    uint512_t &                     operator %= (uint512_t const & rhs);
    uint512_t &                     operator <<= (int shift);
    uint512_t                       operator << (int shift) const;
    uint512_t &                     operator >>= (int shift);
    uint512_t                       operator >> (int shift) const;
    uint512_t                       operator & (uint512_t const & rhs) const;
    uint512_t                       operator & (std::uint64_t rhs) const;
    uint512_t &                     operator &= (uint512_t const & rhs);
    uint512_t                       operator | (uint512_t const & rhs) const;
    uint512_t                       operator | (std::uint64_t rhs) const;
    uint512_t &                     operator |= (uint512_t const & rhs);
    uint512_t                       operator ^ (uint512_t const & rhs) const;
    uint512_t                       operator ^ (std::uint64_t rhs) const;
    uint512_t &                     operator ^= (uint512_t const & rhs);

    bool                            operator == (uint512_t const & rhs) const;
    bool                            operator == (uint64_t rhs) const;
    bool                            operator != (uint512_t const & rhs) const;
    bool                            operator != (uint64_t rhs) const;
    bool                            operator < (uint512_t const & rhs) const;
    bool                            operator <= (uint512_t const & rhs) const;
    bool                            operator > (uint512_t const & rhs) const;
    bool                            operator >= (uint512_t const & rhs) const;

    std::uint64_t                   f_value[8] = { 0 };
};


std::string to_string(uint512_t const & v);


inline std::ostream & operator << (std::ostream & os, uint512_t v)
{
    switch(os.flags() & std::ios_base::basefield)
    {
    case std::ios_base::oct:
        return os << v.to_string(
              8
            , (os.flags() & std::ios_base::showbase) != 0);

    case std::ios_base::hex:
        return os << v.to_string(
              16
            , (os.flags() & std::ios_base::showbase) != 0
            , (os.flags() & std::ios_base::uppercase) != 0);

    default:
        if((os.flags() & std::ios_base::showpos) != 0)
        {
            os << '+';
        }
        return os << v.to_string(10);

    }
}



} // namespace prinbee
// vim: ts=4 sw=4 et
