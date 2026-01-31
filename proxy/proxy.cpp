// Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
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

// prinbee_proxy
//
#include    "proxy.h"

#include    "listener.h"


// prinbee
//
#include    <prinbee/network/constants.h>
#include    <prinbee/version.h>


// advgetopt
//
#include    <advgetopt/exception.h>
//#include    <advgetopt/options.h>
//#include    <advgetopt/utils.h>
#include    <advgetopt/validator_duration.h>


// communicator
//
//#include    <communicator/flags.h>
#include    <communicator/names.h>


// snaplogger
//
#include    <snaplogger/options.h>


// libaddr
//
#include    <libaddr/addr_parser.h>


// snapdev
//
#include    <snapdev/gethostname.h>
//#include    <snapdev/not_reached.h>
//#include    <snapdev/raii_generic_deleter.h>
#include    <snapdev/stringize.h>
#include    <snapdev/to_lower.h>


// cppthread
//
#include    <cppprocess/process.h>
#include    <cppprocess/io_capture_pipe.h>


//// C++
////
//#include    <fstream>
//#include    <map>
//
//
//// C
////
//#include    <glob.h>
//#include    <limits.h>
//#include    <stdlib.h>
//#include    <sys/stat.h>
//#include    <sys/sysmacros.h>
//#include    <unistd.h>


// last include
//
#include    <snapdev/poison.h>





/** \file
 * \brief Proxy daemon that runs on all the machines.
 *
 * The proxy helps each client by handling the communication between
 * the computer on which it sits (i.e. clients) and the prinbee deamons.
 */

namespace prinbee_proxy
{
namespace
{



advgetopt::option const g_options[] =
{
    advgetopt::define_option(
          advgetopt::Name("client-listen")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Specify an address and port to listen on for direct client connections; if the IP is not defined or set to ANY, then only the port is used and this computer public IP address is used.")
        , advgetopt::DefaultValue(":4011")
    ),
    advgetopt::define_option(
          advgetopt::Name("cluster-name")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Specify the name of the cluster the proxy is to work with.")
    ),
    advgetopt::define_option(
          advgetopt::Name("node-name")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Specify the name of this prinbee proxy node. By default the host name is used.")
    ),
    advgetopt::define_option(
          advgetopt::Name("ping-pong-interval")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("How often to send a PING to all the daemons.")
        , advgetopt::Validator("duration(1s...1h)")
        , advgetopt::DefaultValue("5s")
    ),
    advgetopt::define_option(
          advgetopt::Name("prinbee-path")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Specify a path where the database is to be saved.")
        , advgetopt::DefaultValue("/var/lib/prinbee")
    ),
    advgetopt::define_option(
          advgetopt::Name("owner")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Specify the user and group names ([<user>][:<group>]). The names are optional.")
    ),
    advgetopt::end_options()
};


advgetopt::group_description const g_group_descriptions[] =
{
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
        , advgetopt::GroupName("command")
        , advgetopt::GroupDescription("Commands:")
    ),
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
        , advgetopt::GroupName("option")
        , advgetopt::GroupDescription("Options:")
    ),
    advgetopt::end_groups()
};


constexpr char const * const g_configuration_files[] =
{
    "/etc/prinbee/prinbee-proxy.conf",
    nullptr
};


advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "prinbee-proxy",
    .f_group_name = "prinbee",
    .f_options = g_options,
    .f_environment_variable_name = "PRINBEE_PROXY_OPTIONS",
    .f_configuration_files = g_configuration_files,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS
                         | advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [-<opt>]\n"
                     "where -<opt> is one or more of:",
    .f_help_footer = "%c",
    .f_version = PRINBEE_VERSION_STRING,
    .f_license = "GNU GPL v3",
    .f_copyright = "Copyright (c) 2025-"
                   SNAPDEV_STRINGIZE(UTC_BUILD_YEAR)
                   " by Made to Order Software Corporation -- All Rights Reserved",
    .f_groups = g_group_descriptions,
};



}
// no name namespace













/** \class proxy
 * \brief Class handling Prinbee messages between clients and daemons.
 *
 * This class creates the proxy service. This service is used by clients
 * to connect to a Prinbee cluster.
 */



/** \brief Initializes a proxy object.
 *
 * This function parses the command line arguments, reads configuration
 * files, setups the messenger, and the logger.
 *
 * \param[in] argc  The number of arguments in the argv array.
 * \param[in] argv  The array of argument strings.
 */
