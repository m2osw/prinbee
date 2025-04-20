// Copyright (c) 2019-2025  Made to Order Software Corp.  All Rights Reserved
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

#include    "prinbee/exception.h"


// snapdev
//
#include    <snapdev/pathinfo.h>


// C
//
#include    <sys/stat.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace
{



constexpr char const * const    g_default_prinbee_path = "/var/lib/prinbee";
std::string                     g_prinbee_path = g_default_prinbee_path;
constexpr char const * const    g_prinbee_name = "prinbee";



} // no name namespace



char const * get_default_prinbee_path()
{
    return g_default_prinbee_path;
}


void set_prinbee_path(std::string const & path)
{
    if(path.empty())
    {
        throw file_not_found("the top prinbee data path cannot be set to the empty string.");
    }
    if(!snapdev::pathinfo::is_absolute(path))
    {
        throw file_not_found(
              "the top prinbee data path must be an absolute path, \""
            + path
            + "\" is not considered valid.");
    }
    struct stat s;
    if(stat(path.c_str(), &s) != 0)
    {
        throw file_not_found(
              "the top prinbee data path must exist, directory \""
            + path
            + "\" not found.");
    }
    if(!S_ISDIR(s.st_mode))
    {
        throw invalid_type(
              "the top prinbee data path \""
            + path
            + "\" must be a directory.");
    }
    std::string errmsg;
    std::string const real_path(snapdev::pathinfo::realpath(path, errmsg));
    if(!errmsg.empty())
    {
        throw file_not_found(
              "could not retrieve the real prinbee data path for \""
            + path
            + "\" (error: "
            + errmsg
            + ").");
    }
    g_prinbee_path = real_path;
}


std::string const & get_prinbee_path()
{
    return g_prinbee_path;
}


char const * get_prinbee_group()
{
    return g_prinbee_name;
}


char const * get_prinbee_user()
{
    return g_prinbee_name;
}



} // namespace prinbee
// vim: ts=4 sw=4 et
