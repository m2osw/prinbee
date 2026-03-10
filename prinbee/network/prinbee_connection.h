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
 * \brief A connection used by clients to connect to Prinbee.
 *
 * This file defines the prinbee_connection class which is expected to be
 * used by clients to communicate with Prinbee. It keeps track of the
 * state of the Prinbee cluster.
 *
 * The class also automatically creates a binary connection to the servers
 * (proxy or daemon as may be required) whenever it receives the message
 * with the necessary information to connect.
 */

// self
//
#include    <prinbee/state.h>
#include    <prinbee/network/binary_client.h>



// eventdispatcher
//
//#include    <eventdispatcher/communicator.h>
//#include    <eventdispatcher/message.h>


// snapdev
//
#include    <snapdev/string_literal_length.h>


// fluid-settings
//
#include    <fluid-settings/fluid_settings_connection.h>


// C++
//
#include    <functional>
#include    <memory>



namespace prinbee
{



constexpr char const                    g_proxy_state_unknown[] = "unknown";


class prinbee_connection
    : public fluid_settings::fluid_settings_connection
{
public:
    typedef std::shared_ptr<prinbee_connection> pointer_t;
    typedef std::function<void(state status)>   status_change_t;

                                prinbee_connection(
                                      advgetopt::getopt & opts
                                    , std::string const & service_name);
                                prinbee_connection(prinbee_connection const & rhs) = delete;
    virtual                     ~prinbee_connection();

    prinbee_connection &        operator = (prinbee_connection const & rhs) = delete;

    // connection_with_send_message implementation
    //
    virtual void                ready(ed::message & msg) override;

    // fluid_settings_connection implementation
    //
    virtual void                service_status(
                                      std::string const & server
                                    , std::string const & service
                                    , std::string const & status) override;
    virtual void                fluid_settings_changed(
                                      fluid_settings::fluid_settings_status_t status
                                    , std::string const & name
                                    , std::string const & value) override;

    // new callbacks
    //
    virtual void                finish_initialization();
    virtual void                process_proxy_status();

    std::string                 get_proxy_status() const;
    snapdev::timespec_ex        get_last_ping() const;
    bool                        is_proxy_ready() const;
    bool                        is_proxy_registered() const;
    addr::addr const &          get_address() const;
    bool                        has_address() const;
    bool                        is_ping_pong_timer_on() const;

private:
    void                        msg_prinbee_proxy_current_status(ed::message & msg);

    void                        set_proxy_status_and_address(
                                      std::string const & status
                                    , addr::addr const & address);
    void                        start_binary_connection();
    void                        setup_ping_pong_interval();

    ed::communicator::pointer_t f_communicator = ed::communicator::pointer_t();
    std::string                 f_proxy_status = std::string(g_proxy_state_unknown, snapdev::string_literal_length(g_proxy_state_unknown));
    addr::addr                  f_address = addr::addr();
    proxy_connection::pointer_t f_proxy_connection = proxy_connection::pointer_t();
    bool                        f_fluid_settings_ready = false;

    // the state is mainly maintained by the binary connection which is
    // managed by this prinbee_connection messenger
    //
    state                       f_prinbee_state = state();
};


snapdev::timespec_ex const &    proxy_ping_pong_off();



} // namespace prinbee
// vim: ts=4 sw=4 et