proxy::proxy(int argc, char * argv[])
    : f_opts(g_options_environment)
    , f_start_date(snapdev::now())
{
    snaplogger::add_logger_options(f_opts);

    // before we can parse command line arguments, we must create the
    // fluid settings & communicator client objects which happen to
    // dynamically add command line options to f_opts
    //
    f_messenger = std::make_shared<messenger>(this, f_opts);

    f_opts.finish_parsing(argc, argv);
    if(!snaplogger::process_logger_options(f_opts, "/etc/prinbee/logger"))
    {
        throw advgetopt::getopt_exit("logger options generated an error.", 1);
    }

    // setup the path to the prinbee data folder which includes things like
    // the journals used by the proxy
    //
    if(f_opts.is_defined("prinbee-path"))
    {
        prinbee::set_prinbee_path(f_opts.get_string("prinbee-path"));
    }

    // right now we want the proxy to match the cluster name of the
    // Prinbee daemon; at some point, though, we probably want to
    // support all clusters within one proxy
    //
    // we also want to include a node name that way we know which proxy
    // connects to which Prinbee daemon
    //
    f_cluster_name = snapdev::to_lower(f_opts.get_string("cluster_name"));
    if(!prinbee::validate_name(f_cluster_name.c_str(), 100))
    {
        throw advgetopt::getopt_exit("the cluster name is not considered a valid name.", 1);
    }
    if(f_opts.is_defined("node_name"))
    {
        f_node_name = f_opts.get_string("node_name");
    }
    else
    {
        f_node_name = snapdev::gethostname();
    }
    if(!prinbee::validate_name(f_node_name.c_str(), 100))
    {
        throw advgetopt::getopt_exit("node name \"" + f_node_name + "\" is not considered valid.", 1);
    }
    if(!prinbee::verify_node_name(f_node_name.c_str()))
    {
        throw advgetopt::getopt_exit("the node name cannot end with \"_proxy\" or \"_client\", \"" + f_node_name + "\" is not considered valid.", 1);
    }

    if(getuid() == 0
    || geteuid() == 0
    || getgid() == 0
    || getegid() == 0)
    {
        throw prinbee::invalid_user("the prinbee proxy cannot run as root. Try using the \"prinbee\" user and group."); // LCOV_EXCL_LINE
    }
}


/** \brief Do some clean ups.
 *
 * At this point, the destructor is present mainly because we have
 * some virtual functions.
 */
proxy::~proxy()
{
}


std::string const & proxy::get_node_name() const
{
    return f_node_name;
}


/** \brief Finish the proxy service initialization.
 *
 * This function creates all the connections used by the proxy daemon.
 *
 * \note
 * This is separate from the run() function so we can run unit tests
 * against the proxy daemon.
 *
 * \sa run()
 */
void proxy::finish_initialization()
{
    f_communicator = ed::communicator::instance();

    // capture Ctrl-C (SIGINT) to get a clean exit by default
    //
    f_interrupt = std::make_shared<interrupt>(this);
    f_communicator->add_connection(f_interrupt);

    // add the messenger used to communicate with the communicator daemon
    // and other services as required
    //
    f_communicator->add_connection(f_messenger);

    // the following call actually connects the messenger to the
    // communicator daemon
    //
    f_messenger->finish_parsing();

    if(f_opts.is_defined("owner"))
    {
        std::string owner(f_opts.get_string("owner"));
        std::string::size_type const pos(owner.find(':'));
        if(pos == std::string::npos)
        {
            if(!owner.empty())
            {
                f_user = owner;
            }
        }
        else
        {
            std::string part(owner.substr(0, pos));
            if(!part.empty())
            {
                f_user = part;
            }

            part = owner.substr(pos + 1);
            if(!part.empty())
            {
                f_group = part;
            }
        }
    }
}


/** \brief Set the ipwall status from the IPWALL_CURRENT_STATUS message.
 *
 * The daemon listens for IPWALL_CURRENT_STATUS messages, it accepts
 * connections on the binary connection only after the status is UP.
 *
 * \note
 * If later the status goes down, this daemon continues to listen on the
 * same connections. This is safe because the ipwall should never go
 * down once it was up (i.e. we never clear the firewall dry).
 *
 * \param[in] status  The new ipwall status, if true, the firewall is
 *                    considered to be up.
 */
