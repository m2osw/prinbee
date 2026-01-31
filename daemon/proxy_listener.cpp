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
#include    "proxy_listener.h"

#include    "prinbeed.h"


// snaplogger
//
#include    <snaplogger/message.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_daemon
{



/** \class proxy_listener
 * \brief Handle connections from proxies.
 *
 * The prinbee environment expects clients to connect to a proxy instead
 * of directly to one of its daemon. This allows for more data
 * safety by having more control over journaling. It can save data
 * being written on the client's machine until a server is available.
 * It also handles connections to the servers so that way it can
 * send the data directly to the correct server.
 */



/** \brief The proxy listener initialization.
 *
 * This function initializes the proxy listener.
 *
 * \param[in] p  The prinbee server we are listening for.
 * \param[in] a  The address to listen on.
 */
proxy_listener::proxy_listener(prinbeed * p, addr::addr const & a)
    : binary_server(a)
    , f_prinbeed(p)
{
    set_name("proxy_listener");
SNAP_LOG_WARNING << "--- proxy listener using address: " << a << " ---" << SNAP_LOG_SEND;
}


proxy_listener::~proxy_listener()
{
}


/** \brief Initialize a client.
 *
 * Whenever the listener receives a connection through the accept() command,
 * this function gets called with a new client.
 *
 * The function makes further client's initialization such as setting up
 * callbacks for various messages.
 *
 * Finally, it registers the connection with the prinbee daemon so it
 * can be properly managed until disconnected.
 *
 * \param[in] client  The binary server client object that just connected.
 */
void proxy_listener::process_new_connection(prinbee::binary_server_client::pointer_t client)
{
SNAP_LOG_WARNING << "--- process new connection called! ---" << SNAP_LOG_SEND;

    // call the base class version
    //
    binary_server::process_new_connection(client);

    client->add_message_callback(
          prinbee::g_message_error
        , std::bind(&prinbeed::msg_error, f_prinbeed, client, std::placeholders::_1));
    client->add_message_callback(
          prinbee::g_message_ping
        , std::bind(&prinbeed::msg_ping, f_prinbeed, client, std::placeholders::_1));

    // messages to send to workers are all sent to the same function
    //
    client->add_message_callback(
          prinbee::g_message_unknown
        , std::bind(&prinbeed::msg_process_payload, f_prinbeed, client, std::placeholders::_1));

    client->set_disconnected_callback(std::bind(&prinbeed::client_disconnected, f_prinbeed, std::placeholders::_1));

    f_prinbeed->register_connection(client, connection_type_t::CONNECTION_TYPE_PROXY);
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
