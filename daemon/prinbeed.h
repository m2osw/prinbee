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
#include    "ping_pong_timer.h"
#include    "worker_pool.h"


// prinbee
//
#include    <prinbee/database/context_manager.h>
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
                                register_connection(
                                      ed::connection::pointer_t c
                                    , connection_type_t t);
    //connection_reference::pointer_t
    //                            find_connection(std::string const & name);
    connection_reference::pointer_t
                                find_connection_reference(ed::connection::pointer_t c);
    int                         run();
    void                        start_binary_connection();
    void                        send_our_status(ed::message * msg);
    void                        stop(bool quitting);
    void                        send_message(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    void                        send_pings();

    bool                        msg_error(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_ping(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_pong(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_process_payload(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    void                        push_payload(payload_t::pointer_t payload);

    bool                        register_client(payload_t::pointer_t payload);
    bool                        acknowledge(payload_t::pointer_t payload);
    bool                        list_contexts(payload_t::pointer_t payload);
    bool                        get_context(payload_t::pointer_t payload);
    bool                        set_context(payload_t::pointer_t payload);

private:
    void                        check_ipwall_status();
    void                        connect_to_node(
                                      addr::addr const & a
                                    , std::string const & name);
    void                        obtain_cluster_lock(
                                      payload_t::pointer_t payload
                                    , std::string const & lock_name
                                    , cluck::timeout_t timeout = cluck::timeout_t{ 60, 0 }); // 1 min. by default
    void                        release_cluster_lock(payload_t::pointer_t payload);
    bool                        process_obtained_lock(cluck::cluck * c, payload_t::pointer_t payload);
    bool                        process_failed_lock(cluck::cluck * c , payload_t::pointer_t payload);
    void                        expect_acknowledgment(
                                      payload_t::pointer_t payload
                                    , prinbee::binary_message::pointer_t msg);
    void                        send_acknowledgment(payload_t::pointer_t payload, std::uint32_t phase);

    advgetopt::getopt                       f_opts;
    snapdev::timespec_ex const              f_start_date;
    cppthread::mutex                        f_mutex = cppthread::mutex();

    ed::communicator::pointer_t             f_communicator = ed::communicator::pointer_t();
    messenger::pointer_t                    f_messenger = messenger::pointer_t();
    std::string                             f_cluster_name = std::string();
    std::string                             f_node_name = std::string();
    interrupt::pointer_t                    f_interrupt = interrupt::pointer_t();
    ping_pong_timer::pointer_t              f_ping_pong_timer = ping_pong_timer::pointer_t();
    prinbee::binary_server::pointer_t       f_node_listener = prinbee::binary_server::pointer_t();
    prinbee::binary_server::pointer_t       f_proxy_listener = prinbee::binary_server::pointer_t();
    prinbee::binary_server::pointer_t       f_direct_listener = prinbee::binary_server::pointer_t();
    std::string                             f_node_address = std::string();
    std::string                             f_proxy_address = std::string();
    std::string                             f_direct_address = std::string();
    connection_reference::list_t            f_connection_references = connection_reference::list_t();
    versiontheca::decimal::pointer_t        f_protocol_trait = std::make_shared<versiontheca::decimal>();
    versiontheca::versiontheca::pointer_t   f_protocol_version = std::make_shared<versiontheca::versiontheca>(f_protocol_trait, prinbee::g_name_prinbee_protocol_version_node);
    worker_pool::pointer_t                  f_worker_pool = worker_pool::pointer_t();
    prinbee::context_manager::pointer_t     f_context_manager = prinbee::context_manager::pointer_t();
    payload_t::map_t                        f_expected_acknowledgment = payload_t::map_t();
                    

    bool                                    f_fluid_settings_ready = false;
    bool                                    f_ipwall_is_installed = false;
    bool                                    f_ipwall_is_up = false;
    bool                                    f_stable_clock = false;
    bool                                    f_lock_ready = false;
};



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
