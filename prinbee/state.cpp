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


/** \file
 * \brief Lexer of the Prinbee Query Language.
 *
 * The Prinbee Query Language (PBQL) is an SQL-like language. This file
 * transforms the input data in tokens that the parser can then use to
 * create statements.
 *
 * The lexer supports tokens that include keywords (SELECT), identifiers
 * (column name), numbers (integers, floating points), operators (for
 * expressions; +, -, *, /, etc.).
 */

// self
//
#include    "prinbee/state.h"

#include    "prinbee/exception.h"


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
//#include    <snapdev/escape_special_regex_characters.h>
//#include    <snapdev/floating_point_to_string.h>
//#include    <snapdev/not_reached.h>
//#include    <snapdev/not_used.h>
//#include    <snapdev/string_replace_many.h>
//#include    <snapdev/safe_variable.h>
//#include    <snapdev/to_lower.h>
//#include    <snapdev/to_upper.h>
//#include    <snapdev/trim_string.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



std::string const       g_json_field_prinbee = "prinbee";
std::string const       g_json_field_last_updated = "last_updated";
std::string const       g_json_field_journal_application_status = "journal_application_status";
std::string const       g_json_field_journal_local_status = "journal_local_status";
std::string const       g_json_field_journal_remote_status = "journal_remote_status";
std::string const       g_json_field_proxy_status = "proxy_status";
std::string const       g_json_field_cluster_status = "cluster_status";
std::string const       g_json_field_daemons = "daemons";
std::string const       g_json_field_name = "name";
std::string const       g_json_field_daemon_status = "daemon_status";
std::string const       g_json_field_disk_used = "disk_used";
std::string const       g_json_value_disk_use_unknown = "unknown";


struct daemon_status_name_t
{
    daemon_status_t     f_status = daemon_status_t::DAEMON_STATUS_UNKNOWN;
    char const *        f_name = nullptr;
};


daemon_status_name_t g_daemon_status_names[] =
{
    { daemon_status_t::DAEMON_STATUS_UNKNOWN, "unknown" },
    { daemon_status_t::DAEMON_STATUS_HEALTHY, "healthy" },
    { daemon_status_t::DAEMON_STATUS_FULL,    "full"    },
    { daemon_status_t::DAEMON_STATUS_ERROR,   "error"   },
};


char const * to_string(daemon_status_t status)
{
    std::size_t const max(std::size(g_daemon_status_names));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        if(status == g_daemon_status_names[idx].f_status)
        {
            return g_daemon_status_names[idx].f_name;
        }
    }

    return "";
}


daemon_status_t string_to_daemon_status(char const * name)
{
    std::size_t const max(std::size(g_daemon_status_names));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        if(strcmp(name, g_daemon_status_names[idx].f_name) == 0)
        {
            return g_daemon_status_names[idx].f_status;
        }
    }

    return daemon_status_t::DAEMON_STATUS_UNKNOWN;
}



struct journal_status_name_t
{
    journal_status_t    f_status = journal_status_t::JOURNAL_STATUS_UNKNOWN;
    char const *        f_name = nullptr;
};


journal_status_name_t g_journal_status_names[] =
{
    { journal_status_t::JOURNAL_STATUS_UNKNOWN, "unknown" },
    { journal_status_t::JOURNAL_STATUS_OFF,     "off"     },
    { journal_status_t::JOURNAL_STATUS_HEALTHY, "healthy" },
    { journal_status_t::JOURNAL_STATUS_FULL,    "full"    },
    { journal_status_t::JOURNAL_STATUS_ERROR,   "error"   },
};


char const * to_string(journal_status_t status)
{
    std::size_t const max(std::size(g_journal_status_names));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        if(status == g_journal_status_names[idx].f_status)
        {
            return g_journal_status_names[idx].f_name;
        }
    }

    return "";
}


journal_status_t string_to_journal_status(char const * name)
{
    std::size_t const max(std::size(g_journal_status_names));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        if(strcmp(name, g_journal_status_names[idx].f_name) == 0)
        {
            return g_journal_status_names[idx].f_status;
        }
    }

    return journal_status_t::JOURNAL_STATUS_UNKNOWN;
}



struct proxy_status_name_t
{
    proxy_status_t      f_status = proxy_status_t::PROXY_STATUS_UNKNOWN;
    char const *        f_name = nullptr;
};


