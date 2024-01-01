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


/** \file
 * \brief Database file implementation.
 *
 * Each table uses one or more files. Each file is handled by a dbfile
 * object and a corresponding set of blocks.
 */

// self
//
#include    "prinbee/error.h"


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



error::error(
          error_code_t code
        , std::string const & message)
    : f_error_code(code)
    , f_message(message)
{
}


error_code_t error::get_error_code() const
{
    return f_error_code;
}


std::string error::get_error_message() const
{
    return f_message;
}




} // namespace prinbee
// vim: ts=4 sw=4 et