void proxy::set_ipwall_status(bool status)
{
    if(status
    && !f_ipwall_is_up)
    {
        f_ipwall_is_up = true;
        start_binary_connection();
    }
}


/** \brief Handle the PRINBEE_CURRENT_STATUS message.
 *
 * This function handles the PRINBEE_CURRENT_STATUS message. This means
 * registering the prinbee daemon that sent that message and if not
 * yet connected with it, create a connection.
 *
 * Note that like with the communicator daemon, we want to connect from
 * one prinbee daemon to another only if the one has a smaller IP address.
 * Otherwise, do nothing (i.e. the other daemon will connect to us
 * automatically when it receives this very message).
 *
 * \param[in,out] msg  The PRINBEE_CURRENT_STATUS message.
 */
void proxy::msg_prinbee_current_status(ed::message & msg)
{
    if(!msg.has_parameter(prinbee::g_name_prinbee_param_cluster_name))
    {
        SNAP_LOG_ERROR
            << "PRINBEE_CURRENT_STATUS message is missing the parameter with the prinbeed cluster name."
            << SNAP_LOG_SEND;
        return;
    }
    // TODO: I think that inside the proxy this would be different as in:
    //       we want to mark each connection with their cluster name and
    //       a client can use a cluster name on top of the context name
    //       etc.
    {
        std::string const cluster_name(msg.get_parameter(prinbee::g_name_prinbee_param_cluster_name));
        if(cluster_name != f_cluster_name)
        {
            // this is not an error, multiple Prinbee clusters can co-exist
            // in the same communicator cluster
            //
            SNAP_LOG_NOISY
                << "PRINBEE_CURRENT_STATUS message is for a different cluster (expected: \""
                << f_cluster_name
                << "\", got \""
                << cluster_name
                << "\")."
                << SNAP_LOG_SEND;
            return;
        }
    }

    if(!msg.has_parameter(communicator::g_name_communicator_param_status))
    {
        SNAP_LOG_ERROR
            << "PRINBEE_CURRENT_STATUS message is missing the "
            << communicator::g_name_communicator_param_status
            << " parameter."
            << SNAP_LOG_SEND;
        return;
    }

    if(msg.get_parameter(communicator::g_name_communicator_param_status)
                            != communicator::g_name_communicator_value_up)
    {
        SNAP_LOG_VERBOSE
            << "received a PRINBEE_CURRENT_STATUS message where the status is not UP."
            << SNAP_LOG_SEND;
        return;
    }

    // the proxy needs to use the Proxy IP address
    // (of the three sent by the Prinbee daemon)
    //
    if(!msg.has_parameter(prinbee::g_name_prinbee_param_proxy_ip))
    {
        SNAP_LOG_ERROR
            << "PRINBEE_CURRENT_STATUS message is missing the proxy IP address."
            << SNAP_LOG_SEND;
        return;
    }

    if(!msg.has_parameter(prinbee::g_name_prinbee_param_node_name))
    {
        SNAP_LOG_ERROR
            << "PRINBEE_CURRENT_STATUS message is missing the parameter with the prinbeed node name."
            << SNAP_LOG_SEND;
        return;
    }

    std::string const daemon_address(msg.get_parameter(prinbee::g_name_prinbee_param_proxy_ip));
    addr::addr const a(addr::string_to_addr(
                      daemon_address
                    , std::string()
                    , prinbee::NODE_BINARY_PORT));
    switch(a.get_network_type())
    {
    case addr::network_type_t::NETWORK_TYPE_PUBLIC:
    case addr::network_type_t::NETWORK_TYPE_PRIVATE:
    case addr::network_type_t::NETWORK_TYPE_LOOPBACK:
        break;

    default:
        SNAP_LOG_ERROR
            << "this other node address ("
            << daemon_address
            << ") is not a valid address for a node."
            << SNAP_LOG_SEND;
        return;

    }

    // connect to that daemon
    //
    std::string const name(msg.get_parameter(prinbee::g_name_prinbee_param_node_name));
    connect_to_daemon(a, name);
}


/** \brief Set the clock status from the CLOCK_STABLE message.
 *
 * The daemon listens for CLOCK_STABLE messages, it accepts
 * connections on the binary connection only after the click is
 * considered stable (a.k.a. synchronized with an NTP server).
 *
 * \todo
 * If later the status goes down, this daemon continues to run with an
 * invalid clock. This is because once we opened the binary connections,
 * we just don't take them back down until we quit.
 *
 * \param[in] status  The new clock status, if true, the clock is
 *                    considered to be stable.
 */
