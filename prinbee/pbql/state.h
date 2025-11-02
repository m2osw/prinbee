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
 * \brief The state manages contexts in the Prinbee language.
 *
 * This file defines the state class which the parser uses to keep track
 * of the contexts, tables, user types, etc. that the language manages.
 *
 * The state offers callbacks so when an object is still unknown, its
 * owner has a chance to provide the object if available (i.e. in the
 * client/server environment, the client can ask the server about a
 * context, a table inside a context, etc.)
 */

// self
//
//#include    <prinbee/pbql/context.h>
//#include    <prinbee/bigint/uint512.h>



// C++
//
#include    <memory>
#include    <vector>



namespace prinbee
{
namespace pbql
{



class context;


class state_callback
{
public:
    typedef std::shared_ptr<state_callback>    pointer_t;

    virtual             ~state_callback() {}

    virtual bool        get_context(std::string const & name, std::shared_ptr<context> & result) = 0;
};



class state
{
public:
    typedef std::shared_ptr<state>  pointer_t;
    typedef snapdev::callback_manager<state_callbacks::pointer_t>
                                    callback_t;

    callback_t::callback_id_t   add_callback(
                                      state_callback::pointer_t callback
                                    , callback_t::priority_t priority = callback_t::DEFAULT_PRIORITY);

    std::shared_ptr<context>    get_context(std::string const & name) const;

private:
    snapdev::callback_manager<state_callback>           f_callbacks = snapdev::callback_manager<state_callback>();

    std::map<std::string, std::shared_ptr<context>>     f_contexts = std::map<std::string, std::shared_ptr<context>>();
};



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
