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
 *                          | Communicator Daemon + Binary Connection
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
 * ** daemon connection (application can communicate with the daemon)
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
 * good shape mean that everything is working as expected.
 *
 * The following shows the exchange of messages between the client, proxy
 * and daemon (prinbeed). On startup, a client explicitly sends a
 * PRINBEE_GET_STATUS, that way it is immediately sent a
 * PRINBEE_CURRENT_STATUS message. The proxy will automatically sends
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
 * ... [label="proxy tells the client something changed"];
 * proxy->communicatord [label="PRINBEE_CURRENT_STATUS"];
 * communicatord->client [label="PRINBEE_CURRENT_STATUS"];
 * ... [label="proxy explicitly asks the daemon"];
 * proxy->communicatord [label="PRINBEE_GET_STATUS"];
 * communicatord->prinbeed [label="PRINBEE_GET_STATUS"];
 * prinbeed->communicatord [label="PRINBEE_CURRENT_STATUS"];
 * communicatord->proxy [label="PRINBEE_CURRENT_STATUS"];
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
 */


// self
//
#include    "prinbeed.h"



// prinbee
//
//#include    <prinbee/exception.h>
//#include    <prinbee/names.h>
#include    <prinbee/version.h>


// communicatord
//
//#include    <communicatord/flags.h>
//#include    <communicatord/names.h>
//
//
//// as2js
////
//#include    <as2js/json.h>
//
//
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


void prinbeed::set_ipwall_status(bool status)
{
    if(status
    && !f_ipwall_is_up)
    {
        f_ipwall_is_up = true;
        start_binary_connection();
    }
}


/** \brief Run the prinbee daemon.
 *
 * This function is the core function of the daemon. It runs the loop
 * used to lock processes from any number of computers that have access
 * to the prinbee daemon network.
 *
 * \sa add_connections()
 */
int prinbeed::run()
{
    SNAP_LOG_INFO
        << "--------------------------------- prinbeed started."
        << SNAP_LOG_SEND;

    // now run our listening loop
    //
    f_communicator->run();

    return 0;
}




/** \brief Check whether the binary connection can be started and if so open it.
 *
 * This function starts listening for binary connections from proxies.
 *
 * The function does nothing if the following conditions are met:
 *
 * \li the binary connection already exists
 * \li the system needs to be ready (we did not yet receive the READY message)
 * \li the firewall is not yet in place (we did not yet received a "favorable"
 *     status from the ipwall service via the IPWALL_CURRENT_STATUS message)
 */
void prinbeed::start_binary_connection()
{
    // already connected?
    //
    if(f_proxy_listener != nullptr
    && f_node_listener != nullptr)
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

    // is the firewall in place?
    //
    if(!f_ipwall_is_up)
    {
        return;
    }

    // we want the my-address to be defined in case the user wants that as
    // the address to use to open the ports; this gets defined when we
    // receive the READY message from the communicator daemon
    //
    addr::addr my_address(f_messenger->get_my_address());
    if(my_address.is_default())
    {
        return;
    }

    // the daemon is ready to listen for connections, open the ports
    //
    addr::addr listen_address(addr::string_to_addr(
                          f_opts.get_string("proxy_listen")
                        , std::string()
                        , PROXY_BINARY_PORT));
    if(listen_address.is_default())
    {
        my_address.set_port(listen_address.get_port());
        listen_address = my_address;
    }

    // Note: I use an intermediate pointer so if the second allocation
    //       throws, we can avoid saving a half opened server
    //
    // TODO: add support for TLS connections
    //
    prinbee::binary_server::pointer_t proxy_listener(std::make_shared<prinbee::binary_server>(listen_address));

    listen_address = addr::string_to_addr(
                          f_opts.get_string("node_listen")
                        , std::string()
                        , NODE_BINARY_PORT);
    if(listen_address.is_default())
    {
        my_address.set_port(listen_address.get_port());
        listen_address = my_address;
    }

    f_node_listener = std::make_shared<prinbee::binary_server>(listen_address);

    f_proxy_listener = proxy_listener;
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
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
