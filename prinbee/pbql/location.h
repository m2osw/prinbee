// Copyright (c) 2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Location of token in input.
 *
 * The Pribee Query Language (PBQL) reads a file or a string and saves the
 * current location in this object.
 */

// C++
//
#include    <string>



namespace prinbee
{
namespace pbql
{



constexpr char32_t const    END_OF_INPUT = -1;


class location
{
public:
    void                set_filename(std::string const & filename);
    std::string const & get_filename() const;

    void                next_column();
    void                next_line();

    int                 get_column() const;
    int                 get_line() const;

    std::string         get_location() const;

private:
    std::string         f_filename = std::string();
    std::size_t         f_ungetc_pos = 0;
    char32_t            f_ungetc[3];
    int                 f_line = 1;
    int                 f_column = 1;
};



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
