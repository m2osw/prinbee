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


/** \file
 * \brief Location in the input.
 *
 * This object manages the current location in the input file or string.
 * (The CLI uses a string.)
 *
 * The object generates a string with the location which is often used
 * to display an error message if necessary. The location can also be
 * copied in each node so if the error is found at a later time (in the
 * parser or when executing the results) then it can be used with the
 * correct information.
 */

// self
//
#include    "prinbee/pbql/location.h"



// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace pbql
{



void location::set_filename(std::string const & filename)
{
    f_filename = filename;
}


std::string const & location::get_filename() const
{
    return f_filename;
}


void location::next_column()
{
    ++f_column;
}


void location::next_line()
{
    ++f_line;
    f_column = 1;
}


int location::get_column() const
{
    return f_column;
}


int location::get_line() const
{
    return f_line;
}


std::string location::get_location() const
{
    std::string result(f_filename);

    if(!result.empty())
    {
        result += ':';
    }
    result += std::to_string(f_line);
    result += ':';
    result += std::to_string(f_column);
    result += ": ";

    return result;
} // LCOV_EXCL_LINE



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
