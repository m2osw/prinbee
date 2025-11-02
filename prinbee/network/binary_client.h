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
#include    <prinbee/network/binary_message.h>


// libaddr
//
#include    <libaddr/addr.h>


// eventdispatcher
//
#include    <eventdispatcher/timer.h>



namespace prinbee
{



namespace detail
{



class binary_client_impl;



} // detail namespace



class binary_client
    : public ed::timer
{
public:
    typedef std::shared_ptr<binary_client>          pointer_t;

                                binary_client(addr::addr const & a);
                                binary_client(binary_client const &) = delete;
    virtual                     ~binary_client() override;

    binary_client &             operator = (binary_client const &) = delete;

    //bool                        has_input() const;
    //bool                        has_output() const;
    void                        send_message(binary_message & msg);
    binary_message::callback_manager_t::callback_id_t
                                add_message_callback(
                                      message_name_t name
                                    , binary_message::callback_t callback
                                    , binary_message::callback_manager_t::priority_t priority = binary_message::callback_manager_t::DEFAULT_PRIORITY);
    std::string const &         get_last_error() const;

    // ed::tcp_client_connection implementation
    //
    virtual void                process_timeout() override;

    // new callback
    //
    virtual void                process_message(binary_message::pointer_t msg); // passed by pointer so we can save it in our fifo between pool workers
    virtual void                process_connected();
    virtual void                process_disconnected();

private:
    addr::addr                  f_remote_address = addr::addr();
    binary_message::callback_map_t
                                f_callback_map = binary_message::callback_map_t();
    std::shared_ptr<detail::binary_client_impl>
                                f_impl = std::shared_ptr<detail::binary_client_impl>();
    std::string                 f_last_error = std::string();
};



} // namespace prinbee
// vim: ts=4 sw=4 et
