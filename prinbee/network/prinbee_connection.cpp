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
 * \brief The messenger implementation for Prinbee clients.
 *
 * The Prinbee system uses messengers to communicate through the
 * communicator daemon. This is used for the basic status and'
 * make sure that all the necessary dependencies are ready.
 *
 * The clients use this connection which is like a fluid-settings
 * plus some additions specific to the Prinbee system. Especially,
 * the Prinbee connection includes the current cluster status.
 */

// self
//
#include    "prinbee/network/prinbee_connection.h"

#include    "prinbee/exception.h"
#include    "prinbee/names.h"
#include    "prinbee/network/constants.h"


// snaplogger
//
#include    <snaplogger/message.h>


// eventdispatcher
//
#include    <eventdispatcher/dispatcher.h>
#include    <eventdispatcher/dispatcher_support.h>
#include    <eventdispatcher/names.h>


// advgetopt
//
#include    <advgetopt/validator_duration.h>


// libaddr
//
#include    <libaddr/addr_parser.h>


// communicator
//
#include    <communicator/names.h>


// snapdev
//
//#include    <snapdev/escape_special_regex_characters.h>
//#include    <snapdev/floating_point_to_string.h>
#include    <snapdev/math.h>
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



namespace
{



const advgetopt::option g_options[] =
{
    // PRINBEE CONNECTION OPTIONS
    //
    advgetopt::define_option(
          advgetopt::Name("ping-pong-interval")
        , advgetopt::Flags(advgetopt::all_flags<
              advgetopt::GETOPT_FLAG_GROUP_OPTIONS
            , advgetopt::GETOPT_FLAG_DYNAMIC_CONFIGURATION
            , advgetopt::GETOPT_FLAG_REQUIRED
            , advgetopt::GETOPT_FLAG_SHOW_SYSTEM>())
        , advgetopt::EnvironmentVariableName("PRINBEE_PING_PONG_INTERVAL")
        , advgetopt::DefaultValue("5s")
        , advgetopt::Validator("duration(1s...1h)")
        , advgetopt::Help("How often to send a PING to the Prinbee proxy.")
    ),

    // END
    //
    advgetopt::end_options()
};



} // no name namespace



/** \brief Initialize a Prinbee connection to the Prinbee proxy.
 *
 * This class is expected to be used by all the Prinbee clients that want to
 * connect to the Prinbee system. The connection gives us information about
 * the Prinbee status and the ability to read and write data to the database.
 *
 * \param[in] opts  Command line options to tweak the connection settings.
 * \param[in] service_name  Name of the service creating this messenger.
 */
prinbee_connection::prinbee_connection(
          advgetopt::getopt & opts
        , std::string const & service_name)
    : fluid_settings_connection(opts, service_name)
    , f_communicator(ed::communicator::instance())
{
    set_name("prinbee_messenger");
    get_options().parse_options_info(g_options, true);

    get_dispatcher()->add_matches({
            DISPATCHER_MATCH(g_name_prinbee_cmd_prinbee_proxy_current_status, &prinbee_connection::msg_prinbee_proxy_current_status),
        });
}


prinbee_connection::~prinbee_connection()
{
}


/** \brief Finish the initialization of the prinbee client connection.
 *
 * This function makes sure the fluid settings and communicator daemon
 * have a chance to check the command line options and act on them.
 *
 * It is very important to call that function after the constructor
 * returned.
 */
void prinbee_connection::finish_initialization()
{
    process_fluid_settings_options();
    automatic_watch_initialization();
}


/** \brief Called whenever the proxy status changes.
 *
 * Whether we just got the connection to the communicator, fluid-settings,
 * or proxy we get this function called.
 */
void prinbee_connection::process_proxy_status()
{
    // nothing to do here
}


void prinbee_connection::set_proxy_status_and_address(
      std::string const & status
    , addr::addr const & address)
{
    if(status == f_proxy_status
    && address == f_address)
    {
        return;
    }

    f_proxy_status = status;
    f_address = address;

    process_proxy_status();
}


void prinbee_connection::set_proxy_readiness(bool is_ready)
{
    if(is_ready == f_ready)
    {
        return;
    }

    f_ready = is_ready;

    process_proxy_status();
}


