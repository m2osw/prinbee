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
#include    "proxy_connection.h"

#include    "cui.h"


// prinbee
//
#include    <prinbee/names.h>


// snaplogger
//
#include    <snaplogger/message.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_cui
{



/** \class proxy_connection
 * \brief Implementation of a proxy connection.
 *
 * The cui connects to a Prinbee proxy daemon using a proxy connection.
 *
 * The proxy connection is used to send binary messages to the proxy daemon,
 * which either interprets the message (such as the REG message) or
 * forwards it to one or more Prinbee daemons.
 */



/** \brief Initialize the proxy connection object.
 *
 * The proxy connection is a permanent connection to a Prinbee proxy daemon.
 * This means if the connection goes down, it auto-reconnects over and
 * over again until we quit the cui.
 *
 * The cui creates one proxy object to communicate with the proxy daemon.
 *
 * \param[in] c  The cui object.
 */
proxy_connection::proxy_connection(cui * c, addr::addr const & a)
    : binary_client(a)
    , f_cui(c)
{
}


proxy_connection::~proxy_connection()
{
}


/** \brief Add callbacks to automatically dispatch messages.
 *
 * This function is called from cui::start_binary_connection()
 * function.
 *
 * The function also sends the REG message and saves it in the
 * list of messages to be acknowledged.
 */
void proxy_connection::add_callbacks()
{
    pointer_t d(std::dynamic_pointer_cast<proxy_connection>(shared_from_this()));
    add_message_callback(
          prinbee::g_message_error
        , std::bind(&proxy_connection::msg_error, d, d, std::placeholders::_1));
    add_message_callback(
          prinbee::g_message_acknowledge
        , std::bind(&proxy_connection::msg_acknowledge, d, d, std::placeholders::_1));
    // prinbee daemons do not send proxies PING messages, proxies do
    //add_message_callback(
    //      prinbee::g_message_ping
    //    , std::bind(&proxy_connection::msg_ping, f_proxy, shared_from_this(), std::placeholders::_1));
    add_message_callback(
          prinbee::g_message_pong
        , std::bind(&proxy_connection::msg_pong, d, d, std::placeholders::_1));

    // other replies by the proxy
    //
    add_message_callback(
          prinbee::g_message_unknown
        , std::bind(&cui::msg_process_reply, f_cui, std::placeholders::_1, msg_reply_t::MSG_REPLY_RECEIVED));

    // send a REG, we expect an ACK or ERR as a reply
    //
    prinbee::binary_message::pointer_t register_msg(std::make_shared<prinbee::binary_message>());
    register_msg->create_register_message(
          prinbee::g_name_prinbee_cui_client
        , prinbee::g_name_prinbee_protocol_version_node);
    send_message(register_msg);

    expect_acknowledgment(register_msg);
}


/** \brief Record the fact that a message is expecting an acknowledgment.
 *
 * After sending certain messages to a proxy, the proxy connection expects
 * an acknowledgment.
 *
 * For example, when we send the REG (register) message, we expect the ACK
 * (acknowledgment) reply to clearly say that the message was positively
 * received and the proxy connection is registered.
 *
 * If an error occurs, the reply is an ERR (error) instead.
 *
 * \param[in] msg  The message expecting a reply.
 */
void proxy_connection::expect_acknowledgment(prinbee::binary_message::pointer_t msg)
{
    f_expected_acknowledgment[msg->get_serial_number()] = msg;
}


prinbee::msg_error_t const & proxy_connection::get_last_error_message() const
{
    return f_last_error_message;
}


bool proxy_connection::msg_pong(
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
            << "PONG found a corresponding PING request."
            << SNAP_LOG_SEND;

        f_proxy_loadavg = pong.f_loadavg_1min;

        // TODO: do the necessary to get the loadavg from other sources
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


bool proxy_connection::msg_error(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    snapdev::NOT_USED(peer);

    msg->deserialize_error_message(f_last_error_message);

    SNAP_LOG_ERROR
        << peer->get_name()
        << ": "
        << f_last_error_message.f_message_name
        << " ("
        << static_cast<int>(f_last_error_message.f_code)
        << ")"
        << SNAP_LOG_SEND;

    // acknowledge failure
    //
    process_acknowledgment(f_last_error_message.f_serial_number, false);

    return true;
}


bool proxy_connection::msg_acknowledge(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    snapdev::NOT_USED(peer);

    prinbee::msg_acknowledge_t ack;
    if(!msg->deserialize_acknowledge_message(ack))
    {
        return true;
    }

    // acknowledge success
    //
    process_acknowledgment(ack.f_serial_number, true);

    return true;
}


void proxy_connection::process_acknowledgment(prinbee::message_serial_t serial_number, bool success)
{
    auto it(f_expected_acknowledgment.find(serial_number));
    if(it == f_expected_acknowledgment.end())
    {
        // message to acknowledge not found
        //
        return;
    }

    f_cui->msg_process_reply(
          it->second
        , success ? MSG_REPLY_SUCCEEDED : MSG_REPLY_FAILED);
}


prinbee::message_serial_t proxy_connection::get_expected_ping() const
{
    return f_ping_serial_number;
}


void proxy_connection::set_expected_ping(prinbee::message_serial_t serial_number)
{
    f_ping_serial_number = serial_number;
}


bool proxy_connection::has_expected_ping(prinbee::message_serial_t serial_number)
{
    if(f_ping_serial_number == serial_number)
    {
        // got a match, reset these numbers
        //
        f_ping_serial_number = 0;
        f_no_pong_answer = 0;
        f_last_ping = snapdev::now();

        return true;
    }

    return false;
}


std::uint32_t proxy_connection::increment_no_pong_answer()
{
    return ++f_no_pong_answer;
}


std::uint32_t proxy_connection::get_no_pong_answer() const
{
    return f_no_pong_answer;
}


snapdev::timespec_ex const & proxy_connection::get_last_ping() const
{
    return f_last_ping;
}


double proxy_connection::get_proxy_loadavg() const
{
    return f_proxy_loadavg;
}



} // namespace prinbee_cui
// vim: ts=4 sw=4 et
