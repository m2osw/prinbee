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

/** \file
 * \brief Messenger for the Prinbee console interface.
 *
 * The Prinbee console interface makes use of a normal messenger connection.
 * This is used to find the local Prinbee proxy and connect to it.
 */


// self
//
#include    "messenger.h"

#include    "cui.h"


// prinbee
//
#include    <prinbee/names.h>


// eventdispatcher
//
#include    <eventdispatcher/names.h>


// communicator
//
#include    <communicator/names.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_cui
{


/** \class messenger
 * \brief Handle messages from the communicatord.
 *
 * This class is an implementation of the TCP client message connection
 * so we can handle incoming and outgoing messages. We actually use the
 * fluid-settings which itself uses the communicatord connection. All
 * of the basic communication messages used by the communicatord and
 * fluid settings are handled automatically.
 */



/** \brief The messenger initialization.
 *
 * The messenger is the proxy daemon connection to the communicator server.
 *
 * It sets up its dispatcher and calls prinbeed functions whenever it
 * receives a message.
 *
 * \param[in] c  The PBQL CUI object we are listening for (the client).
 * \param[in] opts  The options received from the command line.
 */
messenger::messenger(cui * c, advgetopt::getopt & opts)
    : fluid_settings_connection(opts, "pbql_cui")
    , f_cui(c)
    , f_dispatcher(std::make_shared<ed::dispatcher>(this))
{
    set_name("messenger");
    set_dispatcher(f_dispatcher);
    add_fluid_settings_commands();
    f_dispatcher->add_matches({
            DISPATCHER_MATCH(prinbee::g_name_prinbee_cmd_prinbee_proxy_current_status, &messenger::msg_prinbee_proxy_current_status),
    });
    f_dispatcher->add_communicator_commands();

    // further dispatcher initialization
    //
#ifdef _DEBUG
    f_dispatcher->set_trace();
    f_dispatcher->set_show_matches();
#endif
}


messenger::~messenger()
{
}


/** \brief Finish handling command line options.
 *
 * This function makes sure the fluid settings and communicator daemon
 * have a chance to check the command line options and act on it.
 */
void messenger::finish_parsing()
{
    process_fluid_settings_options();
    automatic_watch_initialization();
}


/** \brief Messenger received the READY message.
 *
 * Whenever we receive the READY message, we also receive our IP address
 * as the "my_address" parameter. This gets copied in the prinbeed object.
 *
 * \param[in,out] msg  The READY message.
 */
void messenger::ready(ed::message & msg)
{
    fluid_settings_connection::ready(msg);

    // request the local proxy status
    //
    // if the proxy has been ready for a while, it won't be sending it's
    // status to us automatically
    //
    ed::message proxy_status;
    proxy_status.set_command(prinbee::g_name_prinbee_cmd_prinbee_proxy_get_status);
    proxy_status.set_service(prinbee::g_name_prinbee_service_proxy);
    proxy_status.set_server(::communicator::g_name_communicator_service_private_broadcast);
    proxy_status.add_parameter(
              ::communicator::g_name_communicator_param_cache
            , ::communicator::g_name_communicator_value_no);
    send_message(proxy_status);

    // the following call is for completeness, but since we just sent the
    // PRINBEE_PROXY_GET_STATUS it won't work since we need that status first
    //
    f_cui->start_binary_connection();
}


/** \brief Handle the CLOCK_STABLE message.
 *
 * This function gets called whenever the daemon receives the
 * CLOCK_STABLE message.
 *
 * \param[in,out] msg  The CLOCK_STABLE message.
 */
void messenger::msg_prinbee_proxy_current_status(ed::message & msg)
{
    f_cui->msg_prinbee_proxy_current_status(msg);
}


/** \brief Let the server know STOP or QUITTING was sent to us.
 *
 * This STOP and QUITTING messages are currently managed through this
 * overridden virtual function.
 *
 * \param[in] quitting  Whether STOP (false) or QUITTING (true) was
 * received.
 */
void messenger::stop(bool quitting)
{
    f_cui->stop(quitting);
}


/** \brief Send the CLUSTER_STATUS to communicatord once ready.
 *
 * This function builds a message and sends it to communicatord.
 *
 * The CLUSTER_UP and CLUSTER_DOWN messages are sent only when that specific
 * event happen and until then we do not know what the state really is
 * (although we assume the worst and use CLUSTER_DOWN until we get a reply).
 *
 * \param[in] status  The status of the fluid settings object.
 * \param[in] name  The name of the changing parameter.
 * \param[in] value  The value of the parameter.
 */
void messenger::fluid_settings_changed(
      fluid_settings::fluid_settings_status_t status
    , std::string const & name
    , std::string const & value)
{
    fluid_settings_connection::fluid_settings_changed(status, name, value);

    if(status == fluid_settings::fluid_settings_status_t::FLUID_SETTINGS_STATUS_READY)
    {
        f_cui->start_binary_connection();
    }
}



} // namespace prinbee_cui
// vim: ts=4 sw=4 et
