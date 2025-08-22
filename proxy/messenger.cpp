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
 * \brief Messenger for the prinbee proxy.
 *
 * The Prinbee proxy has a normal messenger connection. This is used to:
 *
 * 1. Find the Prinbee daemons and connect to them
 * 2. Get the Prinbee daemon binary connection information
 * 2. Let clients find the Proxy
 * 3. Give clients the binary connection information
 *
 * The messenger is also used to make sure that the firewall is up and
 * running before opening the binary connection.
 */


// self
//
#include    "messenger.h"

#include    "proxyd.h"


// prinbee
//
#include    <prinbee/names.h>


// eventdispatcher
//
#include    <eventdispatcher/names.h>


// communicatord
//
#include    <communicatord/names.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_daemon
{


/** \class messenger
 * \brief Handle messages from the communicatord.
 *
 * This class is an implementation of the TCP client message connection
 * so we can handle incoming messages. We actually use the fluid-settings
 * which itself uses the communicatord connection. All of the basic
 * communication messages used by the communicatord and fluid settings
 * are handled automatically.
 *
 * This class handles the lock messages.
 */



/** \brief The messenger initialization.
 *
 * The messenger is the cluck daemon connection to the communicator server.
 *
 * It sets up its dispatcher and calls prinbeed functions whenever it
 * receives a message.
 *
 * \param[in] p  The prinbee object we are listening for (i.e. "daemon").
 * \param[in] opts  The options received from the command line.
 */
messenger::messenger(prinbeed * p, advgetopt::getopt & opts)
    : fluid_settings_connection(opts, "prinbeed")
    , f_prinbeed(p)
    , f_dispatcher(std::make_shared<ed::dispatcher>(this))
{
    set_name("messenger");
    set_dispatcher(f_dispatcher);
    add_fluid_settings_commands();
    f_dispatcher->add_matches({
            DISPATCHER_MATCH(communicatord::g_name_communicatord_cmd_ipwall_current_status, &messenger::msg_ipwall_current_status),
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
    f_prinbeed->start_binary_connection();

    // check the ipwall service status using the IPWALL_GET_STATUS message
    //
    // TODO: I do not think we need the dynamic ipwall for prinbee; just
    //       the ipload being active would be sufficient (i.e. we open a
    //       LAN port so anyone on our LAN can connect; it's not all wide
    //       open; at the same time, the admin can setup that IP address
    //       as the public IP address so it could be semi-dangerous)
    //
    ed::message ipwall_get_status;
    ipwall_get_status.reply_to(msg);
    ipwall_get_status.set_command(communicatord::g_name_communicatord_cmd_ipwall_get_status);
    ipwall_get_status.add_parameter(
              communicatord::g_name_communicatord_param_cache
            , communicatord::g_name_communicatord_value_no);
    send_message(ipwall_get_status);
}


/** \brief Handle the IPWALL_CURRENT_STATUS message.
 *
 * This function gets called whenever the daemon receives the
 * IPWALL_CURRENT_STATUS
 *
 * \param[in] msg  The IPWALL_CURRENT_STATUS message.
 */
void messenger::msg_ipwall_current_status(ed::message & msg)
{
    f_prinbeed->set_ipwall_status(msg.get_parameter(communicatord::g_name_communicatord_param_status)
                                        == communicatord::g_name_communicatord_value_up);
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
    f_prinbeed->stop(quitting);
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
    snapdev::NOT_USED(name, value);

    if(status == fluid_settings::fluid_settings_status_t::FLUID_SETTINGS_STATUS_READY)
    {
        f_prinbeed->start_binary_connection();
    }
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
