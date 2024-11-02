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

    TOKEN_BITWISE_XOR       = '#',
    TOKEN_MODULO            = '%',
    TOKEN_BITWISE_AND       = '&',
    TOKEN_OPEN_PARENTHESIS  = '(',
    TOKEN_CLOSE_PARENTHESIS = ')',
    TOKEN_MULTIPLY          = '*',
    TOKEN_PLUS              = '+',
    TOKEN_COMMA             = ',',
    TOKEN_MINUS             = '-',
    TOKEN_DIVIDE            = '/',
    TOKEN_SEMI_COLON        = ';',
    TOKEN_EQUAL             = '=',
    TOKEN_ABSOLUTE_VALUE    = '@',
    TOKEN_POWER             = '^',
    TOKEN_BITWISE_OR        = '|',
    TOKEN_BITWISE_NOT       = '~',

    TOKEN_other = 1000,

    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_FLOATING_POINT,

    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_SQUARE_ROOT,
    TOKEN_CUBE_ROOT,
    TOKEN_SHIFT_LEFT,
    TOKEN_SHIFT_RIGHT,
    TOKEN_STRING_CONCAT,

//    TOKEN_ALTER,
//    TOKEN_AND,
//    TOKEN_AS,
//    TOKEN_ASC,
//    TOKEN_BEGIN,
//    TOKEN_BY,
//    TOKEN_CHECK,
//    //TOKEN_COMMENT,
//    TOKEN_COMMIT,
//    TOKEN_CONFIG,
//    TOKEN_CREATE,
//    TOKEN_DEFAULT,
//    TOKEN_DELETE,
//    TOKEN_DESC,
//    TOKEN_DISTINCT,
//    TOKEN_DROP,
//    TOKEN_ENUM,
//    TOKEN_EXISTS,
//    TOKEN_FIRST,
//    TOKEN_FROM,
//    TOKEN_HIDDEN,
//    TOKEN_IF,
//    TOKEN_INDEX,
//    TOKEN_INSERT,
//    TOKEN_IN,
//    TOKEN_INTO,
//    TOKEN_KEY,
//    TOKEN_LAST,
//    TOKEN_LIMIT,
//    TOKEN_LOCK,
//    TOKEN_LOGGED,
//    TOKEN_MODE,
//    TOKEN_NOT,
//    TOKEN_NULL,
//    TOKEN_NULLS,
//    TOKEN_MAXIMUM,
//    TOKEN_OR,
//    TOKEN_ORDER,
//    TOKEN_OVERWRITTEN,
//    TOKEN_PRECISION,
//    TOKEN_PRIMARY,
//    TOKEN_ROLLBACK,
//    TOKEN_SECURE,
//    TOKEN_SELECT,
//    TOKEN_SET,
//    TOKEN_SIZE,
//    TOKEN_SPARSE,
//    TOKEN_TABLE,
//    TOKEN_TEMPORARY,
//    TOKEN_TRANSACTION,
//    TOKEN_TYPE,
//    TOKEN_UNIQUE,
//    TOKEN_UNLOGGED,
//    TOKEN_UPDATE,
//    TOKEN_USING,
//    TOKEN_VALUES,
//    TOKEN_VERSIONED,
//    TOKEN_VISIBLE,
//    TOKEN_WHERE,
//    TOKEN_WITH,
//    TOKEN_WITHOUT,
//    TOKEN_WORK,

    // types are identifiers, not keywords; also some system types make use
    // of multiple identifiers: TIMESTAMP WITH TIMEZONE, UNSIGNED INT, ...
    //
    //TOKEN_BIGINT,
    //TOKEN_BOOLEAN,
    //TOKEN_BYTEA,
    //TOKEN_CHAR,
    //TOKEN_DOUBLE,
    //TOKEN_EMAIL,
    //TOKEN_FLOAT2,
    //TOKEN_FLOAT4,
    //TOKEN_FLOAT8,
    //TOKEN_FLOAT10,
    //TOKEN_INET,
    //TOKEN_INT,
    //TOKEN_INT1,
    //TOKEN_INT16,
    //TOKEN_INT2,
    //TOKEN_INT32,
    //TOKEN_INT4,
    //TOKEN_INT64,
    //TOKEN_INT8,
    //TOKEN_INTEGER,
    //TOKEN_REAL,
    //TOKEN_SMALINT,
    //TOKEN_TEXT,
    //TOKEN_TIMESTAMP,
    //TOKEN_TIMEZONE,
    //TOKEN_UNSIGNED,
    //TOKEN_UUID,
};


class node
{
public:
    typedef std::shared_ptr<node>   pointer_t;
    typedef std::vector<pointer_t>  vector_t;

                        node(token_t token, location const & l);

    token_t             get_token() const;
    location const &    get_location() const;

    void                set_string(std::string const & string); // for STRING and IDENTIFIER
    std::string const & get_string() const;
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

    pointer_t           f_parent = pointer_t();
    vector_t            f_children = vector_t();
};



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
