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
#include    "connection_reference.h"
#include    "worker_pool.h"


// prinbee
//
#include    <prinbee/names.h>
#include    <prinbee/network/binary_server.h>


// eventdispatcher
//
#include    <eventdispatcher/message.h>



namespace prinbee_daemon
{



constexpr int const             DIRECT_BINARY_PORT = 4012;
constexpr int const             NODE_BINARY_PORT = 4011;
constexpr int const             PROXY_BINARY_PORT = 4010;


class prinbeed
{
public:
    typedef std::shared_ptr<prinbeed>       pointer_t;

                                prinbeed(int argc, char * argv[]);
                                prinbeed(prinbeed const & rhs) = delete;
    virtual                     ~prinbeed();

    prinbeed &                  operator = (prinbeed const & rhs) = delete;

    void                        finish_initialization();
    void                        set_fluid_settings_ready();
    bool                        is_ipwall_installed() const;
    void                        set_ipwall_status(bool status);
    void                        set_clock_status(bool status);
    void                        lock_status_changed();
    void                        register_prinbee_daemon(ed::message & msg);
    connection_reference::pointer_t
                                register_connection(ed::connection::pointer_t c);
    connection_reference::pointer_t
                                find_connection(std::string const & name);
    int                         run();
    void                        start_binary_connection();
    void                        send_our_status(ed::message * msg);
    void                        stop(bool quitting);
    void                        send_message(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);

    bool                        msg_error(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_ping(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_pong(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_register(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_process_workload(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);

    void                        list_context(payload_t & payload);
    void                        get_context(payload_t & payload);
    void                        set_context(payload_t & payload);

private:
    void                        check_ipwall_status();
    void                        connect_to_node(
                                      addr::addr const & a
                                    , std::string const & name);

    advgetopt::getopt                       f_opts;

    ed::communicator::pointer_t             f_communicator = ed::communicator::pointer_t();
    messenger::pointer_t                    f_messenger = messenger::pointer_t();
    interrupt::pointer_t                    f_interrupt = interrupt::pointer_t();
    prinbee::binary_server::pointer_t       f_node_listener = prinbee::binary_server::pointer_t();
    prinbee::binary_server::pointer_t       f_proxy_listener = prinbee::binary_server::pointer_t();
    prinbee::binary_server::pointer_t       f_direct_listener = prinbee::binary_server::pointer_t();
    std::string                             f_node_address = std::string();
    std::string                             f_proxy_address = std::string();
    std::string                             f_direct_address = std::string();
    connection_reference::map_t             f_connection_reference = connection_reference::map_t();
    versiontheca::decimal::pointer_t        f_protocol_trait = std::make_shared<versiontheca::decimal>();
    versiontheca::versiontheca::pointer_t   f_protocol_version = std::make_shared<versiontheca::versiontheca>(f_protocol_trait, prinbee::g_name_prinbee_protocol_version_node);
    worker_pool::pointer_t                  f_worker_pool = worker_pool::pointer_t();

    bool                                    f_fluid_settings_ready = false;
    bool                                    f_ipwall_is_installed = false;
    bool                                    f_ipwall_is_up = false;
    bool                                    f_stable_clock = false;
    bool                                    f_lock_ready = false;
};



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
