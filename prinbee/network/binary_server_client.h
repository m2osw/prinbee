// Copyright (c) 2016-2024  Made to Order Software Corp.  All Rights Reserved
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

// eventdispatcher
//
#include    <eventdispatcher/tcp_server_client_connection.h>



namespace prinbee
{



class binary_server_client
    : public ed::tcp_server_client_connection
{
public:
    typedef std::shared_ptr<binary_server_client>  pointer_t;

                                binary_server_client(ed::tcp_bio_client::pointer_t client);
                                binary_server_client(binary_server_client const &) = delete;
    virtual                     ~binary_server_client() override;

    binary_server_client &      operator = (binary_server_client const &) = delete;

    // ed::connection implementation
    //
    virtual void                process_read() override;

private:
};



} // namespace prinbee
// vim: ts=4 sw=4 et
