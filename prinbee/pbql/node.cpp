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
 * The Prinbee Query Language (PBQL) is an SQL-like language. This file
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


// snapdev
//
#include    <snapdev/to_lower.h>
#include    <snapdev/to_upper.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace pbql
{



namespace
{



struct token_name_t
{
    token_t                 f_token = token_t::TOKEN_UNKNOWN;
    char                    f_name[32] = {};
};

#define TOKEN_NAME(name)    { token_t::TOKEN_##name, #name }
#define TOKEN_CHAR(name)    { token_t::TOKEN_##name, { static_cast<char>(token_t::TOKEN_##name), '\0' } }

struct token_name_t g_token_names[] =
{
    // that one doesn't work because EOF gets transformed to -1 ahead of time
    //
    //TOKEN_NAME(EOF),
    { token_t::TOKEN_EOF, "EOF" },

    TOKEN_NAME(UNKNOWN),

    TOKEN_CHAR(BITWISE_XOR),
    TOKEN_CHAR(MODULO),
    TOKEN_CHAR(BITWISE_AND),
    TOKEN_CHAR(OPEN_PARENTHESIS),
    TOKEN_CHAR(CLOSE_PARENTHESIS),
    TOKEN_CHAR(MULTIPLY),
    TOKEN_CHAR(PLUS),
    TOKEN_CHAR(COMMA),
    TOKEN_CHAR(MINUS),
    TOKEN_CHAR(PERIOD),
    TOKEN_CHAR(DIVIDE),
    TOKEN_CHAR(COLON),
    TOKEN_CHAR(SEMI_COLON),
    TOKEN_CHAR(LESS),
    TOKEN_CHAR(EQUAL),
    TOKEN_CHAR(GREATER),
    TOKEN_CHAR(ABSOLUTE_VALUE),
    TOKEN_CHAR(POWER),
    TOKEN_CHAR(BITWISE_OR),
    TOKEN_CHAR(REGULAR_EXPRESSION),

    //TOKEN_NAME(other), -- not a token

    TOKEN_NAME(IDENTIFIER),
    TOKEN_NAME(STRING),
    TOKEN_NAME(INTEGER),
    TOKEN_NAME(FLOATING_POINT),

    TOKEN_NAME(NOT_EQUAL),
    TOKEN_NAME(LESS_EQUAL),
    TOKEN_NAME(GREATER_EQUAL),
    TOKEN_NAME(SQUARE_ROOT),
    TOKEN_NAME(CUBE_ROOT),
    TOKEN_NAME(SHIFT_LEFT),
    TOKEN_NAME(SHIFT_RIGHT),
    TOKEN_NAME(STRING_CONCAT),

    //TOKEN_NAME(max) -- not a token
};



} // no name namespace



char const * to_string(token_t t)
{
    std::uint32_t i(0);
    std::uint32_t j(std::size(g_token_names));
    while(i < j)
    {
        std::uint32_t p((j - i) / 2 + i);
        int const r(static_cast<int>(g_token_names[p].f_token) - static_cast<int>(t));
        if(r < 0)
        {
            i = p + 1;
        }
        else if(r > 0)
        {
            j = p;
        }
        else
        {
            return g_token_names[p].f_name;
        }
    }

    return nullptr;
}



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
    case token_t::TOKEN_COLON:
    case token_t::TOKEN_SEMI_COLON:
    case token_t::TOKEN_EQUAL:
    case token_t::TOKEN_ABSOLUTE_VALUE:
    case token_t::TOKEN_OPEN_BRACKET:
    case token_t::TOKEN_CLOSE_BRACKET:
    case token_t::TOKEN_POWER:
    case token_t::TOKEN_BITWISE_OR:
    case token_t::TOKEN_REGULAR_EXPRESSION:
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
    case token_t::TOKEN_SCOPE:
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


std::string node::get_string_lower() const
{
    return snapdev::to_lower(f_string);
}


std::string node::get_string_upper() const
{
    return snapdev::to_upper(f_string);
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
