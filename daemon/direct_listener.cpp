// Copyright (c) 2016-2025  Made to Order Software Corp.  All Rights Reserved
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


// self
//
#include    "direct_listener.h"

#include    "prinbeed.h"


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_daemon
{



/** \class direct_listener
 * \brief Handle direct connections with clients.
 *
 * The prinbee environment lets clients connect directly as well. This
 * is mainly used by admins and debugging to avoid using a proxy.
 */



/** \brief The direct client listener initialization.
 *
 * This function initializes the direct client listener.
 *
 * \param[in] p  The prinbee server we are listening for.
 * \param[in] a  The address to listen on.
 */
direct_listener::direct_listener(prinbeed * p, addr::addr const & a)
    : binary_server(a)
    , f_prinbeed(p)
{
}


direct_listener::~direct_listener()
{
}


/** \brief Call the stop function of the prinbeed object.
 *
 * When this function is called, the signal was received and thus we are
 * asked to quit as soon as possible.
 *
 * \param[in] client  The binary server client object that just connected
 * to us.
 */
void direct_listener::process_new_connection(prinbee::binary_server_client::pointer_t client)
{
    // call the base class version
    //
    binary_server::process_new_connection(client);

    client->add_message_callback(
          prinbee::g_message_error
        , std::bind(&prinbeed::msg_err, f_prinbeed, client, std::placeholders::_1));
    client->add_message_callback(
          prinbee::g_message_ping
        , std::bind(&prinbeed::msg_ping, f_prinbeed, client, std::placeholders::_1));
    //client->add_message_callback(
    //      prinbee::g_message_pong
    //    , std::bind(&prinbeed::msg_pong, f_prinbeed, client, std::placeholders::_1));
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