void proxy::set_clock_status(bool status)
{
    if(status
    && !f_stable_clock)
    {
        f_stable_clock = true;
        start_binary_connection();
    }
}


void proxy::register_client(prinbee::binary_server_client::pointer_t client)
{
    connection_reference::pointer_t ref(std::make_shared<connection_reference>(client));
    f_client_connections[client.get()] = ref;
}


void proxy::client_disconnected(prinbee::binary_server_client::pointer_t client)
{
    auto it(f_client_connections.find(client.get()));
    if(it != f_client_connections.end())
    {
        f_client_connections.erase(it);
    }
    else
    {
        SNAP_LOG_RECOVERABLE_ERROR
            << "received a request to disconnect a client when client was not registered."
            << SNAP_LOG_SEND;
    }
}


connection_reference::pointer_t proxy::find_connection_reference(ed::connection::pointer_t client)
{
    auto it(f_client_connections.find(client.get()));
    if(it == f_client_connections.end())
    {
        return connection_reference::pointer_t();
    }
    return it->second;
}


/** \brief Run the prinbee daemon.
 *
 * This function is the core function of the daemon. It runs the loop
 * used to accept messenger and direct binary connections between the
 * database daemon (prinbeed) and proxy.
 *
 * \sa add_connections()
 */
int proxy::run()
{
    SNAP_LOG_INFO
        << "--------------------------------- prinbee proxy started."
        << SNAP_LOG_SEND;

    // now run our listening loop
    //
    f_communicator->run();

    return 0;
}


void proxy::timed_out()
{
    snapdev::timespec_ex next_time;

    // go through possibly expired connections
    //
    for(auto it(f_client_connections.begin());
             it != f_client_connections.end();
             )
    {
        // was the protocol defined yet?
        //
        if(it->second->get_protocol() == nullptr)
        {
            // no protocol, verify that the client "just" registered
            //
            snapdev::timespec_ex diff(snapdev::now() - it->second->get_connection_date());
            if(diff >= 1.0) // already 1s and still no REG message?!
            {
                // TODO: disconnect
                //
                f_communicator->remove_connection(it->second->get_connection());
                it = f_client_connections.erase(it);
                continue;
            }
            next_time = std::min(next_time, snapdev::timespec_ex(1, 0));
        }

        ++it;
    }
}


/** \brief Start the binary connection.
 *
 * First, this function makes sure it can start the binary connections.
 * This means:
 *
 * \li The firewall is up
 * \li The clock on this computer is considered stable
 * \li The connection to the communicatord is ready
 * \li The connection to the fluid settings server is ready
 *
 * Once ready, it starts the two binary connections:
 *
 * \li A socket to accept connections from proxies
 * \li A socket to accept connections from other daemons
 *
 * The daemons use a complete network, meaning that all the nodes within
 * a cluster connect to all the other nodes in that cluster. In most cases,
 * inter-cluster communication uses only one connection to better control
 * costs.
 *
 * \note
 * Since the service receives different messages that trigger a call to
 * this function, the function checks the server current status every time.
 * This includes a test to see whether the connections are already in place.
 * If so, nothing happens.
 *
 * \exception prinbee::invalid_address
 * The messenger has this computer's IP address defined. It gets used by
 * the proxy listener. If the address happens to be invalid (i.e. not
 * usable to listen on--i.e. documentation IPv6 address) then this
 * exception is raised.
 */
