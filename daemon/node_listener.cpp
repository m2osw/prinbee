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
#include    "node_listener.h"



// last include
//
#include    <snapdev/poison.h>



namespace prinbee_daemon
{



/** \class node_listener
 * \brief Handle the binary server listening for node connections.
 *
 * This class handles connections from other prinbee daemons. The daemons
 * communicate between each others mainly to keep the data replicated.
 * In some cases, also a proxy may send data to the \em wrong daemon.
 * In that case, the daemon needs to forward the data to the correct
 * daemon and it can use this connection to do it.
 */



/** \brief The node_listener initialization.
 *
 * This function initializes the node_server_client connection.
 *
 * \param[in] p  The prinbee server we are listening for.
 * \param[in] a  The address to listen on.
 */
node_listener::node_listener(prinbeed * p, addr::addr const & a)
    : binary_server(a)
    , f_prinbeed(p)
{
    set_name("node_listener");
}


node_listener::~node_listener()
{
}


/** \brief Callback signaling a new connection was obtained.
 *
 * When this function is called, the listener just received a new connection
 * and created a client that will communicate with another prinbee daemon.
 *
 * \param[in] client  The binary server client object that just connected
 * to us.
 */
void node_listener::process_new_connection(prinbee::binary_server_client::pointer_t client)
{
    // call the base class version
    //
    binary_server::process_new_connection(client);

    client->add_message_callback(
          prinbee::g_message_error
        , std::bind(&prinbeed::msg_error, f_prinbeed, client, std::placeholders::_1));
    client->add_message_callback(
          prinbee::g_message_ping
        , std::bind(&prinbeed::msg_ping, f_prinbeed, client, std::placeholders::_1));
    client->add_message_callback(
          prinbee::g_message_pong
        , std::bind(&prinbeed::msg_pong, f_prinbeed, client, std::placeholders::_1));

    // messages to send to workers are all sent to the same function
    //
    client->add_message_callback(
          prinbee::g_message_unknown
        , std::bind(&prinbeed::msg_process_payload, f_prinbeed, client, std::placeholders::_1));

    client->set_disconnected_callback(std::bind(&prinbeed::client_disconnected, f_prinbeed, std::placeholders::_1));

    f_prinbeed->register_connection(client, connection_type_t::CONNECTION_TYPE_NODE);
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
