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
#pragma once

// prinbee
//
#include    <prinbee/network/binary_client.h>


// eventdispatcher
//
#include    <eventdispatcher/communicator.h>


namespace prinbee
{



class prinbee_connection;


enum msg_reply_t
{
    MSG_REPLY_RECEIVED,         // when we receive a message (i.e. not ACK nor ERR)
    MSG_REPLY_FAILED,           // ERR a message we sent
    MSG_REPLY_SUCCEEDED,        // ACK a message we sent
};


class proxy_connection
    : public prinbee::binary_client
{
public:
    typedef std::shared_ptr<proxy_connection>         pointer_t;

                                proxy_connection(prinbee_connection * c, addr::addr const & a);
                                proxy_connection(proxy_connection const & rhs) = delete;
    virtual                     ~proxy_connection() override;

    proxy_connection &          operator = (proxy_connection const & rhs) = delete;

    void                        add_callbacks();
    void                        set_ping_pong_interval(double interval); // in seconds
    bool                        is_ping_pong_timer_on() const;
    void                        expect_acknowledgment(prinbee::binary_message::pointer_t msg);
    prinbee::msg_error_t const &
                                get_last_error_message() const;

    prinbee::message_serial_t   get_expected_ping() const;
    void                        set_expected_ping(prinbee::message_serial_t serial_number);
    std::uint32_t               increment_no_pong_answer();
    std::uint32_t               get_no_pong_answer() const;
    snapdev::timespec_ex const &
                                get_last_ping() const;
    double                      get_proxy_loadavg() const;
    bool                        is_registered() const;

    // binary_client implementation
    //
    virtual void                process_connected() override;

private:
    typedef std::map<prinbee::message_serial_t, prinbee::binary_message::pointer_t>
                                acknowledgment_t;

    bool                        msg_pong(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        has_expected_ping(prinbee::message_serial_t serial_number);
    bool                        send_ping(ed::timer::pointer_t t);

    bool                        msg_process_reply(
                                      prinbee::binary_message::pointer_t msg
                                    , msg_reply_t state);

    bool                        msg_error(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);

    bool                        msg_acknowledge(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    void                        process_acknowledgment(
                                      std::uint32_t serial_number
                                    , bool success);
    void                        set_proxy_registered(bool is_registered);

    prinbee_connection *        f_prinbee_connection = nullptr;
    ed::communicator::pointer_t f_communicator = ed::communicator::pointer_t();
    acknowledgment_t            f_expected_acknowledgment = acknowledgment_t();
    prinbee::msg_error_t        f_last_error_message = prinbee::msg_error_t();
    double                      f_proxy_loadavg = -2.0; // -1.0 is error and 0.0 or more a valid number from the proxy
    bool                        f_registered = false; // becomes true once we get the ACK reply from our REG message

    // support for PING / PONG messages
    //
    ed::timer::pointer_t        f_ping_pong_timer = ed::timer::pointer_t();
    prinbee::message_serial_t   f_ping_serial_number = 0;
    std::uint32_t               f_no_pong_answer = 0;
    snapdev::timespec_ex        f_last_ping = snapdev::timespec_ex();
    bool                        f_ping_pong_timer_on = false;
};



} // namespace prinbee_cui
// vim: ts=4 sw=4 et