void proxy::start_binary_connection()
{
    // already connected?
    //
    if(f_listener != nullptr)
    {
        return;
    }

    // did we receive the READY message?
    //
    if(!f_messenger->is_ready())
    {
        return;
    }

    // did we receive the FLUID_SETTINGS_READY message?
    //
    if(!f_messenger->are_fluid_settings_ready())
    {
        return;
    }

    // did we receive IPWALL_CURRENT_STATUS message with status UP?
    //
    if(!f_ipwall_is_up)
    {
        return;
    }

    // in a cluster of synchronized nodes, the synchronization uses time
    // so the clock has to be up and running properly on each system even
    // clients (since proxies runs on clients)
    //
    if(!f_stable_clock)
    {
        return;
    }

    // we want the my-address to be defined in case the user wants that as
    // the address to use to open the ports; this gets defined when we
    // receive the READY message from the communicator daemon
    //
    addr::addr my_address(f_messenger->get_my_address());
    switch(my_address.get_network_type())
    {
    case addr::network_type_t::NETWORK_TYPE_PUBLIC:
    case addr::network_type_t::NETWORK_TYPE_PRIVATE:
    case addr::network_type_t::NETWORK_TYPE_LOOPBACK:
        break;

    default:
        throw prinbee::invalid_address("the messenger address is not a valid address.");

    }

    // the proxy is ready to listen for connections from clients, open the ports
    //
    addr::addr listen_address(addr::string_to_addr(
                          f_opts.get_string("listen")
                        , std::string()
                        , prinbee::CLIENT_BINARY_PORT));
    my_address.set_port(listen_address.get_port());
    if(listen_address.is_default())
    {
        listen_address = my_address;
    }
    f_address = my_address.to_ipv4or6_string(
                          addr::STRING_IP_ADDRESS
                        | addr::STRING_IP_BRACKET_ADDRESS
                        | addr::STRING_IP_PORT);

    // TODO: add support for TLS connections
    //
    f_listener = std::make_shared<listener>(this, listen_address);
    f_communicator->add_connection(f_listener);

    // request the current status of the prinbee daemons
    //
    ed::message prinbee_get_status;
    prinbee_get_status.set_command(prinbee::g_name_prinbee_cmd_prinbee_get_status);
    prinbee_get_status.set_service(prinbee::g_name_prinbee_service_prinbeed);
    prinbee_get_status.set_server(communicator::g_name_communicator_service_private_broadcast);
    prinbee_get_status.add_parameter(
              communicator::g_name_communicator_param_cache
            , communicator::g_name_communicator_value_no);
    f_messenger->send_message(prinbee_get_status);

    // we also need to send our status to everyone else
    //
    send_our_status(nullptr);

    // initialize the ping pong timer
    // minimum is 1 second and maximum 1 hour
    //
    if(f_ping_pong_timer == nullptr)
    {
        double ping_pong_interval(0.0);
        if(!advgetopt::validator_duration::convert_string(
                      f_opts.get_string("ping_pong_interval")
                    , advgetopt::validator_duration::VALIDATOR_DURATION_DEFAULT_FLAGS
                    , ping_pong_interval))
        {
            SNAP_LOG_CONFIGURATION_WARNING
                << "the --ping-pong-interval does not represent a valid duration."
                << SNAP_LOG_SEND;
            ping_pong_interval = 5.0;
        }

        ping_pong_interval = std::clamp(ping_pong_interval, 1.0, 60.0 * 60.0) * 1'000'000.0;
        f_ping_pong_timer = std::make_shared<ping_pong_timer>(this, ping_pong_interval);
        if(!f_communicator->add_connection(f_ping_pong_timer))
        {
            SNAP_LOG_RECOVERABLE_ERROR
                << "could not add ping-pong timer to list of ed::communicator connections."
                << SNAP_LOG_SEND;
        }
    }
}


void proxy::send_our_status(ed::message * msg)
{
    // we send our status which generates an equivalent of a gossip message
    // since this message is used by clients to connect to the Prinbee proxy
    //
    ed::message prinbee_current_status;
    prinbee_current_status.set_command(prinbee::g_name_prinbee_cmd_prinbee_proxy_current_status);
    if(msg == nullptr)
    {
        prinbee_current_status.set_service(communicator::g_name_communicator_service_private_broadcast);
    }
    else
    {
        prinbee_current_status.reply_to(*msg);
    }

    prinbee_current_status.add_parameter(
              prinbee::g_name_prinbee_param_cluster_name
            , f_cluster_name);
    prinbee_current_status.add_parameter(
              communicator::g_name_communicator_param_cache
            , communicator::g_name_communicator_value_no);

    if(f_address.empty())
    {
        prinbee_current_status.add_parameter(
                  communicator::g_name_communicator_param_status
                , communicator::g_name_communicator_value_down);
    }
    else
    {
        prinbee_current_status.add_parameter(
                  communicator::g_name_communicator_param_status
                , communicator::g_name_communicator_value_up);
        prinbee_current_status.add_parameter(
                  prinbee::g_name_prinbee_param_proxy_ip
                , f_address);
    }

    f_messenger->send_message(prinbee_current_status);
}


