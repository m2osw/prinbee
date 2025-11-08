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
 * \brief Messenger for the prinbee daemon.
 *
 * The Prinbee daemon has a normal messenger connection. This is used to
 * find the daemons and connect to them. The clients make use of a
 * direct connection so communication can happen with large binary data
 * (i.e. large files are to be sent to the backends).
 */


// self
//
#include    "prinbee_worker.h"

#include    "prinbeed.h"


// prinbee
//
//#include    <prinbee/names.h>
#include    <prinbee/network/binary_client.h>


// cluck
//
//#include    <cluck/cluck_status.h>


// eventdispatcher
//
//#include    <eventdispatcher/names.h>


// communicatord
//
//#include    <communicatord/names.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_daemon
{



void payload_t::send_message(prinbee::binary_message::pointer_t msg)
{
    {
        prinbee::binary_server_client::pointer_t c(std::dynamic_pointer_cast<prinbee::binary_server_client>(f_peer));
        if(c != nullptr)
        {
            c->send_message(msg);
            return;
        }
    }

    {
        prinbee::binary_client::pointer_t c(std::dynamic_pointer_cast<prinbee::binary_client>(f_peer));
        if(c != nullptr)
        {
            c->send_message(msg);
            return;
        }
    }

    // the following should never happen since we know of all the possible
    // types of clients
    //
    throw prinbee::logic_error("could not determine peer to send a message to."); // LCOV_EXCL_LINE
}



/** \class prinbee_worker
 * \brief Handle binary messages from the proxy, direct client, and other nodes.
 *
 * This class allows us to handle the binary messages we receive from the
 * proxy, direct local clients, and other nodes in parallel. There will be
 * a minimum of two workers and a maximum of 2x the number of CPUs available.
 *
 * In most cases, the worker sends a reply directly to the client that sent
 * the message being processed. In most cases, though, the payload is
 * processed multiple times for each step required by that specific message.
 * Some of the functions are common to all the messages. For example, the
 * system sends a first ACK reply to let the client know that the server
 * received the message and, if required, that the message was saved in
 * the journal.
 */



/** \brief The prinbee_worker initialization.
 *
 * The messenger is the cluck daemon connection to the communicator server.
 *
 * It sets up its dispatcher and calls prinbeed functions whenever it
 * receives a message.
 *
 * \param[in] p  The prinbee object we are listening for (i.e. "daemon").
 */
prinbee_worker::prinbee_worker(
          std::string const & name
        , std::size_t position
        , typename cppthread::fifo<payload_t>::pointer_t in
        , typename cppthread::fifo<payload_t>::pointer_t out
        , prinbeed * p)
    : worker(
          name
        , position
        , in
        , out)
    , f_prinbeed(p)
{
}


prinbee_worker::~prinbee_worker()
{
}


/** \brief Process one message (or at least part of it).
 *
 * This function receives all the messages added to the FIFO by one of the
 * binary message handlers. A worker thread will pick up the work once
 * available.
 *
 * The function does a \em switch based on the message type.
 *
 * All the functions must return true or false. In most cases, they are likely
 * to return false. If a function needs further work to be done, then it can
 * return true after changing the f_payload protected member variable.
 * Note that it is also possible to create a new payload and call
 * prinbeed::msg_process_workload() with the peer and message.
 *
 * \return true if the payload has to be forwarded to a worker.
 */
bool prinbee_worker::do_work()
{
    switch(f_payload.f_message->get_name())
    {
    case prinbee::g_message_list_contexts:
        return f_prinbeed->list_contexts(f_payload);

    case prinbee::g_message_get_context:
        return f_prinbeed->get_context(f_payload);

    case prinbee::g_message_set_context:
        return f_prinbeed->set_context(f_payload);

    }

    return false;
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
