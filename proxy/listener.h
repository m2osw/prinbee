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
#include    "proxy.h"


// prinbee
//
#include    <prinbee/network/binary_server.h>



namespace prinbee_proxy
{



//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Weffc++"
class listener
    : public prinbee::binary_server
{
public:
    typedef std::shared_ptr<listener> pointer_t;

                                listener(proxy * p, addr::addr const & a);
                                listener(listener const & rhs) = delete;
    virtual                     ~listener() override;

    listener &                  operator = (listener const & rhs) = delete;

    // prinbee::binary_server implementation
    //
    virtual void                process_new_connection(prinbee::binary_server_client::pointer_t client) override;

private:
    proxy *                     f_proxy = nullptr;
};
//#pragma GCC diagnostic pop



} // namespace prinbee_proxy
// vim: ts=4 sw=4 et
