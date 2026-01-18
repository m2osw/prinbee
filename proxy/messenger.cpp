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
 * \brief Messenger for the Prinbee proxy.
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

#include    "proxy.h"


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



namespace prinbee_proxy
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
 * \param[in] p  The proxy object we are listening for (i.e. "daemon").
 * \param[in] opts  The options received from the command line.
 */
messenger::messenger(proxy * p, advgetopt::getopt & opts)
    : fluid_settings_connection(opts, prinbee::g_name_prinbee_service_proxy)
    , f_proxy(p)
{
    set_name("prinbee_proxy_messenger");
    get_dispatcher()->add_matches({
            DISPATCHER_MATCH(::communicator::g_name_communicator_cmd_iplock_current_status, &messenger::msg_iplock_current_status),
            DISPATCHER_MATCH(prinbee::g_name_prinbee_cmd_prinbee_current_status,            &messenger::msg_prinbee_current_status),
            DISPATCHER_MATCH(prinbee::g_name_prinbee_cmd_prinbee_get_status,                &messenger::msg_prinbee_proxy_get_status),
    });
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

    // make sure the firewall is in place, which means:
    //
    // 1. We want to make sure that the ipload command ran successfully
    // 2. The status of the firewall is UP or ACTIVE
    // 3. This service accepts the IPLOCK_CURRENT_STATUS message
    // 4. Here we pro-actively request the status with IPLOCK_GET_STATUS
    // 5. Future changes are broadcast so we do not need to repeat the GET
    //
    ed::message iplock_get_status_msg;
    //iplock_get_status_msg.set_server(".");
    //iplock_get_status_msg.set_service(::communicator::g_name_communicator__service_communicatord);
    iplock_get_status_msg.reply_to(msg); // the ready message is from the communicatord so we can use reply_to() here
    iplock_get_status_msg.set_command(::communicator::g_name_communicator_cmd_iplock_get_status);
    iplock_get_status_msg.add_parameter(
              ::communicator::g_name_communicator_param_cache
            , ::communicator::g_name_communicator_value_no);
    send_message(iplock_get_status_msg);

    // request the current clock status
    //
    ed::message clock_status_msg;
    //clock_status_msg.set_server(".");
    //clock_status_msg.set_service(::communicator::g_name_communicator__service_communicatord);
    clock_status_msg.reply_to(msg); // the ready message is from the communicatord so we can use reply_to() here
    clock_status_msg.set_command(::communicator::g_name_communicator_cmd_clock_status);
    clock_status_msg.add_parameter(
              ::communicator::g_name_communicator_param_cache
            , ::communicator::g_name_communicator_value_no);
    send_message(clock_status_msg);

    // for completeness, call the following, however:
    //
    // * the firewall status will not yet be known
    // * the clock status will not have had time to respond either
    // * the fluid-settings service is not yet registered
    //
    // the proxy service just never expects any of the necessary messages
    // before the READY message is received
    //
    f_proxy->start_binary_connection();
}


/** \brief Handle the CLOCK_STABLE message.
 *
 * This function gets called whenever the daemon receives the
 * CLOCK_STABLE message.
 *
 * \param[in,out] msg  The CLOCK_STABLE message.
 */
void messenger::msg_clock_stable(ed::message & msg)
{
    f_proxy->set_clock_status(msg.get_parameter(::communicator::g_name_communicator_param_clock_resolution)
                                        == ::communicator::g_name_communicator_value_verified);
}


/** \brief Handle the CLOCK_UNSTABLE message.
 *
 * This function gets called whenever the daemon receives the
 * CLOCK_UNSTABLE message.
 *
 * \param[in,out] msg  The CLOCK_UNSTABLE message.
 */
void messenger::msg_clock_unstable(ed::message & msg)
{
    snapdev::NOT_USED(msg);

    f_proxy->set_clock_status(false);
}


/** \brief Handle the IPWALL_CURRENT_STATUS message.
 *
 * This function gets called whenever the daemon receives the
 * IPWALL_CURRENT_STATUS
 *
 * \param[in] msg  The IPWALL_CURRENT_STATUS message.
 */
void messenger::msg_iplock_current_status(ed::message & msg)
{
    // WARNING: the status is checked "manually" instead of calling the
    //          iplock::wait_on_firewall::from_string() and then
    //          testing the result; this is because prinbee cannot
    //          depend on the iplock project
    //
    std::string const status(msg.get_parameter(::communicator::g_name_communicator_param_status));
    f_proxy->set_ipwall_status(
               status == ::communicator::g_name_communicator_value_up
            || status == ::communicator::g_name_communicator_value_active);
}


/** \brief Handle the PRINBEE_CURRENT_STATUS message.
 *
 * This function gets called whenever the proxy receives the
 * PRINBEE_CURRENT_STATUS message. This happens whenever a
 * prinbee daemon broadcasts that message or when replying to
 * our status request.
 *
 * \param[in,out] msg  The PRINBEE_CURRENT_STATUS message.
 */
void messenger::msg_prinbee_current_status(ed::message & msg)
{
    f_proxy->msg_prinbee_current_status(msg);
}


/** \brief Handle the PRINBEE_PROXY_GET_STATUS message.
 *
 * This function is called whenever the proxy receives the
 * PRINBEE_PROXY_GET_STATUS message. This happens whenever a
 * prinbee client broadcasts that message. It sends a direct
 * reply to that client with the PRINBEE_PROXY_CURRENT_STATUS
 * message.
 *
 * \note
 * The proxy also broadcasts the PRINBEE_PROXY_CURRENT_STATUS
 * once it is ready to receive connections.
 *
 * \param[in,out] msg  The PRINBEE_PROXY_GET_STATUS message.
 */
void messenger::msg_prinbee_proxy_get_status(ed::message & msg)
{
    f_proxy->send_our_status(&msg);
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
    f_proxy->stop(quitting);
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
        f_proxy->start_binary_connection();
    }
}



} // namespace prinbee_proxy
// vim: ts=4 sw=4 et
