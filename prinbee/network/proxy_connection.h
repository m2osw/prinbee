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



namespace prinbee
{



class prinbee_connection;


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
    void                        expect_acknowledgment(prinbee::binary_message::pointer_t msg);
    prinbee::msg_error_t const &
                                get_last_error_message() const;

    prinbee::message_serial_t   get_expected_ping() const;
    void                        set_expected_ping(prinbee::message_serial_t serial_number);
    bool                        has_expected_ping(prinbee::message_serial_t serial_number);
    std::uint32_t               increment_no_pong_answer();
    std::uint32_t               get_no_pong_answer() const;
    snapdev::timespec_ex const &
                                get_last_ping() const;
    double                      get_proxy_loadavg() const;

    // binary_client implementation
    //
    virtual void                process_connected() override;

private:
    typedef std::map<prinbee::message_serial_t, prinbee::binary_message::pointer_t>
                                acknowledgment_t;

    bool                        msg_pong(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_error(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_acknowledge(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    void                        process_acknowledgment(
                                      std::uint32_t serial_number
                                    , bool success);

    prinbee_connection *        f_prinbee_connection = nullptr;
    acknowledgment_t            f_expected_acknowledgment = acknowledgment_t();
    prinbee::message_serial_t   f_ping_serial_number = 0;
    std::uint32_t               f_no_pong_answer = 0;
    prinbee::msg_error_t        f_last_error_message = prinbee::msg_error_t();
    snapdev::timespec_ex        f_last_ping = snapdev::timespec_ex();
    double                      f_proxy_loadavg = -2.0; // -1.0 is error and 0.0 or more a valid number from the proxy
    bool                        f_registered = false;
};



} // namespace prinbee_cui
// vim: ts=4 sw=4 et
