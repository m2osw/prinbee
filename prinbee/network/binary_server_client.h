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


// eventdispatcher
//
#include    <eventdispatcher/tcp_server_client_connection.h>


// snapdev
//
#include    <snapdev/callback_manager.h>



namespace prinbee
{



class binary_server_client
    : public ed::tcp_server_client_connection
{
public:
    typedef std::shared_ptr<binary_server_client>       pointer_t;
    //typedef std::function<bool(binary_message & msg)>   callback_t;
    //typedef snapdev::callback_manager<callback_t>       callback_manager_t;

                                binary_server_client(ed::tcp_bio_client::pointer_t client);
                                binary_server_client(binary_server_client const &) = delete;
    virtual                     ~binary_server_client() override;

    binary_server_client &      operator = (binary_server_client const &) = delete;

    void                        send_message(binary_message & msg);
    binary_message::callback_manager_t::callback_id_t
                                add_message_callback(
                                      message_name_t name
                                    , binary_message::callback_t callback
                                    , binary_message::callback_manager_t::priority_t priority = binary_message::callback_manager_t::DEFAULT_PRIORITY);

    // ed::tcp_server_client_connection implementation
    //
    virtual ssize_t             write(void const * buf, std::size_t count) override;
    virtual bool                is_writer() const override;
    virtual void                process_read() override;
    virtual void                process_write() override;
    virtual void                process_hup() override;

    // new callback
    //
    virtual void                process_message(binary_message::pointer_t msg);

private:
    enum read_state_t
    {
        READ_STATE_HEADER,
        READ_STATE_HEADER_ADJUST,
        READ_STATE_DATA,
    };

    //typedef std::map<message_name_t, binary_message::callback_manager_t>    callback_map_t;

    binary_message::pointer_t   get_binary_message();
    void                        reset_binary_message();

    binary_message::callback_map_t
                                f_callback_map = binary_message::callback_map_t();

    read_state_t                f_read_state = read_state_t::READ_STATE_HEADER;
    std::vector<char>           f_data = std::vector<char>();
    std::size_t                 f_data_size = 0;
    binary_message::pointer_t   f_binary_message = binary_message::pointer_t();

    std::vector<char>           f_output = std::vector<char>();
    std::size_t                 f_position = 0;
};



} // namespace prinbee
// vim: ts=4 sw=4 et
