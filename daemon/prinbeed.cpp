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
 * \brief Daemon managing prinbee data in the backend.
 *
 * Prinbee uses several layers to transport the data from your front
 * end applications to the backend where the database actually resides.
 * The Prinbee Daemon is part of the backend. One daemon represents one
 * Node. It manages the actual configuration files, the tables, the
 * indexes, and any other metadata that the application supports.
 *
 * \code
 *   Your Application + Prinbee Library
 *                          |    |
 *                          |    +---> Application Journal
 *                          |
 *                          | Communicator Proxy + Binary Connection
 *                          v
 *      Prinbee Proxy + Prinbee Library
 *                          |    |
 *                          |    +---> Local Journal
 *                          |
 *                          | Communicator Daemon + Binary Connections
 *                          v
 *     Prinbee Daemon + Prinbee Library
 *                          |    |
 *                          |    +---> Remote Journal
 *                          |
 *                          | Communicator Daemon + Binary Connections
 *                          v
 *     Prinbee Daemon + Prinbee Library (replication/reliability)
 * \endcode
 *
 * The daemon is fully managed for you through the Prinbee Library.
 * The two main messages used over the Communicator Daemon are the
 * `PRINBEE_GET_STATUS` and `PRINBEE_CURRENT_STATUS`. The library
 * tracks the current status of each Prinbee element and that message
 * is used to communicate that status between each service. Specifically,
 * it tracks:
 *
 * * the state of the journals (application, local, remote)
 * ** on/off
 * ** percent used
 * ** in error
 * * the state of the transport between the application and the daemon
 * ** not connected (application journal only)
 * ** proxy connection (application can communicate with the proxy)
 * ** daemon connection (application can communicate with a daemon)
 * * the state of the cluster of Nodes in terms of connections
 * ** not connected
 * ** connected
 * ** quorum
 * ** complete
 * * the state of the cluster in terms other than connections
 * ** healthy
 * ** cpu load (we get that through sitter anyway?)
 * ** disk percent used
 * ** in error
 *
 * When the cluster status is quorum or complete the network is in place.
 * When state of each journal and node is healthy, then we are 100% in
 * good shape meaning that everything is working as expected.
 *
 * The following shows the exchange of messages between the client, proxy
 * and daemon (prinbeed). On startup, a client explicitly sends a
 * PRINBEE_GET_STATUS, that way it is immediately sent a
 * PRINBEE_CURRENT_STATUS message. The proxy automatically sends
 * further PRINBEE_CURRENT_STATUS as the status changes over time.
 * A similar process happens between the proxy and the daemon.
 *
 * \msc
 * client,communicatord,proxy,prinbeed;
 *
 * ... [label="client explicitly asks the proxy"];
 * client->communicatord [label="PRINBEE_GET_STATUS"];
 * communicatord->proxy [label="PRINBEE_GET_STATUS"];
 * proxy->communicatord [label="PRINBEE_CURRENT_STATUS"];
 * communicatord->client [label="PRINBEE_CURRENT_STATUS"];
 *
 * ... [label="proxy tells the client something changed"];
 * proxy->communicatord [label="PRINBEE_CURRENT_STATUS"];
 * communicatord->client [label="PRINBEE_CURRENT_STATUS"];
 *
 * ... [label="proxy explicitly asks the daemon"];
 * proxy->communicatord [label="PRINBEE_GET_STATUS"];
 * communicatord->prinbeed [label="PRINBEE_GET_STATUS"];
 * prinbeed->communicatord [label="PRINBEE_CURRENT_STATUS"];
 * communicatord->proxy [label="PRINBEE_CURRENT_STATUS"];
 *
 * ... [label="daemon tells the proxy something changed"];
 * prinbeed->communicatord [label="PRINBEE_CURRENT_STATUS"];
 * communicatord->proxy [label="PRINBEE_CURRENT_STATUS"];
 * \endmsc
 *
 * To tweak things in the backend, use one of the front ends. We provide
 * a CLI with the pbql language to make it easy to start and various
 * client classes in the prinbee library.
 *
 * The clients and servers first use this communicator connection to know
 * how to connect to Prinbee using a binary connection. The binary
 * connection is useful to make things a lot faster, especially when large
 * blocks of data get shared.
 *
 * A client connects by sending a `PRINBEE_STATUS` message. At that
 * point, the server replies with a `PRINBEE_READY` which include the
 * IP and port to use to connect to this Prinbee daemon directly
 * (i.e. the binary connection).
 *
 * If the Prinbee daemon is not ready, then it sends the `NO_PRINBEE`
 * message instead. Once ready, it will then send a `PRINBEE_READY`
 * and all the clients can then connect.
 *
 * \msc
 * client,communicatord,prinbeed;
 *
 * client->communicatord [label="CONNECT_TO_PRINBEE"];
 * communicatord->prinbeed [label="CONNECT_TO_PRINBEE"];
 * prinbeed->communicatord [label="PRINBEE_CLIENT_LISTENER"];
 * communicatord->client [label="PRINBEE_CLIENT_LISTENER"];
 *
 * client->prinbeed [label="<direct connection to binary port>"];
 * \endmsc
 *
 * Once a server is ready, it wants to connect to all the other servers.
 * To do so it broadcasts the `INTERCONNECT` message to all the other
 * prinbeed servers. The `INTERCONNECT` message includes the
 * binary connection information (IP and port). When a server receives
 * the `INTERCONNECT` message it compares its own address with the address
 * of the sender. If the IP address of the other server is smaller, then it
 * uses it to connect, otherwise it replies with an `INTERCONNECT` message
 * sent directly to that other server to make sure a connection happens.
 *
 * \msc
 * prinbeed1,communicatord,prinbeed2;
 *
 * prinbeed1->communicatord [label="INTERCONNECT"];
 * communicatord->prinbeed2 [label="INTERCONNECT"];
 *
 * ---  [label="If prinbee2.IP > prinbee1.IP"];
 * prinbeed2->prinbeed1 [label="<direct connection to binary port>"];
 *
 * ---  [label="If prinbee2.IP < prinbee1.IP"];
 * prinbeed2->communicatord [label="INTERCONNECT"];
 * communicatord->prinbeed1 [label="INTERCONNECT"];
 * prinbeed1->prinbeed2 [label="<direct connection to binary port>"];
 * \endmsc
 *
 * The client and server connections make use of a similar protocol,
 * however, the servers do not send orders like clients do (i.e. no SELECT,
 * UDPATE, INSERT...). Instead, the servers connect to each other to
 * maintain the replication of the data (table data, index data). It is
 * also in charge of the messages necessary when morphing a cluster.
 * (i.e. each time you add a few more nodes to your cluster, the backends
 * react by spreading the data further between all the nodes.)
 *
 * The protocol uses structures which are transformed to binary using the
 * `prinbee/data/structure.*` classes and types. Each message is preceded
 * by one \em header structure which defines the structure type and size.
 *
 * The eventdispatcher setup includes the following:
 *
 * * interrupt -- intercept Ctrl-C and transform it in a STOP message
 * * messenger -- connection between prinbeed and communicatord
 * * node_client -- connection initiated by a prinbeed to another prinbeed
 * * node_listener -- listen for connections from another prinbeed
 * * proxy_listener -- listen for connections from a proxy
 * * direct_listener -- listen for connections directly from a client
 * * prinbee::binary_server_client -- binary connection (prinbeed, proxy, or client)
 *
 * The prinbee::binary_server_client gets used for incoming connections
 * from node_listener, proxy_listener, direct_listener.
 */


