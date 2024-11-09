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
#include    "prinbee/pbql/node.h"

#include    "prinbee/exception.h"


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
    switch(f_token)
    {
    case token_t::TOKEN_EOF:
    case token_t::TOKEN_BITWISE_XOR:
    case token_t::TOKEN_MODULO:
    case token_t::TOKEN_BITWISE_AND:
    case token_t::TOKEN_OPEN_PARENTHESIS:
    case token_t::TOKEN_CLOSE_PARENTHESIS:
    case token_t::TOKEN_MULTIPLY:
    case token_t::TOKEN_PLUS:
    case token_t::TOKEN_COMMA:
    case token_t::TOKEN_MINUS:
    case token_t::TOKEN_PERIOD:
    case token_t::TOKEN_DIVIDE:
    case token_t::TOKEN_SEMI_COLON:
    case token_t::TOKEN_EQUAL:
    case token_t::TOKEN_ABSOLUTE_VALUE:
    case token_t::TOKEN_POWER:
    case token_t::TOKEN_BITWISE_OR:
    case token_t::TOKEN_BITWISE_NOT:
    case token_t::TOKEN_IDENTIFIER:
    case token_t::TOKEN_STRING:
    case token_t::TOKEN_INTEGER:
    case token_t::TOKEN_FLOATING_POINT:
    case token_t::TOKEN_NOT_EQUAL:
    case token_t::TOKEN_LESS:
    case token_t::TOKEN_LESS_EQUAL:
    case token_t::TOKEN_GREATER:
    case token_t::TOKEN_GREATER_EQUAL:
    case token_t::TOKEN_SQUARE_ROOT:
    case token_t::TOKEN_CUBE_ROOT:
    case token_t::TOKEN_SHIFT_LEFT:
    case token_t::TOKEN_SHIFT_RIGHT:
    case token_t::TOKEN_STRING_CONCAT:
        break;

    default:
        throw invalid_token(
              "node created with an invalid token ("
            + std::to_string(static_cast<int>(f_token))
            + ").");

    }
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
    return f_parent.lock();
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
        child->f_parent = shared_from_this();
        f_children.insert(f_children.end(), child);
        return;
    }

    // make sure it's not out of bounds
    //
    if(static_cast<std::size_t>(position) >= f_children.size())
    {
        throw out_of_range("child " + std::to_string(position) + " does not exist.");
    }

    child->f_parent = shared_from_this();
    f_children.insert(std::next(f_children.begin(), position), child);
}



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
