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
#include    "prinbee/network/prinbee_connection.h"

#include    "prinbee/exception.h"
#include    "prinbee/names.h"


// snaplogger
//
#include    <snaplogger/message.h>


// eventdispatcher
//
#include    <eventdispatcher/dispatcher.h>
#include    <eventdispatcher/dispatcher_support.h>
#include    <eventdispatcher/names.h>


// communicatord
//
#include    <communicatord/names.h>


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



/** \brief Initialize a Prinbee connection to the Prinbee proxy.
 *
 * This class is expected to be used by all the Prinbee clients that want to
 * connect to the Prinbee system. The connection gives us information about
 * the Prinbee status and the ability to read and write data to the database.
 *
 * \param[in] opts  Command line options to tweak the connection settings.
 */
prinbee_connection::prinbee_connection(advgetopt::getopt & opts)
    : f_opts(opts)
{
throw logic_error("this object is for clients, however, the message to be sent (PRINBEE_GET_STATUS) needs to go to the local proxy if there isn't a prinbeed running on that computer.");
}


prinbee_connection::~prinbee_connection()
{
}


void prinbee_connection::add_prinbee_commands()
{
    ed::dispatcher_support * ds(dynamic_cast<ed::dispatcher_support *>(this));
    if(ds == nullptr)
    {
        throw logic_error("prinbee_connection::add_prinbee_commands() must be called with a connection that derives from the dispatcher_support class.");
    }
    ed::dispatcher::pointer_t dispatcher(ds->get_dispatcher());
    if(dispatcher == nullptr)
    {
        throw logic_error("the prinbee_connection::add_prinbee_commands() must be called after you setup your dispatcher (set_dispatcher() was not yet called).");
    }
    dispatcher->add_matches({
            DISPATCHER_MATCH(g_name_prinbee_cmd_prinbee_current_status, &prinbee_connection::msg_prinbee_current_status),
            ed::define_match(
                  ed::Expression(communicatord::g_name_communicatord_cmd_status)
                , ed::Callback(std::bind(&prinbee_connection::msg_status, this, std::placeholders::_1))
                , ed::MatchFunc(&ed::one_to_one_callback_match)
                , ed::Priority(ed::dispatcher_match::DISPATCHER_MATCH_CALLBACK_PRIORITY)
            ),
            ::ed::define_match(
                  ::ed::Expression(ed::g_name_ed_cmd_ready)
                , ::ed::Callback(std::bind(&prinbee_connection::msg_ready, this, std::placeholders::_1))
                , ed::MatchFunc(&ed::one_to_one_callback_match)
                , ed::Priority(ed::dispatcher_match::DISPATCHER_MATCH_CALLBACK_PRIORITY)
            ),
        });
}


void prinbee_connection::msg_prinbee_current_status(ed::message & msg)
{
    std::string const state(msg.get_parameter(communicatord::g_name_communicatord_param_status));
    f_prinbee_state.from_json(
              STATE_JOURNAL_APPLICATION_STATUS
            | STATE_JOURNAL_LOCAL_STATUS
            | STATE_JOURNAL_REMOTE_STATUS
            | STATE_PROXY_STATUS
            | STATE_CLUSTER_STATUS
            | STATE_DAEMONS_STATUS
            , state);
    f_prinbee_state.signal_state_changed();
}


void prinbee_connection::msg_status(ed::message & msg)
{
    if(!msg.has_parameter(communicatord::g_name_communicatord_param_status)
    || !msg.has_parameter(communicatord::g_name_communicatord_param_service))
    {
        return;
    }

    std::string const service(msg.get_parameter(communicatord::g_name_communicatord_param_service));
    if(service == g_name_prinbee_service_prinbee)
    {
        // in this case, if the service goes UP, we ignore the message because
        // we will soon receive the PRINBEE_CURRENT_STATUS message; in all other
        // cases we make sure that the status gets checked
        //
        std::string const status(msg.get_parameter(communicatord::g_name_communicatord_param_status));
        if(status != communicatord::g_name_communicatord_value_up)
        {
            // the connection is down; so we should reset the state
            // to all unknown?
            //
            f_prinbee_state.reset();
            f_prinbee_state.signal_state_changed();
        }
    }
}


void prinbee_connection::msg_ready(ed::message & msg)
{
    ed::connection_with_send_message * c(dynamic_cast<ed::connection_with_send_message *>(this));
    if(c == nullptr)
    {
        throw logic_error("the prinbee_connection class must also represent a connection_with_send_message."); // LCOV_EXCL_LINE
    }

    // send a PRINBEE_GET_STATUS query message to get the current database
    // status
    //
    ed::message prinbee_get_status;
    prinbee_get_status.reply_to(msg);
    prinbee_get_status.set_command(g_name_prinbee_cmd_prinbee_get_status);
    prinbee_get_status.add_parameter(
              communicatord::g_name_communicatord_param_cache
            , communicatord::g_name_communicatord_value_no);
    c->send_message(prinbee_get_status);
}



} // namespace prinbee
// vim: ts=4 sw=4 et