// self
//
#include    "prinbeed.h"

#include    "connection_reference.h"
#include    "node_client.h"
#include    "node_listener.h"
#include    "proxy_listener.h"
#include    "direct_listener.h"



// prinbee
//
#include    <prinbee/exception.h>
#include    <prinbee/names.h>
#include    <prinbee/version.h>


// communicatord
//
//#include    <communicatord/flags.h>
#include    <communicatord/names.h>


// cluck
//
#include    <cluck/cluck_status.h>


// cppprocess
//
#include    <cppprocess/io_capture_pipe.h>
#include    <cppprocess/process.h>


//// eventdispatcher
////
//#include    <eventdispatcher/names.h>


// libaddr
//
#include    <libaddr/addr_parser.h>


// snapdev
//
//#include    <snapdev/gethostname.h>
//#include    <snapdev/hexadecimal_string.h>
#include    <snapdev/stringize.h>
//#include    <snapdev/tokenize_string.h>
//#include    <snapdev/to_string_literal.h>


// snaplogger
//
//#include    <snaplogger/logger.h>
#include    <snaplogger/options.h>
//#include    <snaplogger/severity.h>


// advgetopt
//
//#include    <advgetopt/advgetopt.h>
#include    <advgetopt/exception.h>


//// C++
////
//#include    <algorithm>
//#include    <iostream>
//#include    <sstream>
//
//
//// openssl
////
//#include    <openssl/rand.h>