proxy_status_name_t g_proxy_status_names[] =
{
    { proxy_status_t::PROXY_STATUS_UNKNOWN,       "unknown"       },
    { proxy_status_t::PROXY_STATUS_NOT_CONNECTED, "not-connected" },
    { proxy_status_t::PROXY_STATUS_CONNECTING,    "connecting"    },
    { proxy_status_t::PROXY_STATUS_CONNECTED,     "connected"     },
    { proxy_status_t::PROXY_STATUS_NO_FIREWALL,   "no-firewall"   },
    { proxy_status_t::PROXY_STATUS_DAEMON,        "daemon"        },
    { proxy_status_t::PROXY_STATUS_CLUSTER,       "cluster"       },
    { proxy_status_t::PROXY_STATUS_COMPLETE,      "complete"      },
    { proxy_status_t::PROXY_STATUS_ERROR,         "error"         },
};


char const * to_string(proxy_status_t status)
{
    std::size_t const max(std::size(g_proxy_status_names));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        if(status == g_proxy_status_names[idx].f_status)
        {
            return g_proxy_status_names[idx].f_name;
        }
    }

    return "";
}


proxy_status_t string_to_proxy_status(char const * name)
{
    std::size_t const max(std::size(g_proxy_status_names));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        if(strcmp(name, g_proxy_status_names[idx].f_name) == 0)
        {
            return g_proxy_status_names[idx].f_status;
        }
    }

    return proxy_status_t::PROXY_STATUS_UNKNOWN;
}



struct cluster_status_name_t
{
    cluster_status_t    f_status = cluster_status_t::CLUSTER_STATUS_UNKNOWN;
    char const *        f_name = nullptr;
};


cluster_status_name_t g_cluster_status_names[] =
{
    { cluster_status_t::CLUSTER_STATUS_UNKNOWN,       "unknown"       },
    { cluster_status_t::CLUSTER_STATUS_NOT_CONNECTED, "not-connected" },
    { cluster_status_t::CLUSTER_STATUS_CONNECTED,     "connected"     },
    { cluster_status_t::CLUSTER_STATUS_QUORUM,        "quorum"        },
    { cluster_status_t::CLUSTER_STATUS_COMPLETE,      "complete"      },
};


char const * to_string(cluster_status_t status)
{
    std::size_t const max(std::size(g_cluster_status_names));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        if(status == g_cluster_status_names[idx].f_status)
        {
            return g_cluster_status_names[idx].f_name;
        }
    }

    return "";
}


cluster_status_t string_to_cluster_status(char const * name)
{
    std::size_t const max(std::size(g_cluster_status_names));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        if(strcmp(name, g_cluster_status_names[idx].f_name) == 0)
        {
            return g_cluster_status_names[idx].f_status;
        }
    }

    return cluster_status_t::CLUSTER_STATUS_UNKNOWN;
}



void cluster_detail::set_name(std::string const & name)
{
    f_name = name;
}


std::string const & cluster_detail::get_name() const
{
    return f_name;
}


void cluster_detail::set_daemon_status(daemon_status_t status)
{
    f_daemon_status = status;
}


daemon_status_t cluster_detail::get_daemon_status() const
{
    return f_daemon_status;
}


void cluster_detail::set_disk_used(disk_percent_t percent)
{
    f_disk_used = percent;
}


disk_percent_t cluster_detail::get_disk_used() const
{
    return f_disk_used;
}


void cluster_detail::to_json(as2js::json::json_value_ref obj) const
{
    obj[g_json_field_name] = f_name;
    obj[g_json_field_daemon_status] = to_string(f_daemon_status);

    if(f_disk_used == DISK_PERCENT_UNKNOWN)
    {
        obj[g_json_field_disk_used] = g_json_value_disk_use_unknown;
    }
    else
    {
        obj[g_json_field_disk_used] = as2js::integer(f_disk_used);
    }
}


void cluster_detail::from_json(as2js::json::json_value::pointer_t obj)
{
    if(obj->get_type() != as2js::json::json_value::type_t::JSON_TYPE_OBJECT)
    {
        return;
    }
    as2js::json::json_value::object_t const & values(obj->get_object());

    {
        auto const it(values.find(g_json_field_name));
        if(it != values.end()
        && it->second->get_type() == as2js::json::json_value::type_t::JSON_TYPE_STRING)
        {
            f_name = it->second->get_string();
        }
    }

    {
        auto const it(values.find(g_json_field_daemon_status));
        if(it != values.end()
        && it->second->get_type() == as2js::json::json_value::type_t::JSON_TYPE_STRING)
        {
            f_daemon_status = string_to_daemon_status(it->second->get_string().c_str());
        }
    }

    {
        auto const it(values.find(g_json_field_disk_used));
        if(it != values.end())
        {
            if(it->second->get_type() == as2js::json::json_value::type_t::JSON_TYPE_STRING)
            {
                if(it->second->get_string() == g_json_value_disk_use_unknown)
                {
                    f_disk_used = DISK_PERCENT_UNKNOWN;
                }
            }
            else if(it->second->get_type() == as2js::json::json_value::type_t::JSON_TYPE_INTEGER)
            {
                f_disk_used = it->second->get_integer().get();
            }
        }
    }
}


