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

// self
//
#include    <prinbee/pbql/location.h>


// libutf8
//
#include    <libutf8/iterator.h>


// C++
//
#include    <memory>



namespace prinbee
{
namespace pbql
{



class input
{
public:
    typedef std::shared_ptr<input>      pointer_t;

                            input(std::string const & script, std::string const & filename);

    char32_t                getc();
    void                    ungetc(char32_t c);

    location const &        get_location() const;

private:
    std::string const       f_script = std::string();
    libutf8::utf8_iterator  f_input;
    location                f_location = location();
    std::size_t             f_ungetc_pos = 0;
    char32_t                f_ungetc[3];
};


input::pointer_t            create_input(std::string const & filename);



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
