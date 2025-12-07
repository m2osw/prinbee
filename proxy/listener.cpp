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
#include    "listener.h"



// prinbee
//
#include    <prinbee/network/binary_message.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_proxy
{



/** \class direct_listener
 * \brief Handle direct connections with clients.
 *
 * The prinbee environment lets clients connect directly as well. This
 * is mainly used by admins and debugging avoiding potential issues in
 * the proxy implementation.
 */



/** \brief The direct client listener initialization.
 *
 * This function initializes the direct client listener. It allows for
 * clients such as pbql to directly connect to the daemon.
 *
 * The proxy and daemon node connection end points should not be used by
 * a client to avoid issues.
 *
 * \param[in] p  The prinbee server we are listening for.
 * \param[in] a  The address to listen on.
 */
listener::listener(proxy * p, addr::addr const & a)
    : binary_server(a)
    , f_proxy(p)
{
}


listener::~listener()
{
}


/** \brief Process a new direct connection.
 *
 * When a client directly connects to a prinbee daemon, this callback gets
 * called. It registers the client and sets up callback functions that handle
 * messages received by the daemon.
 *
 * \param[in] client  The binary server client object that just connected
 * to us.
 */
void listener::process_new_connection(prinbee::binary_server_client::pointer_t client)
{
    // call the base class version
    //
    binary_server::process_new_connection(client);

    // I don't think clients need to acknowledge anything, the proxy sends
    // acknowledgment to them though
    //client->add_message_callback(
    //      prinbee::g_message_acknowledge
    //    , std::bind(&proxy::msg_acknowledge, f_proxy, client, std::placeholders::_1));
    client->add_message_callback(
          prinbee::g_message_error
        , std::bind(&proxy::msg_error, f_proxy, client, std::placeholders::_1));
    client->add_message_callback(
          prinbee::g_message_ping
        , std::bind(&proxy::msg_ping, f_proxy, client, std::placeholders::_1));
    //client->add_message_callback( -- we reply with a PING, we don't get PONG from clients
    //      prinbee::g_message_pong
    //    , std::bind(&proxy::msg_pong, f_proxy, client, std::placeholders::_1));
    client->add_message_callback(
          prinbee::g_message_register
        , std::bind(&proxy::msg_register, f_proxy, client, std::placeholders::_1));

    // messages to forward to daemons
    //
    client->add_message_callback(
          prinbee::g_message_unknown
        , std::bind(&proxy::msg_forward, f_proxy, client, std::placeholders::_1));

    client->set_disconnected_callback(std::bind(&proxy::client_disconnected, f_proxy, std::placeholders::_1));

    f_proxy->register_client(client);
}



} // namespace prinbee_proxy
// vim: ts=4 sw=4 et
