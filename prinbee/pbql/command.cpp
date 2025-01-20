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
 * \brief Describe and manage a Prinbee PSQL command.
 *
 * The Prinbee Query Language (PBQL) is parsed into commands that then get
 * executed. This file implements that commands with all of their
 * parameters.
 */

// self
//
#include    "prinbee/pbql/command.h"

//#include    "prinbee/pbql/context.h"
#include    "prinbee/exception.h"


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace pbql
{



command::command(command_t cmd)
    : f_command(cmd)
{
}


command_t command::get_command() const
{
    return f_command;
}


/** \brief Check whether \p param is currently defined.
 *
 * This function searches for \p param in all the lists of parameters. If
 * defined, then the function returns its type.
 *
 * The function returns param_type_t::PARAM_TYPE_UNKNOWN if the parameter
 * is not found.
 *
 * \param[in] param  The parameter to search through the list of parameters.
 *
 * \return The parameter type if found, PARAM_TYPE_UNKNOWN otherwise.
 */
param_type_t command::is_defined_as(param_t param) const
{
    auto const bool_it(f_bool_params.find(param));
    if(bool_it != f_bool_params.end())
    {
        return param_type_t::PARAM_TYPE_BOOL;
    }

    auto const int64_it(f_int64_params.find(param));
    if(int64_it != f_int64_params.end())
    {
        return param_type_t::PARAM_TYPE_INT64;
    }

    auto const int512_it(f_int512_params.find(param));
    if(int512_it != f_int512_params.end())
    {
        return param_type_t::PARAM_TYPE_INT512;
    }

    auto const string_it(f_string_params.find(param));
    if(string_it != f_string_params.end())
    {
        return param_type_t::PARAM_TYPE_STRING;
    }

    return param_type_t::PARAM_TYPE_UNKNOWN;
}


bool command::get_bool(param_t param) const
{
    auto const it(f_bool_params.find(param));
    if(it == f_bool_params.end())
    {
        return 0;
    }

    return it->second;
}


void command::set_bool(param_t param, bool value)
{
    param_type_t current_type(is_defined_as(param));
    if(current_type != param_type_t::PARAM_TYPE_UNKNOWN
    && current_type != param_type_t::PARAM_TYPE_BOOL)
    {
        throw type_mismatch(
              "parameter "
            + std::to_string(static_cast<int>(param))
            + " already different as a different type; expected an bool.");
    }

    f_bool_params[param] = value;
}


std::int64_t command::get_int64(param_t param) const
{
    auto const it(f_int64_params.find(param));
    if(it == f_int64_params.end())
    {
        return 0;
    }

    return it->second;
}


void command::set_int64(param_t param, std::int64_t value)
{
    param_type_t current_type(is_defined_as(param));
    if(current_type != param_type_t::PARAM_TYPE_UNKNOWN
    && current_type != param_type_t::PARAM_TYPE_INT64)
    {
        throw type_mismatch(
              "parameter "
            + std::to_string(static_cast<int>(param))
            + " already different as a different type; expected an int64.");
    }

    f_int64_params[param] = value;
}


int512_t command::get_int512(param_t param) const
{
    auto const it(f_int512_params.find(param));
    if(it == f_int512_params.end())
    {
        return 0;
    }

    return it->second;
}


void command::set_int512(param_t param, int512_t value)
{
    param_type_t current_type(is_defined_as(param));
    if(current_type != param_type_t::PARAM_TYPE_UNKNOWN
    && current_type != param_type_t::PARAM_TYPE_INT512)
    {
        throw type_mismatch(
              "parameter "
            + std::to_string(static_cast<int>(param))
            + " already different as a different type; expected an int512.");
    }

    f_int512_params[param] = value;
}


std::string command::get_string(param_t param) const
{
    auto const it(f_string_params.find(param));
    if(it == f_string_params.end())
    {
        return std::string();
    }

    return it->second;
}


void command::set_string(param_t param, std::string value)
{
    param_type_t current_type(is_defined_as(param));
    if(current_type != param_type_t::PARAM_TYPE_UNKNOWN
    && current_type != param_type_t::PARAM_TYPE_STRING)
    {
        throw type_mismatch(
              "parameter "
            + std::to_string(static_cast<int>(param))
            + " already different as a different type; expected a string.");
    }

    f_string_params[param] = value;
}



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
