// Copyright (c) 2016-2024  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/cluck
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
 * \brief Messenger for the prinbee daemon.
 *
 * The Prinbee daemon has a normal messenger connection. This is used to
 * find the daemons and connect to them. The clients make use of a
 * direct connection so communication can happen with large binary data
 * (i.e. large files are to be sent to the backends).
 */


// self
//
#include    "prinbee/network/binary_server.h"

//#include    "prinbeed.h"
//
//
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
//
//
//// last include
////
//#include    <snapdev/poison.h>



namespace prinbee
{


/** \class binary_server
 * \brief Handle messages from clients, proxies, Prinbee daemons.
 *
 * This class is an implementation of the event dispatcher TCP server
 * connection used to accept connections used to handle binary messages.
 *
 * The class is used in the proxy services and the prinbee daemons.
 *
 * Once a connection is obtained, it creates a binary_client object.
 */



/** \brief A binary_server to listen for connection requests.
 *
 * This connection is used to listen for new connection requests between
 * clients, proxies, and daemons using binary messages which are much
 * more compact than the communicator daemon messages that use text.
 *
 * \param[in] a  The address to listen on, it can be ANY.
 * \param[in] opts  The options received from the command line.
 */
binary_server::binary_server(addr::addr const & a)
    : tcp_server_connection(a, std::string(), std::string())
{
}


binary_server::~binary_server()
{
}


void binary_server::process_accept()
{
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
