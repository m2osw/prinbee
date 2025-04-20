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

// eventdispatcher
//
#include    <eventdispatcher/tcp_server_connection.h>



namespace prinbee
{



class prinbeed;


class binary_server
    : public ed::tcp_server_connection
{
public:
    typedef std::shared_ptr<binary_server>  pointer_t;

                                binary_server(addr::addr const & a);
                                binary_server(binary_server const &) = delete;
    virtual                     ~binary_server() override;

    binary_server &             operator = (binary_server const &) = delete;

    // ed::connection implementation
    //
    virtual void                process_accept() override;
};



} // namespace prinbee
// vim: ts=4 sw=4 et
