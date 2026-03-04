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
 * \brief A session is used by a client to manage a Prinbee database.
 *
 * This file defines the session class used by clients to manage the
 * communication between the client and a Prinbee server. This is
 * automatically created when you create your messenger derived from
 * the prinbee_connection object, which also automatically creates a
 * proxy_connection.
 *
 * The session tracks the messages and their replies. Especially, it
 * requests the list of contexts and whenever one of these contexts
 * is made current (`{CONNECT | USE} <context-name>`), its identifier
 * is saved in the session.
 *
 * When you connect to a context, the remainder of the schema of that
 * context is sent to the client. So all the details about the
 * context plus tables, user types, and indexes. This way, it is possible
 * to access the data but especially to verify many things on the client
 * side before sending further messages to Prinbee.
 *
 * The session manages most of the objects available. A few may need to
 * be supplied by the client (TBD). Also when there is a change in the
 * schema, the system sends the info to you, the client, through a
 * callback. Any one of the objects already mentioned may trigger an
 * event: context, table, user types, or indexes.
 *
 * The session is also able to execute PBQL commands. So if you prefer
 * to use an SQL like command, you can do that. Just know that creating
 * a string to parse it to create a binary message is rather counter
 * intuitive and a waste of time and resources since you could create
 * the binary message directly.
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



class context;


// TODO: I don't think we need that since the session will receive the
//       info from the proxy_connection directly
//class state_callback
//{
//public:
//    typedef std::shared_ptr<state_callback>    pointer_t;
//
//    virtual             ~state_callback() {}
//
//    virtual bool        get_context(std::string const & name, std::shared_ptr<context> & result) = 0;
//};



class session
{
public:
    typedef std::shared_ptr<session>  pointer_t;
    //typedef snapdev::callback_manager<state_callbacks::pointer_t>
    //                                callback_t;

    //callback_t::callback_id_t   add_callback(
    //                                  state_callback::pointer_t callback
    //                                , callback_t::priority_t priority = callback_t::DEFAULT_PRIORITY);

    std::shared_ptr<context>    get_context(std::string const & name) const;

private:
    //snapdev::callback_manager<state_callback>           f_callbacks = snapdev::callback_manager<state_callback>();

    // we may want context by ID as well as name
    std::map<std::string, std::shared_ptr<context>>     f_contexts = std::map<std::string, std::shared_ptr<context>>();
};



} // namespace prinbee
// vim: ts=4 sw=4 et