bool cluster_detail::operator == (cluster_detail const & rhs) const
{
    return f_name == rhs.f_name
        && f_daemon_status == rhs.f_daemon_status
        && f_disk_used == rhs.f_disk_used;
}





void state::reset()
{
    set_application_journal_status(journal_status_t::JOURNAL_STATUS_UNKNOWN);
    set_local_journal_status(journal_status_t::JOURNAL_STATUS_UNKNOWN);
    set_remote_journal_status(journal_status_t::JOURNAL_STATUS_UNKNOWN);
    set_proxy_status(proxy_status_t::PROXY_STATUS_UNKNOWN);
    set_cluster_status(cluster_status_t::CLUSTER_STATUS_UNKNOWN);
    if(!f_cluster_details.empty())
    {
        f_cluster_details.clear();
        set_last_updated();
    }
}


void state::set_application_journal_status(journal_status_t status)
{
    if(f_journal_application_status != status)
    {
        f_journal_application_status = status;
        set_last_updated();
    }
}


journal_status_t state::get_application_journal_status() const
{
    return f_journal_application_status;
}


void state::set_local_journal_status(journal_status_t status)
{
    if(f_journal_local_status != status)
    {
        f_journal_local_status = status;
        set_last_updated();
    }
}


journal_status_t state::get_local_journal_status() const
{
    return f_journal_local_status;
}


void state::set_remote_journal_status(journal_status_t status)
{
    if(f_journal_remote_status != status)
    {
        f_journal_remote_status = status;
        set_last_updated();
    }
}


journal_status_t state::get_remote_journal_status() const
{
    return f_journal_remote_status;
}


void state::set_proxy_status(proxy_status_t status)
{
    if(f_proxy_status != status)
    {
        f_proxy_status = status;
        set_last_updated();
    }
}


proxy_status_t state::get_proxy_status() const
{
    return f_proxy_status;
}


void state::set_cluster_status(cluster_status_t status)
{
    if(f_cluster_status != status)
    {
        f_cluster_status = status;
        set_last_updated();
    }
}


cluster_status_t state::get_cluster_status() const
{
    return f_cluster_status;
}


void state::set_daemon_status(cluster_detail::pointer_t status)
{
    auto const it(f_cluster_details.find(status->get_name()));
    if(it == f_cluster_details.end()
    || *it->second != *status)
    {
        f_cluster_details[status->get_name()] = status;
        set_last_updated();
    }
}


cluster_detail::pointer_t state::get_daemon_status(std::string const & name)
{
    auto const it(f_cluster_details.find(name));
    if(it == f_cluster_details.end())
    {
        return cluster_detail::pointer_t();
    }
    return it->second;
}


cluster_detail::map_t const & state::get_cluster_details() const
{
    return f_cluster_details;
}


/** \brief Convert the state in a JSON string.
 *
 * This function converts the state to a JSON string. This is useful to
 * send it over to another system using a communicatord message. It
 * could also directly be sent to a browser.
 *
 * There are different levels of states. Clients view all states as one.
 * The proxy only includes this very proxy state and the state of the
 * backend servers (prinbee daemons). A Prinbee daemon only shares its
 * own state (although it knows about the other daemon states).
 *
 * \param[in] states  The list of states to convert.
 *
 * \return The state as a JSON string.
 */
std::string state::to_json(state_t states) const
{
    as2js::json json;

    as2js::json::json_value_ref root(json[g_json_field_prinbee]);

    root[g_json_field_last_updated] = f_last_updated.to_string();

    if((states & STATE_JOURNAL_APPLICATION_STATUS) != 0)
    {
        root[g_json_field_journal_application_status] = to_string(f_journal_application_status);
    }

    if((states & STATE_JOURNAL_LOCAL_STATUS) != 0)
    {
        root[g_json_field_journal_local_status] = to_string(f_journal_local_status);
    }

    if((states & STATE_JOURNAL_REMOTE_STATUS) != 0)
    {
        root[g_json_field_journal_remote_status] = to_string(f_journal_remote_status);
    }

    if((states & STATE_PROXY_STATUS) != 0)
    {
        root[g_json_field_proxy_status] = to_string(f_proxy_status);
    }

    if((states & STATE_CLUSTER_STATUS) != 0)
    {
        root[g_json_field_cluster_status] = to_string(f_cluster_status);
    }

    if((states & STATE_DAEMONS_STATUS) != 0)
    {
        as2js::json::json_value_ref e(json[g_json_field_daemons]);
        ssize_t idx(0);
        for(auto const & d : f_cluster_details)
        {
            d.second->to_json(e[idx]);
            ++idx;
        }
    }

    return json.get_value()->to_string();
}


