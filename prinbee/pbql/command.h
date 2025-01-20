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
 * \brief The parser creates a list of commands to be executed.
 *
 * This file defines the command class which the parser uses to create
 * a list of commands to be executed.
 *
 * Nothing gets executed while parsing the commands. Instead, it gets
 * added to a vector of commands. Once an entire script was parsed, then
 * you can start the execution of that script.
 */

// self
//
#include    <prinbee/bigint/int512.h>



// C++
//
#include    <map>
#include    <memory>
#include    <vector>



namespace prinbee
{
namespace pbql
{



enum class command_t
{
    COMMAND_UNKNOWN,    // an invalid command

    COMMAND_BEGIN,
    COMMAND_COMMIT,
    COMMAND_CREATE_CONTEXT,
    COMMAND_ROLLBACK,
    COMMAND_SELECT,
};


enum class param_type_t
{
    PARAM_TYPE_UNKNOWN,       // parameter is not defined

    PARAM_TYPE_BOOL,
    PARAM_TYPE_INT64,
    PARAM_TYPE_INT512,
    PARAM_TYPE_STRING,
};


constexpr int const         MAX_LIMIT       = 1'000'000;
constexpr int const         MAX_EXPRESSIONS = 1'000;
constexpr int const         MAX_TABLES      = 20;

enum class param_t
{
    PARAM_UNKNOWN,      // an invalid parameter

    PARAM_CONDITION,
    PARAM_DESCRIPTION,
    PARAM_GROUP,
    PARAM_IF_EXISTS,
    PARAM_LIMIT,
    PARAM_NAME,
    PARAM_ORDER_BY,
    PARAM_PATH,
    PARAM_TYPE,
    PARAM_USER,
    PARAM_WHERE,

    // allow for up to MAX_EXPRESSIONS expressions (for SELECT)
    //
    PARAM_EXPRESSION,
    PARAM_EXPRESSION_end = PARAM_EXPRESSION + MAX_EXPRESSIONS - 1,
    PARAM_COLUMN_NAME,
    PARAM_COLUMN_NAME_end = PARAM_COLUMN_NAME + MAX_EXPRESSIONS - 1,
    PARAM_TABLE,
    PARAM_TABLE_end = PARAM_TABLE + MAX_TABLES - 1,
    PARAM_TABLE_NAME,
    PARAM_TABLE_NAME_end = PARAM_TABLE_NAME + MAX_TABLES - 1,
};


class command
{
public:
    typedef std::shared_ptr<command>    pointer_t;
    typedef std::vector<pointer_t>      vector_t;

                                        command(command_t cmd);

    command_t                           get_command() const;
    param_type_t                        is_defined_as(param_t param) const;
    bool                                get_bool(param_t param) const;
    void                                set_bool(param_t param, bool value);
    std::int64_t                        get_int64(param_t param) const;
    void                                set_int64(param_t param, std::int64_t value);
    int512_t                            get_int512(param_t param) const;
    void                                set_int512(param_t param, int512_t value);
    std::string                         get_string(param_t param) const;
    void                                set_string(param_t param, std::string value);

private:
    typedef std::map<param_t, bool>             map_bool_t;
    typedef std::map<param_t, std::int64_t>     map_int64_t;
    typedef std::map<param_t, int512_t>         map_int512_t;
    typedef std::map<param_t, std::string>      map_string_t;

    command_t const                     f_command = command_t::COMMAND_UNKNOWN;
    map_bool_t                          f_bool_params = map_bool_t();
    map_int64_t                         f_int64_params = map_int64_t();
    map_int512_t                        f_int512_params = map_int512_t();
    map_string_t                        f_string_params = map_string_t();
};



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
