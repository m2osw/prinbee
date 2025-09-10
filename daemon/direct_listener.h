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
#include    "prinbeed.h"


// prinbee
//
#include    <prinbee/network/binary_server.h>
#include    <prinbee/network/binary_server_client.h>


// C
//
//#include    <signal.h>



namespace prinbee_daemon
{



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
class direct_listener
    : public prinbee::binary_server
{
public:
    typedef std::shared_ptr<direct_listener> pointer_t;

                                direct_listener(prinbeed * p, addr::addr const & a);
    virtual                     ~direct_listener() override;

    // prinbee::binary_server implementation
    //
    virtual void                process_new_connection(prinbee::binary_server_client::pointer_t client) override;

private:
    prinbeed *                  f_prinbeed = nullptr;
};
#pragma GCC diagnostic pop



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
