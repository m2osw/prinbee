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
#include    "prinbee/network/binary_server_client.h"


// snaplogger
//
#include    <snaplogger/message.h>


// eventdispatcher
//
#include    <eventdispatcher/communicator.h>


//// communicatord
////
//#include    <communicatord/names.h>


// last include
//
#include    <snapdev/poison.h>



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
 *
 * \warning
 * This class is considered private to the prinbee environment.
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
    // a new client just connected, create a new service_connection
    // object and add it to the ed::communicator object.
    //
    ed::tcp_bio_client::pointer_t const new_client(accept());
    if(new_client == nullptr)
    {
        // an error occurred, report in the logs
        //
        int const e(errno);
        SNAP_LOG_ERROR
            << "somehow accept() of a binary connection failed with errno: "
            << e
            << " -- "
            << strerror(e)
            << SNAP_LOG_SEND;
        return;
    }

    binary_server_client::pointer_t service(std::make_shared<binary_server_client>(new_client));
    addr::addr const remote_addr(service->get_remote_address());
    service->set_name(
              "in: "
            + remote_addr.to_ipv4or6_string(addr::STRING_IP_BRACKET_ADDRESS | addr::STRING_IP_PORT));

    if(!ed::communicator::instance()->add_connection(service))
    {
        // this should never happen here since each new creates a
        // new pointer
        //
        SNAP_LOG_ERROR
            << "new client \""
            << service->get_name()
            << "\" connection could not be added to the ed::communicator list of connections."
            << SNAP_LOG_SEND;
    }
}



} // namespace prinbee
// vim: ts=4 sw=4 et
