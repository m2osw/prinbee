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
#include    "daemon.h"

#include    "proxy.h"


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_proxy
{



/** \class node_client
 * \brief Implementation of a node_client connection.
 *
 * The proxy supports two types of connections. Those from clients and
 * those to daemons. This one is used to connect to the daemons.
 *
 * For most messages received by the client, the proxy forward them to
 * the daemons as is. There are a few exceptions:
 *
 * \li commands used to write data (insert, set, update, delete) are
 *     journaled first so if the daemon does not acknowledge the
 *     change for some time, the proxy can try again (possibly with
 *     a different daemon)
 * \li commands to get data available in the proxy's cache are not sent
 *     to the daemons
 * \li commands directed to the proxy itself are not forwarded
 */



/** \brief Initialize the daemon object.
 *
 * The daemon object is a permanent connection to a Prinbee daemon. This
 * means if the connection goes down, it will auto-reconnect over and
 * over again until we quit the proxy.
 *
 * The proxy uses this type of object to communicate with all the Prinbee
 * daemons.
 *
 * \param[in] p  The proxy server.
 */
daemon::daemon(proxy * p, addr::addr const & a)
    : binary_client(a)
    , f_proxy(p)
{
SNAP_LOG_WARNING << "--- we started a proxy::daemon client..." << SNAP_LOG_SEND;
}


daemon::~daemon()
{
}


/** \brief Add callbacks to automatically dispatch messages.
 *
 * This function is called from:
 *
 * proxy::connect_to_daemon(addr::addr const & a, std::string const & name);
 *
 * so we do not need to register ourselves since it is done by that function.
 */
void daemon::add_callbacks()
{
    pointer_t d(std::dynamic_pointer_cast<daemon>(shared_from_this()));
    add_message_callback(
          prinbee::g_message_error
        , std::bind(&daemon::msg_error, d, d, std::placeholders::_1));
    add_message_callback(
          prinbee::g_message_acknowledge
        , std::bind(&daemon::msg_acknowledge, d, d, std::placeholders::_1));
    // prinbee daemons do not send proxies PING messages, proxies do
    //add_message_callback(
    //      prinbee::g_message_ping
    //    , std::bind(&proxy::msg_ping, f_proxy, shared_from_this(), std::placeholders::_1));
    add_message_callback(
          prinbee::g_message_pong
        , std::bind(&daemon::msg_pong, d, d, std::placeholders::_1));

// TODO: add list of messages we know we do not accept coming in such as REG and PING

    // other replies by the daemons
    //
    add_message_callback(
          prinbee::g_message_unknown
        , std::bind(
              &proxy::msg_process_reply
            , f_proxy
            , d
            , std::placeholders::_1
            , prinbee::msg_reply_t::MSG_REPLY_RECEIVED));
}


void daemon::process_connected()
{
    binary_client::process_connected();

    // on connection, send a REG, we expect an ACK or ERR as a reply
    //
    prinbee::binary_message::pointer_t register_msg(std::make_shared<prinbee::binary_message>());
    register_msg->create_register_message(
          f_proxy->get_node_name() + "_proxy"
        , prinbee::g_name_prinbee_protocol_version_node);
    send_message(register_msg);

    expect_acknowledgment(register_msg);
}


/** \brief Record the fact that a message is expecting an acknowledgment.
 *
 * After sending certain messages to a daemon, the proxy expects an
 * acknowledgment.
 *
 * For example, when we send the REG (register) message, we expect the ACK
 * (acknowledgment) reply to clearly say that the message was positively
 * received.
 *
 * If an error occurs, the reply is an ERR (error) instead.
 *
 * \param[in] msg  The message expecting a reply.
 */
void daemon::expect_acknowledgment(prinbee::binary_message::pointer_t msg)
{
    f_expected_acknowledgment[msg->get_serial_number()] = msg;
}


bool daemon::msg_pong(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    snapdev::NOT_USED(peer);

    prinbee::msg_pong_t pong;
    if(!msg->deserialize_pong_message(pong))
    {
        return true;
    }

    // make sure it was a match
    //
    if(has_expected_ping(pong.f_ping_serial_number))
    {
        SNAP_LOG_VERBOSE
            << "PONG found a corresponding PING request ("
            << pong.f_ping_serial_number
            << ")."
            << SNAP_LOG_SEND;
    }
    else
    {
        // no match was found; this can happen if the connection is lost
        // in between the sending of the reply and the handling of the reply
        //
        SNAP_LOG_MINOR
            << "received a PONG without a corresponding PING request."
            << SNAP_LOG_SEND;
    }

    return true;
}


bool daemon::msg_error(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    snapdev::NOT_USED(peer);

    prinbee::msg_error_t err;
    msg->deserialize_error_message(err);

    SNAP_LOG_ERROR
        << peer->get_name()
        << ": "
        << err.f_message_name
        << " ("
        << static_cast<int>(err.f_code)
        << ")"
        << SNAP_LOG_SEND;

    // acknowledge failure
    //
    process_acknowledgment(err.f_serial_number, false);

    return true;
}


bool daemon::msg_acknowledge(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    snapdev::NOT_USED(peer);

SNAP_LOG_ERROR << "--- got acknowledgment..." << SNAP_LOG_SEND;
    prinbee::msg_acknowledge_t ack;
    if(!msg->deserialize_acknowledge_message(ack))
    {
SNAP_LOG_ERROR << "--- acknowledgment deserialization failed..." << SNAP_LOG_SEND;
        return true;
    }

    // acknowledge success
    //
SNAP_LOG_ERROR << "--- process acknowledgment now..." << SNAP_LOG_SEND;
    process_acknowledgment(ack.f_serial_number, true);

    return true;
}


void daemon::process_acknowledgment(prinbee::message_serial_t serial_number, bool success)
{
    auto it(f_expected_acknowledgment.find(serial_number));
    if(it == f_expected_acknowledgment.end())
    {
        // message to acknowledge not found
        //
SNAP_LOG_ERROR << "--- acknowledgment not found..." << SNAP_LOG_SEND;
        return;
    }
    prinbee::binary_message::pointer_t acknowledged_msg(it->second);
    f_expected_acknowledgment.erase(it);

SNAP_LOG_ERROR << "--- acknowledgment generates reply call..." << SNAP_LOG_SEND;
    f_proxy->msg_process_reply(
          shared_from_this()
        , acknowledged_msg
        , success ? prinbee::MSG_REPLY_SUCCEEDED : prinbee::MSG_REPLY_FAILED);
}


prinbee::message_serial_t daemon::get_expected_ping() const
{
    return f_ping_serial_number;
}


void daemon::set_expected_ping(prinbee::message_serial_t serial_number)
{
    f_ping_serial_number = serial_number;
}


bool daemon::has_expected_ping(prinbee::message_serial_t serial_number)
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


std::uint32_t daemon::increment_no_pong_answer()
{
    return ++f_no_pong_answer;
}



} // namespace daemon
// vim: ts=4 sw=4 et
