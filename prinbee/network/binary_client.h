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
#include    <eventdispatcher/tcp_client_connection.h>



namespace prinbee
{



class binary_client
    : public ed::tcp_client_connection
{
public:
    typedef std::shared_ptr<binary_client>  pointer_t;

                                binary_client(addr::addr const & a);
                                binary_client(binary_client const &) = delete;
    virtual                     ~binary_client() override;

    binary_client &             operator = (binary_client const &) = delete;

    //bool                        has_input() const;
    //bool                        has_output() const;
    void                        send_message(binary_message & msg);

    // ed::tcp_client_connection implementation
    //
    virtual ssize_t             write(void const * buf, std::size_t count) override;
    virtual bool                is_writer() const override;
    virtual void                process_read() override;
    virtual void                process_write() override;
    virtual void                process_hup() override;

    // new callback
    //
    virtual void                process_message(binary_message & msg) = 0;

private:
    enum read_state_t
    {
        READ_STATE_HEADER,
        READ_STATE_HEADER_ADJUST,
        READ_STATE_DATA,
    };

    read_state_t                f_read_state = read_state_t::READ_STATE_HEADER;
    std::vector<char>           f_data = std::vector<char>();
    std::size_t                 f_data_size = 0;
    binary_message              f_binary_message = binary_message();

    std::vector<char>           f_output = std::vector<char>();
    std::size_t                 f_position = 0;
};



} // namespace prinbee
// vim: ts=4 sw=4 et
