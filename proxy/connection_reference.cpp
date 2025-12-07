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
#include    "connection_reference.h"

#include    "proxy.h"


// prinbee
//
#include    <prinbee/network/binary_client.h>
#include    <prinbee/network/binary_server_client.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_proxy
{



/** \class connection_reference
 * \brief Define a reference to a connection inside the prinbee daemon.
 *
 * We want a single list of connections. The reference is used to create
 * a list of all the connections a prinbee daemon manages.
 *
 * 1. incoming connections from other prinbee daemons (binary_server_client)
 * 2. outgoing connections to other prinbee daemons (node_client)
 * 3. incoming connections from prinbee proxies (binary_server_client)
 * 4. incoming connections from prinbee clients (binary_server_client)
 */






connection_reference::connection_reference(ed::connection::pointer_t c)
    : f_connection_date(snapdev::now())
    , f_connection(c)
{
}


connection_reference::~connection_reference()
{
}


/** \brief Time at which this connection reference object was created.
 *
 * This date is used to know whether the connection is invalid. It is
 * considered invalid if it never sends us a REG message to register
 * itself with its protocol version.
 *
 * The proxy service checks all of its connections and if the protocol
 * is still undefined (version 0.0) then it checks the date. If more
 * than a few seconds, then it is considered invalid and the connection
 * is severed.
 *
 * \return The date and time when the connection was created.
 */
snapdev::timespec_ex const & connection_reference::get_connection_date() const
{
    return f_connection_date;
}


//void connection_reference::set_name(std::string const & name)
//{
//    f_name = name;
//}
//
//
//std::string const & connection_reference::get_name() const
//{
//    return f_name;
//}


//void connection_reference::set_connection(ed::connection::pointer_t connection)
//{
//    f_connection = connection;
//}


ed::connection::pointer_t connection_reference::get_connection() const
{
    return f_connection;
}


addr::addr connection_reference::get_remote_address() const
{
    {
        prinbee::binary_server_client::pointer_t c(std::dynamic_pointer_cast<prinbee::binary_server_client>(f_connection));
        if(c != nullptr)
        {
            return c->get_remote_address();
        }
    }

    {
        prinbee::binary_client::pointer_t c(std::dynamic_pointer_cast<prinbee::binary_client>(f_connection));
        if(c != nullptr)
        {
            return c->get_remote_address();
        }
    }

    // the following should never happen since we know of all the possible
    // types of clients
    //
    throw prinbee::logic_error("could not determine peer to retrieve its IP address."); // LCOV_EXCL_LINE
}


void connection_reference::set_protocol(versiontheca::versiontheca::pointer_t protocol)
{
    f_protocol = protocol;
}


versiontheca::versiontheca::pointer_t connection_reference::get_protocol() const
{
    return f_protocol;
}


void connection_reference::set_expected_ping(std::uint32_t serial_number)
{
    f_ping_serial_number = serial_number;
}


std::uint32_t connection_reference::get_expected_ping()
{
    return f_ping_serial_number;
}


bool connection_reference::has_expected_ping(std::uint32_t serial_number)
{
    if(f_ping_serial_number == serial_number)
    {
        // got a match, reset these numbers
        //
        f_ping_serial_number = 0;
        f_no_pong_answer = 0;

        return true;
    }

    return false;
}


std::uint32_t connection_reference::increment_no_pong_answer()
{
    return ++f_no_pong_answer;
}



} // namespace prinbee_proxy
// vim: ts=4 sw=4 et