void proxy::connect_to_daemon(addr::addr const & a, std::string const & name)
{
    daemon::pointer_t d(std::make_shared<daemon>(this, a));
    d->set_name(name);
    d->add_callbacks();

    // this call just registers the connection in a table in prinbeed,
    // it does not send the REG message to the other side, which we do
    // when we get the process_connected() called
    //
    f_daemon_connections[d.get()] = d;

    if(!f_communicator->add_connection(d))
    {
        SNAP_LOG_RECOVERABLE_ERROR
            << "could not add connection to daemon to list of ed::communicator connections."
            << SNAP_LOG_SEND;
    }

SNAP_LOG_WARNING << "--- connected with " << name << " prinbee daemon at " << a << " as " << f_node_name << "_proxy ... waiting for REG acknowledgement." << SNAP_LOG_SEND;
}


bool proxy::msg_error(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    std::string name;
    ed::connection::pointer_t client(std::dynamic_pointer_cast<ed::connection>(peer));
    if(client == nullptr)
    {
        throw prinbee::logic_error("peer is not an eventdispatcher connection, cannot retrieve its name.");
    }
    name = client->get_name();

    prinbee::msg_error_t err;
    msg->deserialize_error_message(err);

    SNAP_LOG_ERROR
        << name
        << ": "
        << err.f_message_name
        << " ("
        << static_cast<int>(err.f_code)
        << ")"
        << SNAP_LOG_SEND;

    return true;
}


bool proxy::msg_process_reply(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg
    , prinbee::msg_reply_t state)
{
    snapdev::NOT_USED(peer, msg, state);

    return true;
}


void proxy::send_message(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    prinbee::binary_client::pointer_t client(std::dynamic_pointer_cast<prinbee::binary_client>(peer));
    if(client != nullptr)
    {
        client->send_message(msg);
        return;
    }

    prinbee::binary_server_client::pointer_t server_client(std::dynamic_pointer_cast<prinbee::binary_server_client>(peer));
    if(server_client != nullptr)
    {
        server_client->send_message(msg);
        return;
    }

    throw prinbee::logic_error("unknown peer type, cannot send message to it.");
}


void proxy::send_pings()
{
    for(auto const & it : f_daemon_connections)
    {
        if(it.second->get_expected_ping() != 0)
        {
            std::uint32_t const count(it.second->increment_no_pong_answer());
            if(count >= prinbee::MAX_PING_PONG_FAILURES)
            {
                SNAP_LOG_ERROR
                    << "connection never replied from our last "
                    << prinbee::MAX_PING_PONG_FAILURES
                    << " PING signals; reconnecting."
                    << SNAP_LOG_SEND;
                throw prinbee::not_yet_implemented("easy in concept, we'll implement that later though...");

                // don't send a PING now, just loop to handle the next connection
                //
                continue;
            }
            SNAP_LOG_MAJOR
                << "connection never replied from our last "
                << count
                << " PING signals."
                << SNAP_LOG_SEND;
        }

        prinbee::binary_message::pointer_t ping_msg(std::make_shared<prinbee::binary_message>());
        ping_msg->create_ping_message();
        it.second->set_expected_ping(ping_msg->get_serial_number());
        send_message(it.second, ping_msg);
    }
}


/** \brief Register a client.
 *
 * Whenever a client connects to a proxy, it immediately sends a REG message.
 * This function checks the version of the client to make sure the proxy can
 * properly communicate with it (i.e. has backward compatibility is necessary).
 *
 * \param[in] peer  The very connection that just received the REG message.
 * \param[in,out] msg  The REG message.
 *
 * \return Always true.
 */
