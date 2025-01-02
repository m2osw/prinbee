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
 * \brief Lexer of the Prinbee Query Language.
 *
 * The Pribee Query Language (PBQL) is an SQL-like language. This file
 * defines the classes supported by the lexer.
 */

// self
//
#include    <prinbee/pbql/input.h>
#include    <prinbee/pbql/node.h>



namespace prinbee
{
namespace pbql
{



class lexer
{
public:
    typedef std::shared_ptr<lexer>  pointer_t;

    void                set_input(input::pointer_t in);

    node::pointer_t     get_next_token();

private:
    input::pointer_t    f_input = input::pointer_t();
};



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