std::string prinbee_connection::get_proxy_status() const
{
    if(f_proxy_connection == nullptr)
    {
        // the status is defined, the connection received a
        // PRINBEE_PROXY_CURRENT_STATUS message and that means the
        // proxy service is running but not yet available to
        // receive binary connections
        //
        if(!f_proxy_status.empty())
        {
            return "not available";
        }
        return "--";
    }

    std::stringstream ss;

    if(!f_ready)
    {
        // TODO: last_error.empty() is not sufficient
        //
        std::string const & last_error(f_proxy_connection->get_last_error());
        if(last_error.empty())
        {
            if(f_proxy_connection->is_enabled())
            {
                // no error but the timer is enabled that means we are
                // still trying to connect; this state happens at the
                // beginning or right after a lost connection
                //
                return "connecting";
            }

            // if there are no errors and the timer is disabled, then the
            // connection is there, but we're not yet "ready" (the REG
            // message was not acknowledged)
            //
            return "connected";
        }
        ss << "connection error: " << last_error;
        return ss.str();
    }

    ss << "registered";

    if(f_proxy_connection->get_last_ping() != snapdev::timespec_ex())
    {
        double const loadavg(f_proxy_connection->get_proxy_loadavg());
        if(loadavg >= 0.0)
        {
            ss << ", loadavg: "
               << loadavg;
        }
        else if(snapdev::quiet_floating_point_equal(loadavg, -1.0))
        {
            ss << ", loadavg: err";
        }
        // else loadavg == -2.0, not known yet

        std::uint32_t const no_answer(f_proxy_connection->get_no_pong_answer());
        if(no_answer > 0)
        {
            ss << " (stale: " << no_answer << ")";
        }
        else
        {
            ss << " (active)";
        }
    }

    return ss.str();
}


snapdev::timespec_ex prinbee_connection::get_last_ping() const
{
    if(f_proxy_connection == nullptr)
    {
        return snapdev::timespec_ex();
    }

    return f_proxy_connection->get_last_ping();
}


/** \brief Check the state of the proxy.
 *
 * This function verifies that the proxy is ready to receive a binary
 * connection from this client. This involves the following:
 *
 * \li It receives a valid IP and port from the proxy via a communicator
 *     daemon message;
 * \li The proxy status is "up" (so the proxy itself is considered up and
 *     running);
 * \li The messenger is "registered," which means it connected to the
 *     fluid-settings service; this is important to make sure we are
 *     going to receive the correct dynamic values;
 * \li The messenger is "ready," which means it was registered with the
 *     communicator daemon.
 *
 * When the fluid settings tells us it is ready, we have all the dynamic
 * values loaded too.
 *
 * \note
 * is_ready() is redundant since the are_fluid_settings_registered()
 * function has to return false if the messenger is not ready. However,
 * at this point, I'm not entirely sure that the states change as expected
 * when a connection is lost.
 * \note
 * Similarly, the are_fluid_settings_registered() needs to be true for
 * are_fluid_settings_ready() to also be true, so it is itself redundant.
 *
 * \return true if all the conditions representing a valid proxy are met.
 */
bool prinbee_connection::is_proxy_ready() const
{

SNAP_LOG_INFO
<< "----- is_proxy_ready() called! has_address()? " << std::boolalpha << has_address()
<< " f_proxy_status = " << f_proxy_status
<< " are fluid_settings registered? " << are_fluid_settings_registered()
<< " are fluid_settings ready? " << are_fluid_settings_ready()
<< " is messenger ready? " << is_ready()
<< SNAP_LOG_SEND;

    return has_address()                    // proxy gave us its binary IP address and port
        && f_proxy_status == "up"           // proxy service is considered up and running
        && are_fluid_settings_registered()  // registered with fluid-settings service
        && are_fluid_settings_ready()       // received the FLUID_SETTING_STATUS_READY message
        && is_ready();                      // the messenger is ready
}


bool prinbee_connection::is_proxy_connected() const
{
    return f_proxy_connection != nullptr
        && f_proxy_connection->is_connected();
}


addr::addr const & prinbee_connection::get_address() const
{
    return f_address;
}


bool prinbee_connection::has_address() const
{
    return f_address != addr::addr();
}


