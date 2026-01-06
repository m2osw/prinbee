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
 * \brief Messenger for the prinbee daemon.
 *
 * The Prinbee daemon has a normal messenger connection. This is used to
 * find the other daemons and connect to them. The clients make use of a
 * direct connection so communication can happen with large binary data
 * (i.e. large files are to be sent to the backends).
 */


// self
//
#include    "messenger.h"

#include    "prinbeed.h"


// prinbee
//
#include    <prinbee/names.h>


// cluck
//
#include    <cluck/cluck_status.h>


// eventdispatcher
//
#include    <eventdispatcher/names.h>


// communicator
//
#include    <communicator/names.h>


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
 * The messenger is the Prinbee daemon connection to the communicator server.
 *
 * It sets up its dispatcher and calls Prinbeed functions whenever it
 * receives a message.
 *
 * \param[in] p  The Prinbee object we are listening for (i.e. "daemon").
 * \param[in] opts  The options received from the command line.
 */
messenger::messenger(prinbeed * p, advgetopt::getopt & opts)
    : fluid_settings_connection(opts, prinbee::g_name_prinbee_service_prinbeed)
    , f_prinbeed(p)
    , f_dispatcher(std::make_shared<ed::dispatcher>(this))
{
    set_name("messenger");
    set_dispatcher(f_dispatcher);
    add_fluid_settings_commands();
    f_dispatcher->add_matches({
            DISPATCHER_MATCH(::communicator::g_name_communicator_cmd_clock_stable,          &messenger::msg_clock_stable),
            DISPATCHER_MATCH(::communicator::g_name_communicator_cmd_clock_unstable,        &messenger::msg_clock_unstable),
            DISPATCHER_MATCH(::communicator::g_name_communicator_cmd_iplock_current_status, &messenger::msg_iplock_current_status),
            DISPATCHER_MATCH(prinbee::g_name_prinbee_cmd_prinbee_current_status,            &messenger::msg_prinbee_current_status),
            DISPATCHER_MATCH(prinbee::g_name_prinbee_cmd_prinbee_get_status,                &messenger::msg_prinbee_get_status),
    });
    f_dispatcher->add_communicator_commands();

#ifdef _DEBUG
    // further dispatcher initialization
    //
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
    cluck::listen_to_cluck_status(
              std::dynamic_pointer_cast<ed::connection_with_send_message>(shared_from_this())
            , f_dispatcher
            , std::bind(&messenger::msg_lock_status, this, std::placeholders::_1));

    process_fluid_settings_options();
    automatic_watch_initialization();
}


/** \brief Messenger received the READY message.
 *
 * Whenever we receive the READY message, we also receive our IP address
 * as the "my_address" parameter. This is made available via the
 * get_my_address() function.
 *
 * \param[in,out] msg  The READY message.
 *
 * \sa connection_with_send_message::msg_ready()
 * \sa connection_with_send_message::get_my_address()
 */
void messenger::ready(ed::message & msg)
{
    SNAP_LOG_TRACE
        << "got messenger::ready() called."
        << SNAP_LOG_SEND;
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
    // the prinbee daemon just never expects any of the necessary messages
    // before the READY message is received
    //
    f_prinbeed->start_binary_connection();
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
    f_prinbeed->set_clock_status(msg.get_parameter(::communicator::g_name_communicator_param_clock_resolution)
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

    f_prinbeed->set_clock_status(false);
}


/** \brief Handle the IPWALL_CURRENT_STATUS message.
 *
 * This function gets called whenever the daemon receives the
 * IPWALL_CURRENT_STATUS message.
 *
 * \param[in,out] msg  The IPWALL_CURRENT_STATUS message.
 */
void messenger::msg_iplock_current_status(ed::message & msg)
{
    // WARNING: the status is checked "manually" instead of calling the
    //          iplock::wait_on_firewall::from_string() and then
    //          testing the result; this is because prinbee cannot
    //          depend on the iplock project
    //
    std::string const status(msg.get_parameter(::communicator::g_name_communicator_param_status));
    f_prinbeed->set_ipwall_status(
               status == ::communicator::g_name_communicator_value_up
            || status == ::communicator::g_name_communicator_value_active);
}


/** \brief Handle the PRINBEE_CURRENT_STATUS message.
 *
 * This function gets called whenever the daemon receives the
 * PRINBEE_CURRENT_STATUS message. This happens whenever another
 * prinbee daemon broadcasts that message. This is also used as a
 * gossip to interconnect all the prinbee daemon together.
 *
 * \param[in,out] msg  The PRINBEE_CURRENT_STATUS message.
 */
void messenger::msg_prinbee_current_status(ed::message & msg)
{
    f_prinbeed->register_prinbee_daemon(msg);
}


/** \brief Handle the PRINBEE_GET_STATUS message.
 *
 * This function gets called whenever the daemon receives the
 * PRINBEE_GET_STATUS message. This happens whenever another
 * prinbee daemon broadcasts that message. This is also used as a
 * gossip to interconnect all the prinbee daemon together.
 *
 * This server replies with a PRINBEE_CURRENT_STATUS.
 *
 * \param[in,out] msg  The PRINBEE_GET_STATUS message.
 */
void messenger::msg_prinbee_get_status(ed::message & msg)
{
    f_prinbeed->send_our_status(&msg);
}


/** \brief Handle a change in lock status.
 *
 * This function is called whenever we receive a NO_LOCK or LOCK_READY
 * message from the cluck service. We react by checking whether we can
 * open our binary connections or not.
 *
 * \param[in,out] msg  The NO_LOCK or LOCK_READY message.
 */
void messenger::msg_lock_status(ed::message & msg)
{
    snapdev::NOT_USED(msg);
    f_prinbeed->lock_status_changed();
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
    fluid_settings_connection::fluid_settings_changed(status, name, value);

    if(status == fluid_settings::fluid_settings_status_t::FLUID_SETTINGS_STATUS_READY)
    {
        f_prinbeed->start_binary_connection();
    }
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
