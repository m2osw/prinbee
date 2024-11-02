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
 * \brief Lexer of the Prinbee Query Language.
 *
 * The Pribee Query Language (PBQL) is an SQL-like language. This file
 * transforms the input data in tokens that the parser can then use to
 * create statements.
 *
 * The lexer supports tokens that include keywords (SELECT), identifiers
 * (column name), numbers (integers, floating points), operators (for
 * expressions; +, -, *, /, etc.).
 */

// self
//
#include    "prinbee/pbql/lexer.h"

#include    "prinbee/exception.h"


//// advgetopt
////
//#include    <advgetopt/conf_file.h>
//#include    <advgetopt/validator_integer.h>
//
//
//// snaplogger
////
//#include    <snaplogger/message.h>
//
//
//// snapdev
////
//#include    <snapdev/hexadecimal_string.h>
//#include    <snapdev/mkdir_p.h>
//#include    <snapdev/pathinfo.h>
//#include    <snapdev/stream_fd.h>
//#include    <snapdev/unique_number.h>
//
//
//// C
////
//#include    <linux/fs.h>
//#include    <sys/ioctl.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace pbql
{



node::node(token_t token, location const & l)
    : f_token(token)
    , f_location(l)
{
}


token_t node::get_token() const
{
    return f_token;
}


location const & node::get_location() const
{
    return f_location;
}


void node::set_string(std::string const & s)
{
    f_string = s;
}


std::string const & node::get_string() const
{
    return f_string;
}


void node::set_integer(uint512_t i)
{
    f_integer = i;
}


uint512_t node::get_integer()
{
    return f_integer;
}


void node::set_floating_point(long double d)
{
    f_floating_point = d;
}


long double node::get_floating_point()
{
    return f_floating_point;
}


node::pointer_t node::get_parent() const
{
    return f_parent;
}


node::pointer_t node::get_child(int position) const
{
    if(static_cast<std::size_t>(position) >= f_children.size())
    {
        throw out_of_range("child " + std::to_string(position) + " does not exist.");
    }

    return f_children[position];
}


std::size_t node::get_children_size() const
{
    return f_children.size();
}


void node::insert_child(int position, pointer_t child)
{
    // if position is -1, do an append
    //
    if(static_cast<std::size_t>(position) == static_cast<std::size_t>(-1))
    {
        position = f_children.size();
    }

    // if end of list, just do an append
    //
    if(static_cast<std::size_t>(position) == f_children.size())
    {
        f_children.insert(f_children.end(), child);
        return;
    }

    // make sure it's not out of bounds
    //
    if(static_cast<std::size_t>(position) >= f_children.size())
    {
        throw out_of_range("child " + std::to_string(position) + " does not exist.");
    }

    f_children.insert(std::next(f_children.begin(), position), child);
}



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