bool prinbee_connection::msg_process_reply(
      prinbee::binary_message::pointer_t msg
    , msg_reply_t state)
{
    // received a message reply from f_proxy_connection, process it
    //
    // the 'msg' is the message we SENT, the reply was an ACK or ERR
    // which pointed to that message, nothing more
    //
    switch(msg->get_name())
    {
    case prinbee::g_message_register:
        if(state == MSG_REPLY_SUCCEEDED)
        {
            // we are registered, ready to rock
            //
            set_proxy_readiness(true);
        }
        else
        {
            // we cannot register, trying again will fail again, what
            // to do?!
            //
            set_proxy_readiness(false);
        }
        return true;

    }

    SNAP_LOG_ERROR
        << "prinbee reply \""
        << prinbee::message_name_to_string(msg->get_name())
        << "\" not understood."
        << SNAP_LOG_SEND;

    return true;
}


void prinbee_connection::msg_prinbee_proxy_current_status(ed::message & msg)
{
    std::string status("unknown");
    if(msg.has_parameter(::communicator::g_name_communicator_param_status))
    {
        status = msg.get_parameter(::communicator::g_name_communicator_param_status);
    }

    addr::addr address;
    if(msg.has_parameter(g_name_prinbee_param_proxy_ip))
    {
        address = addr::string_to_addr(
                  msg.get_parameter(g_name_prinbee_param_proxy_ip)
                , "127.0.0.1"
                , CLIENT_BINARY_PORT
                , "tcp");
    }

    set_proxy_status_and_address(status, address);
}


void prinbee_connection::service_status(
      std::string const & server
    , std::string const & service
    , std::string const & status)
{
    fluid_settings_connection::service_status(server, service, status);

    if(service == g_name_prinbee_service_proxy
    && status != ::communicator::g_name_communicator_value_up)
    {
        // in this case, if the service goes UP, we ignore the message because
        // we will soon receive the PRINBEE_PROXY_CURRENT_STATUS message;
        // in all other cases we reset the status back to "down"
        //
        set_proxy_status_and_address("down", f_address);
    }
}


void prinbee_connection::ready(ed::message & msg)
{
    fluid_settings_connection::ready(msg);

    ed::connection_with_send_message * c(dynamic_cast<ed::connection_with_send_message *>(this));
    if(c == nullptr)
    {
        throw logic_error("the prinbee_connection class must also represent a connection_with_send_message."); // LCOV_EXCL_LINE
    }

    // send a PRINBEE_PROXY_GET_STATUS query message to get the current
    // database status
    //
    // TODO: to support direct connections, we would have to send a
    //       PRINBEE_GET_STATUS instead and then create a direct
    //       connection instead of the proxy connection...
    //
    ed::message prinbee_get_status_msg;
    prinbee_get_status_msg.reply_to(msg);
    prinbee_get_status_msg.set_command(g_name_prinbee_cmd_prinbee_proxy_get_status);
    prinbee_get_status_msg.add_parameter(
              ::communicator::g_name_communicator_param_cache
            , ::communicator::g_name_communicator_value_no);
    c->send_message(prinbee_get_status_msg);
}


void prinbee_connection::start_binary_connection()
{
    if(!is_proxy_ready())
    {
        // disconnect if we were connected before
        //
        f_communicator->remove_connection(f_proxy_connection);
        f_proxy_connection = nullptr;

        f_communicator->remove_connection(f_ping_pong_timer);
        f_ping_pong_timer = nullptr;
        return;
    }

    // already connected?
    //
    if(f_proxy_connection != nullptr)
    {
        SNAP_LOG_TRACE
            << "start_binary_connection: Proxy connection already allocated."
            << SNAP_LOG_SEND;
        return;
    }

    // the client is ready to connect to the local proxy binary port
    //
    f_proxy_connection = std::make_shared<proxy_connection>(this, f_address);
    f_proxy_connection->add_callbacks();
    f_communicator->add_connection(f_proxy_connection);

    // now that we have a proxy connection, initialize the ping-pong timer
    //
    if(f_ping_pong_timer == nullptr)
    {
        f_ping_pong_timer = std::make_shared<ed::timer>(0);
        if(f_communicator->add_connection(f_ping_pong_timer))
        {
            f_ping_pong_timer->get_callback_manager().add_callback(
                [this](ed::timer::pointer_t t) {
                    return this->send_ping(t);
                });

            set_ping_pong_interval();
        }
        else
        {
            SNAP_LOG_RECOVERABLE_ERROR
                << "could not add ping-pong timer to list of ed::communicator connections."
                << SNAP_LOG_SEND;
            f_ping_pong_timer.reset();
        }
    }
}


