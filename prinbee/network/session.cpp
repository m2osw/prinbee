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


/** \file
 * \brief State of the Prinbee Query Language.
 *
 * The Prinbee Query Language (PBQL) uses the state to keep track of the
 * contexts it is working with.
 */

// self
//
#include    "prinbee/pbql/state.h"

#include    "prinbee/pbql/context.h"
#include    "prinbee/exception.h"


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace pbql
{



state::callback_t::callback_id_t state::add_callback(
      state_callback::pointer_t callback
    , callback_t::priority_t priority)
{
    return f_callbacks.add_callback(callback, priority);
}


context::pointer_t state::get_context(std::string const & name) const
{
    std::shared_ptr<context> result;
    snapdev::NOT_USED(f_callbacks.call(&state_callbacks::get_context, name, std::ref(result)));

    return result;
}



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
