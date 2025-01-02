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
 * \brief Parser of the Prinbee Query Language.
 *
 * The Pribee Query Language (PBQL) is an SQL-like language. This file
 * defines the classes supported by the parser.
 */

// self
//
#include    <prinbee/pbql/lexer.h>
#include    <prinbee/pbql/command.h>


// advgetopt
//
#include    <advgetopt/utils.h>



namespace prinbee
{
namespace pbql
{



enum class transaction_t : std::uint8_t
{
    TRANSACTION_UNDEFINED,
    TRANSACTION_SCHEMA,
    TRANSACTION_DATA,
};


class parser
{
public:
    typedef std::shared_ptr<parser>     pointer_t;

                        parser(lexer::pointer_t l);

    command::vector_t const &
                        parse();

private:
    void                parse_alter_index();
    void                parse_alter_table();
    void                parse_alter_type();

    void                parse_begin();
    void                parse_create_context();
    void                parse_create_index();
    void                parse_create_table();
    void                parse_create_type();

    void                expect_semi_colon(
                              std::string const & command
                            , node::pointer_t n = node::pointer_t());
    node::pointer_t     keyword_string(
                              std::string commands
                            , advgetopt::string_list_t const & keywords
                            , bool & optional_found
                            , token_t next_token_type = token_t::TOKEN_UNKNOWN);
    void                parse_transaction_command(std::string const & cmd_name, command_t cmd);

    std::string         parse_expression(node::pointer_t n);

    lexer::pointer_t    f_lexer = lexer::pointer_t();
    command::vector_t   f_commands = command::vector_t();
};



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
