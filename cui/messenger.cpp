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
 * The messenger is primarily the CUI connection to the communicator server.
 * Since it derives from the prinbee_connection, it also manages the
 * binary connection to the prinbee proxy.
 *
 * It sets up its dispatcher and calls prinbeed functions whenever it
 * receives a message.
 *
 * \param[in] c  The PBQL CUI object we are listening for (the client).
 * \param[in] opts  The list of command line options.
 */
messenger::messenger(cui * c, advgetopt::getopt & opts)
    : prinbee_connection(opts, "pbql_cui")
    , f_cui(c)
{
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
    prinbee_connection::finish_initialization();
}


/** \brief Messenger received the READY message.
 *
 * Whenever we receive the READY message, we also receive our IP address
 * in the "my_address" parameter. This gets copied in the messenger object.
 *
 * \param[in,out] msg  The READY message.
 */
void messenger::ready(ed::message & msg)
{
    fluid_settings_connection::ready(msg);
}


/** \brief Handle a change of status in the proxy.
 *
 * This function gets called whenever the prinbee_connection receives
 * a message that ends up changing the proxy status.
 */
void messenger::process_proxy_status()
{
    prinbee_connection::process_proxy_status();

    if(is_proxy_connected())
    {
        f_cui->proxy_ready();
    }

    f_cui->update_status();
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
    prinbee_connection::fluid_settings_changed(status, name, value);

    switch(status)
    {
    case fluid_settings::fluid_settings_status_t::FLUID_SETTINGS_STATUS_READY:
        // ...
        break;

    case fluid_settings::fluid_settings_status_t::FLUID_SETTINGS_STATUS_VALUE:
    case fluid_settings::fluid_settings_status_t::FLUID_SETTINGS_STATUS_NEW_VALUE:
        //if(name == "...")
        //{
        //    apply_new_value();
        //}
        break;

    default:
        // ignore the delete, etc.
        break;

    }
}



} // namespace prinbee_cui
// vim: ts=4 sw=4 et
