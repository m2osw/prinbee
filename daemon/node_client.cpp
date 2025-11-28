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
#include    "node_client.h"

#include    "prinbeed.h"


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_daemon
{



/** \class node_client
 * \brief Implementation of a node_client connection.
 *
 * We support several types of connections. This is the "node client" which
 * is when a prinbee daemon connects to another prinbee daemon. A daemon does
 * so when its IP address is smaller than the other prinbee daemon. In the
 * end, this creates a full mesh allowing for messages to travel from any
 * daemon to any daemon making it a lot faster when we are required to
 * replicate the data.
 */



/** \brief Initialize the node client object.
 *
 * The node client object is a permanent connection to another prinbee
 * daemon.
 *
 * The prinbee daemon uses this object when its IP address is larger
 * than that other prinbee daemon. The listeners use the
 * node_server_client instead.
 *
 * \param[in] p  The prinbee server.
 */
node_client::node_client(prinbeed * p, addr::addr const & a)
    : binary_client(a)
    , f_prinbeed(p)
{
}


node_client::~node_client()
{
}


void node_client::add_callbacks()
{
    add_message_callback(
          prinbee::g_message_error
        , std::bind(&prinbeed::msg_error, f_prinbeed, shared_from_this(), std::placeholders::_1));
    add_message_callback(
          prinbee::g_message_ping
        , std::bind(&prinbeed::msg_ping, f_prinbeed, shared_from_this(), std::placeholders::_1));
    add_message_callback(
          prinbee::g_message_pong
        , std::bind(&prinbeed::msg_pong, f_prinbeed, shared_from_this(), std::placeholders::_1));

    // messages to send to workers are all sent to the same function
    //
    add_message_callback(
          prinbee::g_message_unknown
        , std::bind(&prinbeed::msg_process_payload, f_prinbeed, shared_from_this(), std::placeholders::_1));
}


/** \brief Call the stop function of the prinbeed object.
 *
 * When this function is called, the signal was received and thus we are
 * asked to quit as soon as possible.
 */
void node_client::process_signal()
{
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
