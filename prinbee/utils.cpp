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


/** \file
 * \brief Various utility function implementation.
 *
 * Basic utility functions used throughout the prinbee database
 * implementation.
 */

// self
//
#include    "prinbee/utils.h"


// snaplogger
//
//#include    <snaplogger/message.h>


// C++
//
//#include    <iostream>
//#include    <type_traits>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



bool validate_name(std::string const & name, size_t const max_length)
{
    if(name.empty())
    {
        return false;
    }
    if(name.length() > max_length)
    {
        return false;
    }

    char c(name[0]);
    if((c < 'a' || c > 'z')
    && (c < 'A' || c > 'Z')
    && c != '_')
    {
        return false;
    }

    auto const max(name.length());
    for(std::remove_const<decltype(max)>::type idx(1); idx < max; ++idx)
    {
        c = name[idx];
        if((c < 'a' || c > 'z')
        && (c < 'A' || c > 'Z')
        && (c < '0' || c > '9')
        && c != '_')
        {
            return false;
        }
    }

    return true;
}



} // namespace prinbee
// vim: ts=4 sw=4 et
