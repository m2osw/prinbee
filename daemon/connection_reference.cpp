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

#include    "prinbeed.h"


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_daemon
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






connection_reference::connection_reference()
{
}


connection_reference::~connection_reference()
{
}


void connection_reference::set_name(std::string const & name)
{
    f_name = name;
}


std::string const & connection_reference::get_name() const
{
    return f_name;
}


void connection_reference::set_connection(ed::connection::pointer_t connection)
{
    f_connection = connection;
}


ed::connection::pointer_t connection_reference::get_connection() const
{
    return f_connection;
}


void connection_reference::set_protocol(versiontheca::versiontheca::pointer_t protocol)
{
    f_protocol = protocol;
}


versiontheca::versiontheca::pointer_t connection_reference::get_protocol() const
{
    return f_protocol;
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
