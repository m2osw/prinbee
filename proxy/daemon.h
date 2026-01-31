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

// self
//
//#include    "proxy.h"


// prinbee
//
#include    <prinbee/network/binary_client.h>



namespace prinbee_proxy
{



class proxy;


class daemon
    : public prinbee::binary_client
{
public:
    typedef std::shared_ptr<daemon>         pointer_t;
    typedef std::map<daemon *, pointer_t>   map_t;

                                daemon(proxy * p, addr::addr const & a);
                                daemon(daemon const & rhs) = delete;
    virtual                     ~daemon() override;

    daemon &                    operator = (daemon const & rhs) = delete;

    void                        add_callbacks();
    void                        expect_acknowledgment(prinbee::binary_message::pointer_t msg);

    prinbee::message_serial_t   get_expected_ping() const;
    void                        set_expected_ping(prinbee::message_serial_t serial_number);
    bool                        has_expected_ping(prinbee::message_serial_t serial_number);
    std::uint32_t               increment_no_pong_answer();

    // binary_client implementation
    //
    virtual void                process_connected();

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

    proxy *                     f_proxy = nullptr;
    acknowledgment_t            f_expected_acknowledgment = acknowledgment_t();
    prinbee::message_serial_t   f_ping_serial_number = 0;
    std::uint32_t               f_no_pong_answer = 0;
};



} // namespace daemon
// vim: ts=4 sw=4 et
