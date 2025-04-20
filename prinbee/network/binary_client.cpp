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

/** \file
 * \brief Client connection handling binary messages between prinbee components.
 *
 * The Prinbee accepts binary connections from clients and daemons from
 * proxies. This implements such connections.
 */


// self
//
#include    "prinbee/network/binary_client.h"



//// prinbee
////
//#include    <prinbee/names.h>
//
//
//// eventdispatcher
////
//#include    <eventdispatcher/names.h>
//
//
//// communicatord
////
//#include    <communicatord/names.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{


/** \class binary_client
 * \brief Handle messages from clients, proxies, Prinbee daemons.
 *
 * This class is an implementation of the event dispatcher TCP server
 * connection used to connect to the Proxy or Prinbee Daemon.
 */



/** \brief A binary connection to communicate with Prinbee.
 *
 * This connection is used to communicate between clients, proxies, and
 * daemons using binary messages which are way more compact than the
 * communicator daemon messages that use text.
 *
 * \param[in] a  The address to connect to.
 */
binary_client::binary_client(addr::addr const & a)
    : tcp_client_connection(a)
{
}


binary_client::~binary_client()
{
}


void binary_client::process_read()
{
}



} // namespace prinbee
// vim: ts=4 sw=4 et