/** \brief Convert a JSON string in a Prinbee state.
 *
 * This function parses the input JSON and saves the state in this class.
 *
 * \param[in] states  The list of states to convert, other states are ignored.
 * \param[in] json  The JSON string to convert.
 */
void state::from_json(state_t states, std::string const & json)
{
    as2js::input_stream<std::stringstream>::pointer_t in(std::make_shared<as2js::input_stream<std::stringstream>>());
    *in << json;

    as2js::json::pointer_t load_json(std::make_shared<as2js::json>());
    as2js::json::json_value::pointer_t root(load_json->parse(in));

    if(root->get_type() != as2js::json::json_value::type_t::JSON_TYPE_OBJECT)
    {
        // TODO: add error
        return;
    }

    as2js::json::json_value::object_t const & values(root->get_object());

    {
        auto const it(values.find(g_json_field_last_updated));
        if(it != values.end()
        && it->second->get_type() == as2js::json::json_value::type_t::JSON_TYPE_STRING)
        {
            f_last_updated.from_string(it->second->get_string(), std::string());
        }
    }

    if((states & STATE_JOURNAL_APPLICATION_STATUS) != 0)
    {
        auto const it(values.find(g_json_field_journal_application_status));
        if(it != values.end()
        && it->second->get_type() == as2js::json::json_value::type_t::JSON_TYPE_STRING)
        {
            f_journal_application_status = string_to_journal_status(it->second->get_string().c_str());
        }
    }

    if((states & STATE_JOURNAL_LOCAL_STATUS) != 0)
    {
        auto const it(values.find(g_json_field_journal_local_status));
        if(it != values.end()
        && it->second->get_type() == as2js::json::json_value::type_t::JSON_TYPE_STRING)
        {
            f_journal_local_status = string_to_journal_status(it->second->get_string().c_str());
        }
    }

    if((states & STATE_JOURNAL_REMOTE_STATUS) != 0)
    {
        auto const it(values.find(g_json_field_journal_remote_status));
        if(it != values.end()
        && it->second->get_type() == as2js::json::json_value::type_t::JSON_TYPE_STRING)
        {
            f_journal_remote_status = string_to_journal_status(it->second->get_string().c_str());
        }
    }

    if((states & STATE_PROXY_STATUS) != 0)
    {
        auto const it(values.find(g_json_field_proxy_status));
        if(it != values.end()
        && it->second->get_type() == as2js::json::json_value::type_t::JSON_TYPE_STRING)
        {
            f_proxy_status = string_to_proxy_status(it->second->get_string().c_str());
        }
    }

    if((states & STATE_CLUSTER_STATUS) != 0)
    {
        auto const it(values.find(g_json_field_cluster_status));
        if(it != values.end()
        && it->second->get_type() == as2js::json::json_value::type_t::JSON_TYPE_STRING)
        {
            f_cluster_status = string_to_cluster_status(it->second->get_string().c_str());
        }
    }

    if((states & STATE_DAEMONS_STATUS) != 0)
    {
        auto const it(values.find(g_json_field_daemons));
        if(it != values.end()
        && it->second->get_type() == as2js::json::json_value::type_t::JSON_TYPE_ARRAY)
        {
            as2js::json::json_value::array_t e(it->second->get_array());
            for(auto const & obj : e)
            {
                cluster_detail::pointer_t d(std::make_shared<cluster_detail>());
                d->from_json(obj);
                f_cluster_details[d->get_name()] = d;
            }
        }
    }
}


state::callback_manager_t & state::get_callback_manager()
{
    return f_state_callback;
}


void state::set_last_updated()
{
    f_last_updated = snapdev::now();
    f_state_changed = true;
}


void state::signal_state_changed()
{
    if(f_state_changed)
    {
        f_state_changed = false;
        f_state_callback.call(std::ref(*this));
    }
}



} // namespace prinbee
// vim: ts=4 sw=4 et
