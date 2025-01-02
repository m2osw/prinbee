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
 * \brief A node represents a token of the Prinbee Query Language.
 *
 * This file implements the node which holds a token and its value when
 * necessary (i.e. identifier name, string, integer, floating point, etc.)
 * Nodes can be organized in a tree, useful for expression execution.
 */

// self
//
#include    <prinbee/pbql/location.h>
#include    <prinbee/bigint/uint512.h>



// C++
//
#include    <memory>
#include    <vector>



namespace prinbee
{
namespace pbql
{



enum class token_t
{
    TOKEN_EOF = -1,
    TOKEN_UNKNOWN,

    TOKEN_BITWISE_XOR        = '#',
    TOKEN_MODULO             = '%',
    TOKEN_BITWISE_AND        = '&',
    TOKEN_OPEN_PARENTHESIS   = '(',
    TOKEN_CLOSE_PARENTHESIS  = ')',
    TOKEN_MULTIPLY           = '*',
    TOKEN_PLUS               = '+',
    TOKEN_COMMA              = ',',
    TOKEN_MINUS              = '-',
    TOKEN_PERIOD             = '.',
    TOKEN_DIVIDE             = '/',
    TOKEN_COLON              = ':',
    TOKEN_SEMI_COLON         = ';',
    TOKEN_LESS               = '<',
    TOKEN_EQUAL              = '=',
    TOKEN_GREATER            = '>',
    TOKEN_ABSOLUTE_VALUE     = '@',
    TOKEN_OPEN_BRACKET       = '[',
    TOKEN_CLOSE_BRACKET      = ']',
    TOKEN_POWER              = '^', // exponential
    TOKEN_BITWISE_OR         = '|',
    TOKEN_REGULAR_EXPRESSION = '~',

    TOKEN_other = 1000,

    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_FLOATING_POINT,

    TOKEN_NOT_EQUAL,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_EQUAL,
    TOKEN_SQUARE_ROOT,
    TOKEN_CUBE_ROOT,
    TOKEN_SCOPE,
    TOKEN_SHIFT_LEFT,
    TOKEN_SHIFT_RIGHT,
    TOKEN_STRING_CONCAT,

    TOKEN_max // create all tokens before this one
};


char const *        to_string(token_t t);


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
class node
    : public std::enable_shared_from_this<node>
{
public:
    typedef std::shared_ptr<node>   pointer_t;
    typedef std::weak_ptr<node>     weak_ptr_t;
    typedef std::vector<pointer_t>  vector_t;

                        node(token_t token, location const & l);

    token_t             get_token() const;
    location const &    get_location() const;

    void                set_string(std::string const & string); // for STRING and IDENTIFIER
    std::string const & get_string() const;
    std::string         get_string_lower() const;
    std::string         get_string_upper() const;
    void                set_integer(uint512_t i);
    uint512_t           get_integer();
    void                set_floating_point(long double d);
    long double         get_floating_point();

    pointer_t           get_parent() const;
    pointer_t           get_child(int position) const;
    std::size_t         get_children_size() const;
    void                insert_child(int position, pointer_t child);

private:
    token_t             f_token = token_t::TOKEN_UNKNOWN;
    location            f_location = location();

    std::string         f_string = std::string();
    uint512_t           f_integer = uint512_t();
    long double         f_floating_point = 0.0;

    weak_ptr_t          f_parent = weak_ptr_t();
    vector_t            f_children = vector_t();
};
#pragma GCC diagnostic pop



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
