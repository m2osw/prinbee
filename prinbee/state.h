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
//#include    <prinbee/pbql/location.h>
//#include    <prinbee/bigint/uint512.h>



// as2js
//
#include    <as2js/json.h>


// snapdev
//
#include    <snapdev/callback_manager.h>
#include    <snapdev/timespec_ex.h>


// C++
//
#include    <memory>
#include    <map>



namespace prinbee
{



enum class daemon_status_t
{
    DAEMON_STATUS_UNKNOWN,
    DAEMON_STATUS_HEALTHY,
    DAEMON_STATUS_FULL,
    DAEMON_STATUS_ERROR,
};

extern char const *         to_string(daemon_status_t status);
extern daemon_status_t      string_to_daemon_status(char const * name);


typedef std::uint8_t            disk_percent_t;

constexpr disk_percent_t const  DISK_PERCENT_UNKNOWN = 255;
constexpr disk_percent_t const  DISK_PERCENT_EMPTY = 0;
constexpr disk_percent_t const  DISK_PERCENT_FULL = 100;



// status of a one specific Prinbee Daemon
class cluster_detail
{
public:
    typedef std::shared_ptr<cluster_detail>     pointer_t;
    typedef std::map<std::string, pointer_t>    map_t;

    void                    set_name(std::string const & name);
    std::string const &     get_name() const;

    void                    set_daemon_status(daemon_status_t status);
    daemon_status_t         get_daemon_status() const;

    void                    set_disk_used(disk_percent_t percent);
    disk_percent_t          get_disk_used() const;

    void                    to_json(as2js::json::json_value_ref obj) const;
    void                    from_json(as2js::json::json_value::pointer_t obj);

    bool                    operator == (cluster_detail const & rhs) const;

private:
    std::string             f_name = std::string();
    daemon_status_t         f_daemon_status = daemon_status_t::DAEMON_STATUS_UNKNOWN;
    disk_percent_t          f_disk_used = DISK_PERCENT_UNKNOWN; // percent use 0 to 100 -- 255 represents "unknown"
};



typedef std::uint32_t           state_t;

constexpr state_t const         STATE_JOURNAL_APPLICATION_STATUS = 0x00000001;  // status of the journal in your application
constexpr state_t const         STATE_JOURNAL_LOCAL_STATUS       = 0x00000002;  // status of the journal in your proxy (proxy running locally)
constexpr state_t const         STATE_JOURNAL_REMOTE_STATUS      = 0x00000004;  // status of the remote journals (daemon side)
constexpr state_t const         STATE_PROXY_STATUS               = 0x00000008;  // status of the proxy
constexpr state_t const         STATE_CLUSTER_STATUS             = 0x00000010;  // general status of the cluster (connection wise)
constexpr state_t const         STATE_DAEMONS_STATUS             = 0x00000020;  // detailed status of the cluster (including each known daemon)


enum class journal_status_t
{
    JOURNAL_STATUS_UNKNOWN,
    JOURNAL_STATUS_OFF,
    JOURNAL_STATUS_HEALTHY,
    JOURNAL_STATUS_FULL,
    JOURNAL_STATUS_ERROR,
};

extern char const *         to_string(journal_status_t status);
extern journal_status_t     string_to_journal_status(char const * name);


enum class proxy_status_t
{
    PROXY_STATUS_UNKNOWN,       // default state
    PROXY_STATUS_NOT_CONNECTED, // not yet tried to connect to proxy
    PROXY_STATUS_CONNECTING,    // trying to connect to proxy
    PROXY_STATUS_CONNECTED,     // your application is connected to the proxy
    PROXY_STATUS_NO_FIREWALL,   // application is connected, but proxy cannot detect a firewall
    PROXY_STATUS_DAEMON,        // connected to the proxy which is connected to at least a daemon
    PROXY_STATUS_CLUSTER,       // connected to proxy + one daemons per replication group
    PROXY_STATUS_COMPLETE,      // connected to proxy + all daemons
    PROXY_STATUS_ERROR,         // not able to connect to proxy or proxy has errors
};

extern char const *         to_string(proxy_status_t status);
extern proxy_status_t       string_to_proxy_status(char const * name);


enum class cluster_status_t
{
    CLUSTER_STATUS_UNKNOWN,
    CLUSTER_STATUS_NOT_CONNECTED,
    CLUSTER_STATUS_CONNECTED,
    CLUSTER_STATUS_QUORUM,
    CLUSTER_STATUS_COMPLETE,
};

extern char const *         to_string(cluster_status_t status);
extern cluster_status_t     string_to_cluster_status(char const * name);



class state
{
public:
    typedef std::function<bool(state const &)>          state_callback_t;
    typedef snapdev::callback_manager<state_callback_t> callback_manager_t;

    void                    reset();
    void                    set_application_journal_status(journal_status_t status);
    journal_status_t        get_application_journal_status() const;
    void                    set_local_journal_status(journal_status_t status);
    journal_status_t        get_local_journal_status() const;
    void                    set_remote_journal_status(journal_status_t status);
    journal_status_t        get_remote_journal_status() const;
    void                    set_proxy_status(proxy_status_t status);
    proxy_status_t          get_proxy_status() const;
    void                    set_cluster_status(cluster_status_t status);
    cluster_status_t        get_cluster_status() const;
    void                    set_daemon_status(cluster_detail::pointer_t status);
    cluster_detail::pointer_t
                            get_daemon_status(std::string const & name);
    cluster_detail::map_t const &
                            get_cluster_details() const;

    std::string             to_json(state_t states) const;
    void                    from_json(state_t states, std::string const & json);

    // helper functions
    //
    bool                    can_read() const;
    bool                    can_read_any() const;
    bool                    can_write() const;
    bool                    can_write_safely() const;   // write with actual replication

    callback_manager_t &    get_callback_manager();
    void                    signal_state_changed();

private:
    void                    set_last_updated();

    snapdev::timespec_ex    f_last_updated = snapdev::timespec_ex();
    journal_status_t        f_journal_application_status = journal_status_t::JOURNAL_STATUS_UNKNOWN;
    journal_status_t        f_journal_local_status = journal_status_t::JOURNAL_STATUS_UNKNOWN;
    journal_status_t        f_journal_remote_status = journal_status_t::JOURNAL_STATUS_UNKNOWN;
    proxy_status_t          f_proxy_status = proxy_status_t::PROXY_STATUS_UNKNOWN;
    cluster_status_t        f_cluster_status = cluster_status_t::CLUSTER_STATUS_UNKNOWN;
    cluster_detail::map_t   f_cluster_details = cluster_detail::map_t();

    bool                    f_state_changed = false;
    callback_manager_t      f_state_callback = callback_manager_t();
};



} // namespace prinbee
// vim: ts=4 sw=4 et