// last include
//
#include    <snapdev/poison.h>






namespace prinbee_daemon
{


namespace
{



advgetopt::option const g_options[] =
{
    advgetopt::define_option(
          advgetopt::Name("cluster-name")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Specify the name of this prinbee cluster.")
        , advgetopt::DefaultValue("prinbee")
    ),
    advgetopt::define_option(
          advgetopt::Name("proxy-listen")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Specify an address and port to listen on for proxy connections; if the IP is not defined or set to ANY, then only the port is used and this computer public IP address is used.")
        , advgetopt::DefaultValue(":4010")
    ),
    advgetopt::define_option(
          advgetopt::Name("node-listen")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("Specify an address and port to listen on for node connections; if the IP is not defined or set to ANY, then only the port is used and this computer public IP address is used.")
        , advgetopt::DefaultValue(":4011")
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
    "/etc/prinbee/prinbeed.conf",
    nullptr
};


advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "prinbeed",
    .f_group_name = "prinbee",
    .f_options = g_options,
    .f_environment_variable_name = "PRINBEED_OPTIONS",
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













/** \class prinbeed
 * \brief Class handling Prinbee database files on the backend.
 *
 * This class is used to manage all the files that Prinbee handles on the
 * backend. This is the set of contexts, their schemata, tables, indexes,
 * data files, etc.
 *
 * There should be only one daemon running per computer. The daemon connects
 * to other daemons as required and may use worker threads to do work that
 * take a long time.
 *
 * Most of the writes end up in a Journal. Then the daemon acknowledge
 * steps as they happen (i.e. received, saved in the journal, written
 * to table data file, when replication is involved, the replication
 * level, indexed, and errors along the way if any).
 *
 * \note
 * Most of the work is done by calling functions in the library. This
 * daemon is mainly used to do all the communication between the parties.
 */



/** \brief Initializes a prinbee object.
 *
 * This function parses the command line arguments, reads configuration
 * files, setups the messenger, and the logger.
 *
 * \param[in] argc  The number of arguments in the argv array.
 * \param[in] argv  The array of argument strings.
 */
prinbeed::prinbeed(int argc, char * argv[])
    : f_opts(g_options_environment)
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
        throw advgetopt::getopt_exit("logger options generated an error.", 0);
    }
}


/** \brief Do some clean ups.
 *
 * At this point, the destructor is present mainly because we have
 * some virtual functions.
 */
prinbeed::~prinbeed()
{
}


/** \brief Finish the prinbee daemon initialization.
 *
 * This function creates all the connections used by the prinbee daemon.
 *
 * \note
 * This is separate from the run() function so we can run unit tests
 * against the prinbee daemon.
 *
 * \sa run()
 */
void prinbeed::add_connections()
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
void prinbeed::set_ipwall_status(bool status)
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
void prinbeed::register_prinbee_daemon(ed::message & msg)
{
    if(!msg.has_parameter(communicatord::g_name_communicatord_param_status))
    {
        return;
    }

    if(msg.get_parameter(communicatord::g_name_communicatord_param_status)
                            != communicatord::g_name_communicatord_value_up)
    {
        return;
    }

// TODO: I think we have an array of IPs and also we only
//       want to connect to another prinbeed if its IP is smaller
    if(msg.has_parameter(prinbee::g_name_prinbee_param_node_ip))
    {
        std::string const node_address(msg.get_parameter(prinbee::g_name_prinbee_param_node_ip));
        addr::addr const a(addr::string_to_addr(
                          node_address
                        , std::string()
                        , NODE_BINARY_PORT));
        switch(a.get_network_type())
        {
        case addr::network_type_t::NETWORK_TYPE_PUBLIC:
        case addr::network_type_t::NETWORK_TYPE_PRIVATE:
        case addr::network_type_t::NETWORK_TYPE_LOOPBACK:
            break;

        default:
            throw prinbee::invalid_address("the other node address is not a valid address.");

        }
        std::string const name(msg.get_parameter(prinbee::g_name_prinbee_param_name));
        connect_to_node(a, name);
    }


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
void prinbeed::set_clock_status(bool status)
{
    if(status
    && !f_stable_clock)
    {
        f_stable_clock = true;
        start_binary_connection();
    }
}


/** \brief Save the current lock status.
 *
 * Whenever we start the prinbee daemon, we want to be able to lock
 * multiple computers whenever certain operations happen. In particular,
 * changes to the schema have to happen in synchrony so make sure it
 * happens on all computers concerned by said schema.
 */
void prinbeed::lock_status_changed()
{
    if(!f_lock_ready
    && cluck::is_lock_ready())
    {
        f_lock_ready = true;
        start_binary_connection();
    }
}


/** \brief Run the prinbee daemon.
 *
 * This function is the core function of the daemon. It runs the loop
 * used to accept messenger and direct binary connections between the
 * database daemon (prinbeed) and proxy.
 *
 * \sa add_connections()
 */
int prinbeed::run()
{
    SNAP_LOG_INFO
        << "--------------------------------- prinbeed started."
        << SNAP_LOG_SEND;

    is_ipwall_installed();

    // now run our listening loop
    //
    f_communicator->run();

    return 0;
}


/** \brief Check whether we can find iplock.
 *
 * The system checks whether the firewall is in place by waiting for the
 * ipload and ipwall to be ready. This works by sending an IPWALL_GET_STATUS
 * message and expecting the IPWALL_CURRENT_STATUS as the reply.
 *
 * Only, the iplock services depends on the prinbee services. This is because
 * the iplock system wants to have access to a table in the database. So to
 * make things work, we instead use the messages. However, the messages will
 * never happen if the packages weren't installed, which is a possible
 * installation scheme. After all, if you have prinbee installed on a
 * back end system with firewall on other computers, it would not be
 * required to have iplock running on its own computers.
 *
 * So here we check whether the iplock system is installed. If not, we'll
 * skip on the IPWALL_CURRENT_STATUS altogether.
 */
void prinbeed::check_ipwall_status()
{
    cppprocess::process p("is ipwall active?");
    p.set_command("systemctl");
    p.add_argument("is-enabled");
    p.add_argument("ipwall");
    cppprocess::io_capture_pipe::pointer_t out(std::make_shared<cppprocess::io_capture_pipe>());
    p.set_output_io(out);
    int r(p.start());
    if(r == 0)
    {
        r = p.wait();
    }
    SNAP_LOG_VERBOSE
        << '"'
        << p.get_command_line()
        << "\" query output ("
        << r
        << "): "
        << out->get_trimmed_output()
        << SNAP_LOG_SEND;

    f_ipwall_is_installed = r == 0;
}


bool prinbeed::is_ipwall_installed() const
{
    return f_ipwall_is_installed;
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
void prinbeed::start_binary_connection()
{
    // already connected?
    //
    if(f_node_listener != nullptr
    && f_proxy_listener != nullptr
    && f_direct_listener != nullptr)
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
    if(!f_messenger->is_registered())
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
    // so the clock has to be up and running properly on each system
    //
    if(!f_stable_clock)
    {
        return;
    }

    // check whether the lock system (cluckd) is ready
    //
    // many operations require a lock so we must make sure that the lock
    // is ready before we start (we may actually ease this later because
    // we probably want the journal to be active even without the lock
    // since that's a local operation and thus cluckd is not required)
    //
    if(!f_lock_ready)
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

    // the daemon is ready to listen for connections, open the ports
    //
    addr::addr node_address(addr::string_to_addr(
                          f_opts.get_string("node_listen")
                        , std::string()
                        , NODE_BINARY_PORT));
    my_address.set_port(node_address.get_port());
    if(node_address.is_default())
    {
        node_address = my_address;
    }
    f_node_address = my_address.to_ipv4or6_string(
                          addr::STRING_IP_ADDRESS
                        | addr::STRING_IP_BRACKET_ADDRESS
                        | addr::STRING_IP_PORT);

    addr::addr proxy_address(addr::string_to_addr(
                          f_opts.get_string("proxy_listen")
                        , std::string()
                        , PROXY_BINARY_PORT));
    my_address.set_port(proxy_address.get_port());
    if(proxy_address.is_default())
    {
        proxy_address = my_address;
    }
    f_proxy_address = my_address.to_ipv4or6_string(
                          addr::STRING_IP_ADDRESS
                        | addr::STRING_IP_BRACKET_ADDRESS
                        | addr::STRING_IP_PORT);

    addr::addr direct_address(addr::string_to_addr(
                          f_opts.get_string("direct_listen")
                        , std::string()
                        , DIRECT_BINARY_PORT));
    my_address.set_port(direct_address.get_port());
    if(direct_address.is_default())
    {
        direct_address = my_address;
    }
    f_direct_address = my_address.to_ipv4or6_string(
                          addr::STRING_IP_ADDRESS
                        | addr::STRING_IP_BRACKET_ADDRESS
                        | addr::STRING_IP_PORT);

    // TODO: add support for TLS connections
    //
    f_node_listener = std::make_shared<node_listener>(this, node_address);
    f_communicator->add_connection(f_node_listener);

    f_proxy_listener = std::make_shared<proxy_listener>(this, proxy_address);
    f_communicator->add_connection(f_proxy_listener);

    f_direct_listener = std::make_shared<direct_listener>(this, direct_address);
    f_communicator->add_connection(f_direct_listener);

    // request the current status of the prinbee cluster
    //
    ed::message prinbee_get_status;
    prinbee_get_status.set_command(prinbee::g_name_prinbee_cmd_prinbee_get_status);
    prinbee_get_status.set_service(prinbee::g_name_prinbee_service_prinbee);
    prinbee_get_status.set_server(communicatord::g_name_communicatord_service_private_broadcast);
    prinbee_get_status.add_parameter(
              communicatord::g_name_communicatord_param_cache
            , communicatord::g_name_communicatord_value_no);
    f_messenger->send_message(prinbee_get_status);

    // we also need to send our status to everyone else
    //
    send_our_status(nullptr);
}


void prinbeed::send_our_status(ed::message * msg)
{
    // we send our status which generates an equivalent of a gossip message
    // since this message is used to interconnect all the prinbee daemons
    // and prinbee proxies in a cluster
    //
    ed::message prinbee_current_status;
    prinbee_current_status.set_command(prinbee::g_name_prinbee_cmd_prinbee_current_status);
    if(msg == nullptr)
    {
        prinbee_current_status.set_service(communicatord::g_name_communicatord_service_private_broadcast);
    }
    else
    {
        prinbee_current_status.reply_to(*msg);
    }

    prinbee_current_status.add_parameter(
              prinbee::g_name_prinbee_param_name
            , f_opts.get_string("cluster_name"));
    prinbee_current_status.add_parameter(
              communicatord::g_name_communicatord_param_cache
            , communicatord::g_name_communicatord_value_no);

    if(f_node_address.empty()
    || f_proxy_address.empty()
    || f_direct_address.empty())
    {
        prinbee_current_status.add_parameter(
                  communicatord::g_name_communicatord_param_status
                , communicatord::g_name_communicatord_value_down);
    }
    else
    {
        prinbee_current_status.add_parameter(
                  communicatord::g_name_communicatord_param_status
                , communicatord::g_name_communicatord_value_up);
        prinbee_current_status.add_parameter(
                  prinbee::g_name_prinbee_param_node_ip
                , f_node_address);
        prinbee_current_status.add_parameter(
                  prinbee::g_name_prinbee_param_proxy_ip
                , f_proxy_address);
        prinbee_current_status.add_parameter(
                  prinbee::g_name_prinbee_param_direct_ip
                , f_direct_address);
    }

    f_messenger->send_message(prinbee_current_status);
}


void prinbeed::connect_to_node(addr::addr const & a, std::string const & name)
{
    node_client::pointer_t n(std::make_shared<node_client>(this, a));
    n->add_callbacks();

    register_connection(n, name);
}


void prinbeed::register_connection(ed::connection::pointer_t c, std::string const & name)
{
    connection_reference::pointer_t ref(std::make_shared<connection_reference>());
    ref->set_name(name);
    ref->set_connection(c);
    f_connection_reference[name] = ref;
}


bool prinbeed::msg_err(
      ed::connection::pointer_t //peer
    , prinbee::binary_message & )//msg)
{
    return true;
}


bool prinbeed::msg_register(
      ed::connection::pointer_t //peer
    , prinbee::binary_message & msg)
{
    prinbee::register_t r;
    if(!msg.deserialize_register_message(r))
    {
        return true;
    }

    // TODO: actually implement the register message

    return true;
}


bool prinbeed::msg_ping(
      ed::connection::pointer_t peer
    , prinbee::binary_message & msg)
{
    snapdev::NOT_USED(msg);

    prinbee::binary_message pong;
    pong.set_name(prinbee::g_message_pong);
    prinbee::binary_client::pointer_t client(std::dynamic_pointer_cast<prinbee::binary_client>(peer));
    if(client != nullptr)
    {
        client->send_message(pong);
    }
    else
    {
        prinbee::binary_server_client::pointer_t server_client(std::dynamic_pointer_cast<prinbee::binary_server_client>(peer));
        if(server_client != nullptr)
        {
            server_client->send_message(pong);
        }
        else
        {
            throw prinbee::logic_error("unknown peer type, cannot send message to it.");
        }
    }

    return true;
}


bool prinbeed::msg_pong(
      ed::connection::pointer_t //peer
    , prinbee::binary_message & )//msg)
{
    return true;
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
void prinbeed::stop(bool quitting)
{
    if(f_messenger != nullptr)
    {
        f_messenger->unregister_fluid_settings(quitting);
        f_communicator->remove_connection(f_messenger);
        f_messenger.reset();
    }

    if(f_communicator != nullptr)
    {
        f_communicator->remove_connection(f_interrupt);
        f_interrupt.reset();
    }

    if(f_node_listener != nullptr)
    {
        f_communicator->add_connection(f_node_listener);
        f_node_listener.reset();
    }

    if(f_proxy_listener != nullptr)
    {
        f_communicator->add_connection(f_proxy_listener);
        f_proxy_listener.reset();
    }

    if(f_direct_listener != nullptr)
    {
        f_communicator->add_connection(f_direct_listener);
        f_direct_listener.reset();
    }

// TODO: also close all the node_client connections
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