void prinbee_connection::set_ping_pong_interval()
{
    if(f_ping_pong_timer == nullptr)
    {
        return;
    }

    double ping_pong_interval(0.0);
    if(!advgetopt::validator_duration::convert_string(
                  get_options().get_string("ping_pong_interval")
                , advgetopt::validator_duration::VALIDATOR_DURATION_DEFAULT_FLAGS
                , ping_pong_interval))
    {
        SNAP_LOG_CONFIGURATION_WARNING
            << "the --ping-pong-interval does not represent a valid duration."
            << SNAP_LOG_SEND;
        ping_pong_interval = 5.0;
    }

    // minimum is 1 second and maximum 1 hour
    //
    ping_pong_interval = std::clamp(ping_pong_interval, 1.0, 60.0 * 60.0) * 1'000'000.0;

    f_ping_pong_timer->set_timeout_delay(ping_pong_interval);
}


bool prinbee_connection::send_ping(ed::timer::pointer_t t)
{
    snapdev::NOT_USED(t);

    if(f_proxy_connection == nullptr)
    {
        return true;
    }

    if(f_proxy_connection->get_expected_ping() != 0)
    {
        std::uint32_t const count(f_proxy_connection->increment_no_pong_answer());
        if(count >= prinbee::MAX_PING_PONG_FAILURES)
        {
            SNAP_LOG_ERROR
                << "connection never replied from our last "
                << prinbee::MAX_PING_PONG_FAILURES
                << " PING signals; reconnecting."
                << SNAP_LOG_SEND;

            // TODO: actually implement...
            //
            throw prinbee::not_yet_implemented("easy in concept, we'll implement that later though...");

            // don't send a PING now
            //
            return true;
        }
        SNAP_LOG_MAJOR
            << "connection never replied from our last "
            << count
            << " PING signals."
            << SNAP_LOG_SEND;
    }

    prinbee::binary_message::pointer_t ping_msg(std::make_shared<prinbee::binary_message>());
    ping_msg->create_ping_message();
    f_proxy_connection->set_expected_ping(ping_msg->get_serial_number());
    f_proxy_connection->send_message(ping_msg);

    return false;
}


/** \brief Attempt a connection to the proxy.
 *
 * This function gets called whenever the status of the fluid settings
 * changes. When that happens and the status is now READY, we want to
 * check whether we can connect to the proxy binary port.
 *
 * \param[in] status  The status of the fluid settings object.
 * \param[in] name  The name of the changing parameter.
 * \param[in] value  The value of the parameter.
 */
void prinbee_connection::fluid_settings_changed(
      fluid_settings::fluid_settings_status_t status
    , std::string const & name
    , std::string const & value)
{
    fluid_settings_connection::fluid_settings_changed(status, name, value);

    switch(status)
    {
    case fluid_settings::fluid_settings_status_t::FLUID_SETTINGS_STATUS_READY:
        start_binary_connection();
        break;

    case fluid_settings::fluid_settings_status_t::FLUID_SETTINGS_STATUS_VALUE:
    case fluid_settings::fluid_settings_status_t::FLUID_SETTINGS_STATUS_NEW_VALUE:
        if(name == "pbql-cui::ping-pong-interval")
        {
            set_ping_pong_interval();
        }
        break;

    default:
        // ignore the delete, etc.
        break;

    }
}



    // TODO: the state of the cluster needs to be transmitted using the
    //       binary connection -- until then we have a very poor idea
    //       of the state anyway (not much better than when connected
    //       through the messenger)
    //
    //std::string const state(msg.get_parameter(communicator::g_name_communicator_param_status));
    //f_prinbee_state.from_json(
    //          STATE_JOURNAL_APPLICATION_STATUS
    //        | STATE_JOURNAL_LOCAL_STATUS
    //        | STATE_JOURNAL_REMOTE_STATUS
    //        | STATE_PROXY_STATUS
    //        | STATE_CLUSTER_STATUS
    //        | STATE_DAEMONS_STATUS
    //        , state);
    //f_prinbee_state.signal_state_changed();


} // namespace prinbee
// vim: ts=4 sw=4 et
