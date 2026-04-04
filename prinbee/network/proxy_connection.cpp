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
#include    "prinbee/network/proxy_connection.h"

#include    "prinbee/network/constants.h"
#include    "prinbee/network/prinbee_connection.h"


// prinbee
//
#include    <prinbee/names.h>


// snaplogger
//
#include    <snaplogger/message.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



/** \class proxy_connection
 * \brief Implementation of a proxy connection.
 *
 * The prinbee_connection connects to a Prinbee proxy daemon using a
 * proxy connection.
 *
 * The proxy connection is used to send binary messages to the proxy
 * daemon, which either interprets the message (such as the REG message)
 * or forwards it to one or more Prinbee daemons.
 *
 * \note
 * This connection is considered private within a prinbee_connection.
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
 * \param[in] a  The address to connect to.
 */
proxy_connection::proxy_connection(prinbee_connection * c, addr::addr const & a)
    : binary_client(a)
    , f_prinbee_connection(c)
    , f_communicator(ed::communicator::instance())
{
    set_name("proxy");
}


proxy_connection::~proxy_connection()
{
    f_communicator->remove_connection(f_ping_pong_timer);
}


void proxy_connection::set_ping_pong_interval(double interval)
{
    // by setting this value to 0.0, the user is turning the functionality OFF
    //
    f_ping_pong_timer_on = snapdev::quiet_floating_point_not_equal(interval, 0.0);
    if(!f_ping_pong_timer_on)
    {
        f_communicator->remove_connection(f_ping_pong_timer);
        f_ping_pong_timer.reset();
        return;
    }

    if(f_ping_pong_timer == nullptr)
    {
        ed::timer::pointer_t ping_pong_timer = std::make_shared<ed::timer>(0);
        ping_pong_timer->set_name("proxy_ping_pong_timer");
        if(!f_communicator->add_connection(ping_pong_timer))
        {
            SNAP_LOG_RECOVERABLE_ERROR
                << "could not add ping-pong timer to the list of ed::communicator connections."
                << SNAP_LOG_SEND;
            return;
        }
        f_ping_pong_timer = ping_pong_timer;

        f_ping_pong_timer->get_callback_manager().add_callback(
            [this](ed::timer::pointer_t t) {
                return this->send_ping(t);
            });
    }

    // minimum is 1 second and maximum 1 hour
    //
    interval = std::clamp(interval, 1.0, 60.0 * 60.0) * 1'000'000.0;
    f_ping_pong_timer->set_timeout_delay(interval);
}


bool proxy_connection::send_ping(ed::timer::pointer_t t)
{
    snapdev::NOT_USED(t);

    if(get_expected_ping() != 0)
    {
        std::uint32_t const count(increment_no_pong_answer());
        if(count >= MAX_PING_PONG_FAILURES)
        {
            SNAP_LOG_ERROR
                << "connection never replied from our last "
                << MAX_PING_PONG_FAILURES
                << " PING signals; reconnecting."
                << SNAP_LOG_SEND;

            // TODO: actually implement...
            //
            throw not_yet_implemented("easy in concept, we'll implement that later though...");

            // don't send a PING now
            //
            return true;
        }
        SNAP_LOG_MAJOR
            << "connection never replied from our last "
            << count
            << " PING signals."
            << SNAP_LOG_SEND;
    }

    binary_message::pointer_t ping_msg(std::make_shared<binary_message>());
    ping_msg->create_ping_message();
    set_expected_ping(ping_msg->get_serial_number());
    send_message(ping_msg);

    return false;
}


bool proxy_connection::is_ping_pong_timer_on() const
{
    return f_ping_pong_timer_on;
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
    weak_pointer_t wd(std::dynamic_pointer_cast<proxy_connection>(shared_from_this()));
    add_message_callback(
          g_message_error
        , [wd](binary_message::pointer_t msg)
          {
              pointer_t d(wd.lock());
              if(d != nullptr)
              {
                  d->msg_error(msg);
              }
              return true;
          });
    add_message_callback(
          g_message_acknowledge
        , [wd](binary_message::pointer_t msg)
          {
              pointer_t d(wd.lock());
              if(d != nullptr)
              {
                  d->msg_acknowledge(msg);
              }
              return true;
          });
    add_message_callback(
          g_message_pong
        , [wd](binary_message::pointer_t msg)
          {
              pointer_t d(wd.lock());
              if(d != nullptr)
              {
                  d->msg_pong(msg);
              }
              return true;
          });

    // other replies by the proxy
    //
    add_message_callback(
          g_message_unknown
        , [wd](binary_message::pointer_t msg)
          {
              pointer_t d(wd.lock());
              if(d != nullptr)
              {
                  d->msg_process_reply(msg, msg_reply_t::MSG_REPLY_RECEIVED);
              }
              return true;
          });
        //, std::bind(&proxy_connection::msg_process_reply, d, std::placeholders::_1, msg_reply_t::MSG_REPLY_RECEIVED));
}


