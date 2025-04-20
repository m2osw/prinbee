// Copyright (c) 2024-2025  Made to Order Software Corp.  All Rights Reserved
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



enum class type_t
{
    TYPE_BOOLEAN,
    TYPE_INT1,
    TYPE_INT2,
    TYPE_INT4,
    TYPE_INT8,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UNSIGNED_INT1,
    TYPE_UNSIGNED_INT2,
    TYPE_UNSIGNED_INT4,
    TYPE_UNSIGNED_INT8,
    TYPE_UNSIGNED_INT16,
    TYPE_UNSIGNED_INT32,
    TYPE_UNSIGNED_INT64,
    TYPE_FLOAT4,
    TYPE_FLOAT8,
    TYPE_FLOAT10,
    TYPE_TEXT,
};


char const * cast_type_to_as2js_type(type_t const type);


enum class token_t
{
    TOKEN_EOF = -1,
    TOKEN_UNKNOWN,

    // lexer one character token
    //
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

    // lexer multi-character tokens
    //
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
    TOKEN_UNMATCHED_REGULAR_EXPRESSION,
    TOKEN_STRING_CONCAT,

    // parser additional tokens (glue in our tree)
    //
    TOKEN_ALL_FIELDS,
    TOKEN_AT,
    TOKEN_BETWEEN,
    TOKEN_CAST,
    TOKEN_FALSE,
    TOKEN_FUNCTION_CALL, // get_string() to retrieve the name
    TOKEN_ILIKE,
    TOKEN_LIKE,
    TOKEN_LIST,
    TOKEN_LOGICAL_OR,
    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_NOT,
    TOKEN_NULL,
    TOKEN_TRUE,
    TOKEN_TYPE, // basic types (INT, REAL, TEXT...)

    // meta types -- used by is_literal()
    TOKEN_BOOLEAN,      // true or false
    TOKEN_NUMBER,       // integer or floating point

    TOKEN_max // create all tokens before this one
};


std::string             to_string(token_t t, bool quote_char = true);


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
    bool                is_literal(token_t match_type = token_t::TOKEN_UNKNOWN) const;

    void                set_string(std::string const & string); // for STRING and IDENTIFIER
    std::string const & get_string() const;
    std::string         get_string_lower() const;
    std::string         get_string_upper() const;
    std::string         get_string_auto_convert() const;
    bool                get_boolean_auto_convert() const;
    int512_t            get_integer_auto_convert() const;
    long double         get_floating_point_auto_convert() const;
    void                set_integer(int512_t i);
    int512_t            get_integer();
    void                set_floating_point(long double d);
    long double         get_floating_point();

    pointer_t           get_parent() const;
    pointer_t           get_child(int position) const;
    std::size_t         get_children_size() const;
    void                set_child(int position, pointer_t child);
    void                insert_child(int position, pointer_t child);

    std::string         to_as2js() const;
    std::string         convert_like_pattern(std::string const sql_pattern) const;
    std::string         to_tree(int indent = 0) const;

private:
    std::string         recursive_to_as2js() const;

    token_t             f_token = token_t::TOKEN_UNKNOWN;
    location            f_location = location();

    std::string         f_string = std::string();
    int512_t            f_integer = int512_t();
    long double         f_floating_point = 0.0;

    weak_ptr_t          f_parent = weak_ptr_t();
    vector_t            f_children = vector_t();

    mutable int         f_temp_counter = 0;
};
#pragma GCC diagnostic pop



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
