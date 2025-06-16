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


// snapdev
//
#include    <snapdev/callback_manager.h>



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
    typedef std::shared_ptr<binary_client>              pointer_t;
    typedef std::function<bool(binary_message & msg)>   callback_t;
    typedef snapdev::callback_manager<callback_t>       callback_manager_t;

                                binary_client(addr::addr const & a);
                                binary_client(binary_client const &) = delete;
    virtual                     ~binary_client() override;

    binary_client &             operator = (binary_client const &) = delete;

    //bool                        has_input() const;
    //bool                        has_output() const;
    void                        send_message(binary_message & msg);
    callback_manager_t::callback_id_t
                                add_message_callback(
                                      message_name_t name
                                    , callback_t callback
                                    , callback_manager_t::priority_t priority = callback_manager_t::DEFAULT_PRIORITY);
    std::string const &         get_last_error() const;

    // ed::tcp_client_connection implementation
    //
    virtual void                process_timeout() override;

    // new callback
    //
    virtual void                process_message(binary_message & msg) = 0;
    virtual void                process_connected();
    virtual void                process_disconnected();

private:
    enum read_state_t
    {
        READ_STATE_HEADER,
        READ_STATE_HEADER_ADJUST,
        READ_STATE_DATA,
    };

    typedef std::map<message_name_t, callback_manager_t>    callback_map_t;

    addr::addr                  f_remote_address = addr::addr();

    //read_state_t                f_read_state = read_state_t::READ_STATE_HEADER;
    //std::vector<char>           f_data = std::vector<char>();
    //std::size_t                 f_data_size = 0;
    //binary_message              f_binary_message = binary_message();

    //std::vector<char>           f_output = std::vector<char>();
    //std::size_t                 f_position = 0;
    callback_map_t              f_callback_map = callback_map_t();

    std::shared_ptr<detail::binary_client_impl>
                                f_impl = std::shared_ptr<detail::binary_client_impl>();
    std::string                 f_last_error = std::string();
};



} // namespace prinbee
// vim: ts=4 sw=4 et
