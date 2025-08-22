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
#include    "messenger.h"
#include    "interrupt.h"


// prinbee
//
#include    <prinbee/network/binary_server.h>


// eventdispatcher
//
#include    <eventdispatcher/message.h>



namespace prinbee_daemon
{



constexpr int const             PROXY_BINARY_PORT = 4010;
constexpr int const             NODE_BINARY_PORT = 4011;


class prinbeed
{
public:
    typedef std::shared_ptr<prinbeed>       pointer_t;

                                prinbeed(int argc, char * argv[]);
                                prinbeed(prinbeed const & rhs) = delete;
    virtual                     ~prinbeed();

    prinbeed &                  operator = (prinbeed const & rhs) = delete;

    void                        add_connections();
    void                        set_fluid_settings_ready();
    bool                        is_ipwall_installed() const;
    void                        set_ipwall_status(bool status);
    void                        set_clock_status(bool status);
    void                        lock_status_changed();
    void                        register_prinbee_daemon(ed::message & msg);
    int                         run();
    void                        start_binary_connection();
    void                        send_our_status(ed::message * msg);
    void                        stop(bool quitting);

private:
    void                        check_ipwall_status();

    advgetopt::getopt                       f_opts;

    ed::communicator::pointer_t             f_communicator = ed::communicator::pointer_t();
    messenger::pointer_t                    f_messenger = messenger::pointer_t();
    interrupt::pointer_t                    f_interrupt = interrupt::pointer_t();
    prinbee::binary_server::pointer_t       f_proxy_listener = prinbee::binary_server::pointer_t();
    prinbee::binary_server::pointer_t       f_node_listener = prinbee::binary_server::pointer_t();
    std::string                             f_proxy_address = std::string();
    std::string                             f_node_address = std::string();

    bool                                    f_fluid_settings_ready = false;
    bool                                    f_ipwall_is_installed = false;
    bool                                    f_ipwall_is_up = false;
    bool                                    f_stable_clock = false;
    bool                                    f_lock_ready = false;
};



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
