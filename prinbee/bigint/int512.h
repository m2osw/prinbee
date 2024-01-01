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
#include    <ostream>
#include    <string>



namespace prinbee
{



struct uint512_t;

struct int512_t
{
                                    int512_t();
                                    int512_t(int512_t const & rhs);
                                    int512_t(uint512_t const & rhs);
                                    int512_t(std::initializer_list<std::uint64_t> rhs);
                                    int512_t(std::int64_t rhs);

    int512_t &                      operator = (int512_t const & rhs) = default;

    bool                            is_positive() const { return f_high_value >= 0; }
    bool                            is_negative() const { return f_high_value < 0; }
    int512_t                        abs() const { if(f_high_value < 0) return -*this; else return *this; }
    int512_t &                      zero();
    bool                            is_zero() const;
    int                             compare(int512_t const & rhs) const;

    std::size_t                     bit_size() const;
    std::string                     to_string(int base = 10, bool introducer = false, bool uppercase = false) const;

    int512_t                        operator - () const;
    int512_t &                      operator += (int512_t const & rhs);
    int512_t &                      operator += (int64_t rhs);
    int512_t &                      operator -= (int512_t const & rhs);
    int512_t &                      operator -= (int64_t rhs);

    int512_t &                      operator ++ ();
    int512_t &                      operator -- ();

    bool                            operator == (int512_t const & rhs) const;
    bool                            operator == (int64_t rhs) const;
    bool                            operator != (int512_t const & rhs) const;
    bool                            operator != (int64_t rhs) const;
    bool                            operator < (int512_t const & rhs) const;
    bool                            operator < (std::int64_t rhs) const;
    bool                            operator <= (int512_t const & rhs) const;
    bool                            operator > (int512_t const & rhs) const;
    bool                            operator >= (int512_t const & rhs) const;

    std::uint64_t                   f_value[7] = { 0 };
    std::int64_t                    f_high_value = 0;
};


extern std::string to_string(int512_t const & v);


inline std::ostream & operator << (std::ostream & os, int512_t v)
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
        if(v.is_positive()
        && (os.flags() & std::ios_base::showpos) != 0)
        {
            os << '+';
        }
        return os << v.to_string(10);

    }
}



} // namespace prinbee
// vim: ts=4 sw=4 et
