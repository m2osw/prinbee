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
#include    "connection_reference.h"
#include    "daemon.h"
#include    "interrupt.h"
#include    "messenger.h"
#include    "ping_pong_timer.h"


// prinbee
//
#include    <prinbee/names.h>
#include    <prinbee/network/binary_server.h>



namespace prinbee_proxy
{



enum msg_reply_t
{
    MSG_REPLY_RECEIVED,         // when we receive a message (i.e. not ACK nor ERR)
    MSG_REPLY_FAILED,           // ERR a message we sent
    MSG_REPLY_SUCCEEDED,        // ACK a message we sent
};


class proxy
{
public:
    //typedef std::shared_ptr<proxy>      pointer_t;

                                proxy(int argc, char * argv[]);
                                proxy(proxy const & rhs) = delete;
    virtual                     ~proxy();

    proxy &                     operator = (proxy const & rhs) = delete;

    void                        finish_initialization();
    void                        set_fluid_settings_ready();
    bool                        is_ipwall_installed() const;
    void                        set_ipwall_status(bool status);
    void                        set_clock_status(bool status);
    void                        register_prinbee_daemon(ed::message & msg);
    void                        register_client(prinbee::binary_server_client::pointer_t client);
    void                        client_disconnected(prinbee::binary_server_client::pointer_t client);
    connection_reference::pointer_t
                                find_connection_reference(ed::connection::pointer_t c);
    void                        daemon_disconnected(daemon::pointer_t daemon);
    //connection_reference::pointer_t
    //                            register_connection(
    //                                  ed::connection::pointer_t c
    //                                , connection_type_t t);
    //connection_reference::pointer_t
    //                            find_connection(std::string const & name);
    int                         run();
    void                        timed_out();
    void                        start_binary_connection();
    void                        send_our_status(ed::message * msg);
    void                        stop(bool quitting);
    void                        send_message(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    void                        send_pings();

    void                        msg_prinbee_current_status(ed::message & msg);

    //bool                        msg_acknowledge(
    //                                  ed::connection::pointer_t peer
    //                                , prinbee::binary_message::pointer_t msg);
    bool                        msg_error(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_ping(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_register(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_forward(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg);
    bool                        msg_process_reply(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg
                                    , msg_reply_t state);

private:
    void                        check_ipwall_status();
    void                        connect_to_daemon(
                                      addr::addr const & a
                                    , std::string const & name);
    void                        send_acknowledgment(
                                      ed::connection::pointer_t peer
                                    , prinbee::binary_message::pointer_t msg
                                    , std::uint32_t phase);

    advgetopt::getopt                       f_opts;
    snapdev::timespec_ex const              f_start_date;

    ed::communicator::pointer_t             f_communicator = ed::communicator::pointer_t();
    messenger::pointer_t                    f_messenger = messenger::pointer_t();
    std::string                             f_cluster_name = std::string();
    interrupt::pointer_t                    f_interrupt = interrupt::pointer_t();
    ping_pong_timer::pointer_t              f_ping_pong_timer = ping_pong_timer::pointer_t();
    std::string                             f_address = std::string();
    std::string                             f_user = prinbee::get_prinbee_user();
    std::string                             f_group = prinbee::get_prinbee_group();
    prinbee::binary_server::pointer_t       f_listener = prinbee::binary_server::pointer_t();
    daemon::map_t                           f_daemon_connections = daemon::map_t();
    connection_reference::map_t             f_client_connections = connection_reference::map_t();
    versiontheca::decimal::pointer_t        f_protocol_trait = std::make_shared<versiontheca::decimal>();
    versiontheca::versiontheca::pointer_t   f_protocol_version = std::make_shared<versiontheca::versiontheca>(f_protocol_trait, prinbee::g_name_prinbee_protocol_version_node);

    bool                                    f_fluid_settings_ready = false;
    bool                                    f_ipwall_is_installed = false;
    bool                                    f_ipwall_is_up = false;
    bool                                    f_stable_clock = false;
};



} // namespace prinbee_proxy
// vim: ts=4 sw=4 et