bool proxy::msg_register(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    prinbee::msg_register_t r;
    if(!msg->deserialize_register_message(r))
    {
        return true;
    }

    versiontheca::decimal::pointer_t their_protocol_trait(std::make_shared<versiontheca::decimal>());
    versiontheca::versiontheca::pointer_t their_protocol(std::make_shared<versiontheca::versiontheca>(their_protocol_trait, r.f_protocol_version));
    if(f_protocol_version->get_major() != their_protocol->get_major())
    {
        // the major version must be exactly equal or we cannot deal with
        // that protocol (it would be too much work to be backward compatible)
        //
        prinbee::binary_message::pointer_t error_msg(std::make_shared<prinbee::binary_message>());
        error_msg->create_error_message(
              msg
            , prinbee::err_code_t::ERR_CODE_PROTOCOL_UNSUPPORTED
            , "protocol \""
            + r.f_protocol_version
            + "\" not supported.");
        send_message(peer, error_msg);
        return true;
    }

    snapdev::timespec_ex now(snapdev::now());
    snapdev::timespec_ex diff(now - r.f_now);
    if(diff < static_cast<std::int64_t>(0))
    {
        diff = -diff;
    }
    if(diff >= 0.01) // 10ms or more is bad for the database
    {
        prinbee::binary_message::pointer_t error_msg(std::make_shared<prinbee::binary_message>());
        error_msg->create_error_message(
              msg
            , prinbee::err_code_t::ERR_CODE_TIME_DIFFERENCE_TOO_LARGE
            , "time difference too large: "
            + diff.to_string("%s.%N")
            + " seconds.");
        send_message(peer, error_msg);
        return true;
    }

    connection_reference::pointer_t ref = find_connection_reference(peer);
    if(ref == nullptr)
    {
        prinbee::binary_message::pointer_t error_msg(std::make_shared<prinbee::binary_message>());
        error_msg->create_error_message(
              msg
            , prinbee::err_code_t::ERR_CODE_UNKNOWN_PEER
            , "peer \""
            + r.f_name
            + "\" not found in proxy list of clients.");
        send_message(peer, error_msg);
        return true;
    }

    peer->set_name(r.f_name);

    ref->set_protocol(their_protocol);

    send_acknowledgment(peer, msg, 0);

    return true;
}


bool proxy::msg_forward(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    snapdev::NOT_USED(peer);

    // TODO: add to journal & make sure we add "expected replies" so when
    //       the server replies, we know what to do with that
    //
    if(!f_daemon_connections.empty())
    {
        f_daemon_connections.begin()->second->send_message(msg);
    }

    return true;
}


//bool proxy::msg_acknowledge(
//      ed::connection::pointer_t peer
//    , prinbee::binary_message::pointer_t msg)
//{
//    prinbee::msg_acknowledge_t ack;
//    if(!msg->deserialize_acknowledge_message(ack))
//    {
//        return true;
//    }
//
//    payload_t::pointer_t other_payload;
//    {
//        auto it(f_expected_acknowledgment.find(ack.f_serial_number));
//        if(it == f_expected_acknowledgment.end())
//        {
//            return true;
//        }
//        other_payload = it->second;
//        f_expected_acknowledgment.erase(it);
//    }
//    other_payload->set_acknowledged_by(ack.f_serial_number, payload->f_peer);
//    push_payload(other_payload);
//
//    return true;
//}


bool proxy::msg_ping(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg)
{
    prinbee::binary_message::pointer_t pong(std::make_shared<prinbee::binary_message>());
    pong->create_pong_message(msg);
    send_message(peer, pong);

    return true;
}


void proxy::send_acknowledgment(
      ed::connection::pointer_t peer
    , prinbee::binary_message::pointer_t msg
    , std::uint32_t phase)
{
    prinbee::binary_message::pointer_t acknowledge_msg(std::make_shared<prinbee::binary_message>());
    acknowledge_msg->create_acknowledge_message(msg, phase);
    send_message(peer, acknowledge_msg);
}


/** \brief Called whenever we receive the STOP command or equivalent.
 *
 * This function makes sure the prinbee daemon exits as quickly as
 * possible. This means unregistering all the daemon's connections
 * from the communicator.
 *
 * If possible, the function sends an UNREGISTER message to the
 * communicator daemon.
 *
 * \param[in] quitting  Set to true if we received a QUITTING message (false
 * usually means we received a STOP message).
 */
void proxy::stop(bool quitting)
{
    if(f_messenger != nullptr)
    {
        f_messenger->unregister_fluid_settings(quitting);
        f_communicator->remove_connection(f_messenger);
        f_messenger.reset();
    }

    if(f_interrupt != nullptr)
    {
        f_communicator->remove_connection(f_interrupt);
        f_interrupt.reset();
    }

    if(f_listener != nullptr)
    {
        f_communicator->remove_connection(f_listener);
        f_listener.reset();
    }

    if(f_ping_pong_timer != nullptr)
    {
        f_communicator->remove_connection(f_ping_pong_timer);
        f_ping_pong_timer.reset();
    }

// TODO: also close all the node_client connections
//
// TODO: also stop the worker threads (that is, we need to stop adding more
//       work and once the FIFO is empty & all the threads are waiting we
//       can then stop the whole thing...)
}




} // namespace prinbee_proxy
// vim: ts=4 sw=4 et
