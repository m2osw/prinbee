// Copyright (c) 2024-2025  Made to Order Software Corp.  All Rights Reserved
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

/** \file
 * \brief A connection used by clients to connect and communicate with Prinbee.
 *
 * This file defines the prinbee_connection class which is expected to be
 * used by clients to communicate with Prinbee. It keeps track of the
 * status of Prinbee (mainly UP or DOWN, but it knows about the state of
 * each node too).
 *
 * The class also offers functions to send data (write) and retrieve data
 * (read) from the Prinbee database.
 */

// self
//
#include    <prinbee/state.h>
//#include    <prinbee/bigint/uint512.h>



// eventdispatcher
//
#include    <eventdispatcher/connection.h>
#include    <eventdispatcher/message.h>


// advgetopt
//
#include    <advgetopt/advgetopt.h>


// C++
//
#include    <functional>
#include    <memory>



namespace prinbee
{



class prinbee_connection
{
public:
    typedef std::shared_ptr<prinbee_connection> pointer_t;
    typedef std::function<void(state status)>   status_change_t;

                        prinbee_connection(advgetopt::getopt & opts);
    virtual             ~prinbee_connection();

    void                add_prinbee_commands();

private:
    void                msg_prinbee_current_status(ed::message & msg);
    void                msg_status(ed::message & msg);
    void                msg_ready(ed::message & msg);

    advgetopt::getopt & f_opts;
    state               f_prinbee_state = state();
};



} // namespace prinbee
// vim: ts=4 sw=4 et
