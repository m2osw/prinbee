// Copyright (c) 2016-2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Client connection handling binary messages between prinbee components.
 *
 * The Prinbee accepts binary connections from clients and daemons from
 * proxies. This implements such connections.
 */


// self
//
#include    "messenger.h"

#include    "prinbeed.h"


// prinbee
//
#include    <prinbee/names.h>


// eventdispatcher
//
#include    <eventdispatcher/names.h>


// communicatord
//
#include    <communicatord/names.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_daemon
{


/** \class binary_connection
 * \brief Handle messages from clients, proxies, Prinbee daemons.
 *
 * This class is an implementation of the event dispatcher TCP server
 * connection used to accept connection used to handle binary messages.
 *
 * The class is used in the proxy services and the prinbee daemons.
 */



/** \brief A binary connection to communicate with Prinbee.
 *
 * This connection is used to communicate between clients, proxies, and
 * daemons using binary messages which are way more compact than the
 * communicator daemon message that use text.
 *
 * \param[in] a  The address to listen on, it can be ANY.
 * \param[in] opts  The options received from the command line.
 */
binary_connection::binary_connection(addr::addr const & a)
    : tcp_server_connection(a, std::string(), std::string())
{
}


binary_connection::~binary_connection()
{
}


void binary_connection::process_accept()
{
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
