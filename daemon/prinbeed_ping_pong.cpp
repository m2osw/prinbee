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
 * \brief Daemon handling the PING and PONG messages.
 *
 * This file specifically implements the PING and PONG messages. These
 * are used to make sure connections stay up and running.
 */


// self
//
#include    "prinbeed.h"

//#include    "connection_reference.h"
//#include    "node_client.h"
//#include    "node_listener.h"
//#include    "proxy_listener.h"
//#include    "direct_listener.h"



// prinbee
//
//#include    <prinbee/exception.h>
//#include    <prinbee/names.h>
//#include    <prinbee/version.h>


// communicatord
//
//#include    <communicatord/flags.h>
//#include    <communicatord/names.h>


// cluck
//
//#include    <cluck/cluck_status.h>


// cppprocess
//
//#include    <cppprocess/io_capture_pipe.h>
//#include    <cppprocess/process.h>


//// eventdispatcher
////
//#include    <eventdispatcher/names.h>


// libaddr
//
//#include    <libaddr/addr_parser.h>


// snapdev
//
//#include    <snapdev/gethostname.h>
//#include    <snapdev/stringize.h>
//#include    <snapdev/tokenize_string.h>
//#include    <snapdev/to_lower.h>


// snaplogger
//
//#include    <snaplogger/logger.h>
//#include    <snaplogger/options.h>
//#include    <snaplogger/severity.h>


// advgetopt
//
//#include    <advgetopt/advgetopt.h>
//#include    <advgetopt/exception.h>


//// C++
////
//#include    <algorithm>
//#include    <iostream>
//#include    <sstream>
//
//
//// openssl
////
//#include    <openssl/rand.h>


// last include
//
#include    <snapdev/poison.h>






namespace prinbee_daemon
{



void prinbeed::send_pings()
{
    // to make sure things stay connected, send a ping to other nodes
    // once in a while -- we only send those to other prinbee daemon;
    // the clients sends PING to proxies or daemons and proxies send
    // PING to daemons
    //
    addr::addr const my_address(f_messenger->get_my_address());
    for(auto const & ref : f_connection_references)
    {
        connection_type_t const type(ref->get_connection_type());
        if(type == connection_type_t::CONNECTION_TYPE_NODE
        && ref->get_remote_address() < my_address)
        {
            if(ref->get_expected_ping() != 0)
            {
                std::uint32_t const count(ref->increment_no_pong_answer());
                if(count >= MAX_PING_PONG_FAILURES)
                {
                    SNAP_LOG_ERROR
                        << "connection never replied from our last "
                        << MAX_PING_PONG_FAILURES
                        << " PING signals; reconnecting."
                        << SNAP_LOG_SEND;
                    throw prinbee::not_yet_implemented("easy in concept, we'll implement that later though...");

                    // don't send a PING now, just loop to handle the next connection
                    //
                    continue;
                }
                SNAP_LOG_MAJOR
                    << "connection never replied from our last PING signal ("
                    << count
                    << ")."
                    << SNAP_LOG_SEND;
            }

            prinbee::binary_message::pointer_t ping_msg(std::make_shared<prinbee::binary_message>());
            ping_msg->create_ping_message();
            ref->set_expected_ping(ping_msg->get_serial_number());
            send_message(ref->get_connection(), ping_msg);
        }
    }
}


bool prinbeed::msg_ping(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    prinbee::binary_message::pointer_t pong(std::make_shared<prinbee::binary_message>());
    pong->create_pong_message(msg);
    send_message(peer, pong);

    return true;
}


bool prinbeed::msg_pong(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    snapdev::NOT_USED(peer);

    prinbee::msg_pong_t pong;
    if(!msg->deserialize_pong_message(pong))
    {
        return true;
    }

    // look for a match
    //
    for(auto const & ref : f_connection_references)
    {
        connection_type_t const type(ref->get_connection_type());
        if(type == connection_type_t::CONNECTION_TYPE_NODE
        && ref->has_expected_ping(pong.f_ping_serial_number))
        {
            SNAP_LOG_VERBOSE
                << "PONG found a corresponding PING request."
                << SNAP_LOG_SEND;
            return true;
        }
    }

    // no match was found; this can happen if the connection is lost
    // in between the sending of the reply and the handling of the reply
    //
    SNAP_LOG_MINOR
        << "received a PONG without a corresponding PING request."
        << SNAP_LOG_SEND;

    return true;
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