void proxy_connection::process_connected()
{
    binary_client::process_connected();

    // on connection to a daemon, send a REG, we expect an ACK or ERR as a reply
    //
    binary_message::pointer_t register_msg(std::make_shared<binary_message>());
    register_msg->create_register_message(
          f_prinbee_connection->get_name() + "_client"
        , g_name_prinbee_protocol_version_node);
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
void proxy_connection::expect_acknowledgment(binary_message::pointer_t msg)
{
    f_expected_acknowledgment[msg->get_serial_number()] = msg;
}


msg_error_t const & proxy_connection::get_last_error_message() const
{
    return f_last_error_message;
}


bool proxy_connection::msg_pong(binary_message::pointer_t msg)
{
    msg_pong_t pong;
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

        f_proxy_loadavg = pong.f_loadavg_1min; // this is the proxy's loadavg which is likely the same as this client's...

        // TODO: do the necessary to get the loadavg from other sources

        f_prinbee_connection->process_proxy_status();
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


bool proxy_connection::msg_error(binary_message::pointer_t msg)
{
    msg->deserialize_error_message(f_last_error_message);

    SNAP_LOG_ERROR
        << get_name()
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


bool proxy_connection::msg_acknowledge(binary_message::pointer_t msg)
{
    msg_acknowledge_t ack;
    if(!msg->deserialize_acknowledge_message(ack))
    {
        return true;
    }

    // acknowledge success
    //
    process_acknowledgment(ack.f_serial_number, true);

    return true;
}


void proxy_connection::process_acknowledgment(message_serial_t serial_number, bool success)
{
    auto it(f_expected_acknowledgment.find(serial_number));
    if(it == f_expected_acknowledgment.end())
    {
        // message to acknowledge not found
        //
        SNAP_LOG_MINOR
            << "acknowledgment for message with serial number "
            << serial_number
            << " not found."
            << SNAP_LOG_SEND;
        return;
    }

    // we have some client <-> proxy communication that we can take care of
    // right here
    //

    msg_process_reply(
          it->second
        , success ? MSG_REPLY_SUCCEEDED : MSG_REPLY_FAILED);
}


bool proxy_connection::msg_process_reply(
      binary_message::pointer_t msg
    , msg_reply_t state)
{
SNAP_LOG_ERROR
<< "--- msg_process_reply() -- "
<< (state == MSG_REPLY_SUCCEEDED ? "(ACK) -- " : "")
<< message_name_to_string(msg->get_name())
<< SNAP_LOG_SEND;
    // received a reply from the connection, process it
    //
    // the 'msg' is either a fresh reply or the message we sent
    //
    // the ACK or ERR are _transformed_ using our sent message and setting
    // the state to MSG_REPLY_SUCCEEDED or MSG_REPLY_FAILED, whereas for
    // a fresh reply, it is set to MSG_REPLY_RECEIVED
    //
    switch(msg->get_name())
    {
    case g_message_register:
        if(state == MSG_REPLY_SUCCEEDED)
        {
            // we are registered, ready to rock
            //
            set_proxy_registered(true);

            // immediately requests the list of contexts
            //
            // we need the identifiers for any other requests so we can as
            // well get those now
            //
            get_context_list();
        }
        else
        {
            // we cannot register, trying again will fail again, what
            // to do?!
            //
            // Note: we already logged the error message
            //
            set_proxy_registered(false);
        }
        return true;

    case g_message_list_contexts:
        if(state == MSG_REPLY_RECEIVED)
        {
            save_context_list(msg);
        }
        else
        {
            // in most likelihood the proxy is shutting down or cannot
            // connect to any backend for a "long time"
            //
            //... what to do here? ...
            SNAP_LOG_ERROR
                << "could not get list of contexts from proxy."
                << SNAP_LOG_SEND;
        }
        return true;

    }

    SNAP_LOG_ERROR
        << "prinbee reply \""
        << message_name_to_string(msg->get_name())
        << "\" not understood."
        << SNAP_LOG_SEND;

    return true;
}


void proxy_connection::set_proxy_registered(bool is_registered)
{
    if(is_registered == f_registered)
    {
        return;
    }

    f_registered = is_registered;

    f_prinbee_connection->process_proxy_status();
}


bool proxy_connection::is_registered() const
{
    return f_registered;
}


void proxy_connection::get_context_list()
{
    // if we already received the list, no need to resend the message;
    //
    // TODO: we may need to have a TTL and try again...
    //
    // i.e. once we are connected changes are reported automatically and on a
    //      reconnection we do not need to get another copy of the same data
    //
    if(!f_context_list_available)
    {
        binary_message::pointer_t list_context_msg(std::make_shared<binary_message>());
        list_context_msg->create_list_contexts_message(advgetopt::string_list_t{}); // list ignored in request
        send_message(list_context_msg);
    }
}


void proxy_connection::save_context_list(binary_message::pointer_t msg)
{
    msg_list_contexts_t context_list;
    if(!msg->deserialize_list_contexts_message(context_list))
    {
        SNAP_LOG_ERROR
            << "\"list of contexts\" message could not be deserialized."
            << SNAP_LOG_SEND;
        return;
    }
    f_context_list = context_list;

    // the proxy will automatically give us a new list if/when it changes
    // so there is a race condition where our list may be out of date for
    // a little bit
    //
    f_context_list_available = true;

#ifdef _DEBUG
    std::cout << "info:client: got list of " << f_context_list.f_list.size() << " contexts.\r\n";
    for(auto c : f_context_list.f_list)
    {
        std::cout << std::setw(10) << c.f_context_id << ": " << c.f_name << "\r\n";
    }
    std::cout << std::flush;
#endif

    f_prinbee_connection->process_proxy_status();
}


bool proxy_connection::has_context_list() const
{
    return f_context_list_available;
}


message_serial_t proxy_connection::get_expected_ping() const
{
    return f_ping_serial_number;
}


void proxy_connection::set_expected_ping(message_serial_t serial_number)
{
    f_ping_serial_number = serial_number;
}


bool proxy_connection::has_expected_ping(message_serial_t serial_number)
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
