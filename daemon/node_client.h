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
#include    <prinbee/network/binary_client.h>


// C
//
//#include    <signal.h>



namespace prinbee_daemon
{



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
class node_client
    : public prinbee::binary_client
{
public:
    typedef std::shared_ptr<node_client> pointer_t;

                                node_client(prinbeed * p, addr::addr const & a);
                                node_client(node_client const & rhs) = delete;
    virtual                     ~node_client() override;

    node_client &               operator = (node_client const & rhs) = delete;

    void                        add_callbacks();

    // ed::connection implementation
    //
    virtual void                process_signal() override;

private:
    prinbeed *                  f_prinbeed = nullptr;
};
#pragma GCC diagnostic pop



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
