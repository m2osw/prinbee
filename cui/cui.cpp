// Copyright (c) 2011-2025  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/communicatord
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


// self
//
#include    "cui.h"



// prinbee
//
#include    <prinbee/names.h>
#include    <prinbee/network/ports.h>
#include    <prinbee/pbql/parser.h>
#include    <prinbee/version.h>


// communicatord
//
//#include    <communicatord/communicator.h>
#include    <communicator/names.h>


//// eventdispatcher
////
//#include    <eventdispatcher/cui_connection.h>
//#include    <eventdispatcher/local_stream_client_message_connection.h>
//#include    <eventdispatcher/local_dgram_server_message_connection.h>
//#include    <eventdispatcher/tcp_client_message_connection.h>
//#include    <eventdispatcher/udp_server_message_connection.h>
//
//
//// edhttp
////
//#include    <edhttp/uri.h>


// snaplogger
//
#include    <snaplogger/message.h>
#include    <snaplogger/options.h>


// libaddr
//
#include    <libaddr/addr_parser.h>
//#include    <libaddr/iface.h>


// snapdev
//
#include    <snapdev/file_contents.h>
#include    <snapdev/math.h>
#include    <snapdev/stringize.h>


// advgetopt
//
#include    <advgetopt/advgetopt.h>
//#include    <advgetopt/conf_file.h>
#include    <advgetopt/exception.h>
#include    <advgetopt/validator_duration.h>


//// C++
////
//#include    <atomic>
//
//
//// ncurses
////
//#include    <ncurses.h>
//
//
//// readline
////
//#include    <readline/readline.h>
//#include    <readline/history.h>


// last include
//
#include    <snapdev/poison.h>



/** \file
 * \brief A tool to interface with the Prinbee database using PBQL.
 *
 * This tool understands PBQL. It opens an interactive prompt where
 * you can enter PBQL commands. Alternatively, you can use the \c --command
 * command line option to run that one command and exit or the \c --file
 * to run all the commands found in a named file (a .pbql script).
 */



namespace prinbee_cui
{
namespace
{



const advgetopt::option g_command_line_options[] =
{
    // TBD
    // at the moment we expect the IP:port info from the proxy status message
    // so we do not need this... but maybe later we want to support this...
    // (in case we want a direct connection or no proxy is running locally)
    //
    //advgetopt::define_option(
    //      advgetopt::Name("address")
    //    , advgetopt::ShortName('a')
    //    , advgetopt::Flags(advgetopt::all_flags<
    //          advgetopt::GETOPT_FLAG_GROUP_OPTIONS
    //        , advgetopt::GETOPT_FLAG_REQUIRED>())
    //    , advgetopt::Help("the address and port to connect to a Prinbee proxy (i.e. \"127.0.0.1:4013\").")
    //    //, advgetopt::DefaultValue(":4013") -- there is no default, by default we send a message using the communicator daemon to retrieve the local proxy info
    //),
    advgetopt::define_option(
          advgetopt::Name("command")
        , advgetopt::ShortName('c')
        , advgetopt::Flags(advgetopt::any_flags<
              advgetopt::GETOPT_FLAG_GROUP_COMMANDS
            , advgetopt::GETOPT_FLAG_REQUIRED
            , advgetopt::GETOPT_FLAG_COMMAND_LINE>())
        , advgetopt::Help("if defined, run this command and then exit.")
    ),
    advgetopt::define_option(
          advgetopt::Name("documentation")
        , advgetopt::Flags(advgetopt::all_flags<
              advgetopt::GETOPT_FLAG_GROUP_OPTIONS
            , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("path to the .hlp files.")
        , advgetopt::DefaultValue("/usr/share/doc/prinbee/cui")
    ),
    advgetopt::define_option(
          advgetopt::Name("file")
        , advgetopt::ShortName('f')
        , advgetopt::Flags(advgetopt::any_flags<
              advgetopt::GETOPT_FLAG_GROUP_COMMANDS
            , advgetopt::GETOPT_FLAG_REQUIRED
            , advgetopt::GETOPT_FLAG_COMMAND_LINE>())
        , advgetopt::Help("if defined, run the commands found in the specified file and then exit.")
    ),
    advgetopt::define_option(
          advgetopt::Name("interactive")
        , advgetopt::ShortName('i')
        , advgetopt::Flags(advgetopt::any_flags<
              advgetopt::GETOPT_FLAG_GROUP_COMMANDS
            , advgetopt::GETOPT_FLAG_FLAG
            , advgetopt::GETOPT_FLAG_COMMAND_LINE>())
        , advgetopt::Help("if defined, open a prompt; this is the default if no --command or --file is specified.")
    ),
    advgetopt::define_option(
          advgetopt::Name("ping-pong-interval")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_REQUIRED
                    , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("How often to send a PING to all the daemons.")
        , advgetopt::Validator("duration")
        , advgetopt::DefaultValue("5s")
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


char const * const g_configuration_directories[] =
{
    "/etc/prinbee",
    nullptr
};


advgetopt::options_environment const g_command_line_options_environment =
{
    .f_project_name = "pbql",
    .f_options = g_command_line_options,
    .f_environment_variable_name = "PBQL",
    .f_configuration_filename = "pbql.conf",
    .f_configuration_directories = g_configuration_directories,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [-<opt>] [-c <command>]\n"
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




cui::cui(int argc, char * argv[])
    : f_opts(g_command_line_options_environment)
{
    snaplogger::add_logger_options(f_opts);

    // before we can parse command line arguments, we must create the
    // fluid settings & communicator client objects which happen to
    // dynamically add command line options to f_opts
    //
    f_messenger = std::make_shared<messenger>(this, f_opts);

    f_opts.finish_parsing(argc, argv);
    if(!snaplogger::process_logger_options(
              f_opts
            , "/etc/prinbee/logger"
            , std::cout
            , false))
    {
        // exit on any error
        //
        throw advgetopt::getopt_exit("logger options generated an error.", 1);
    }

    if(f_opts.is_defined("command"))
    {
        f_command = f_opts.get_string("command");
    }
    if(f_opts.is_defined("file"))
    {
        f_file = f_opts.get_string("file");
    }

    if(f_command.empty()
    && f_file.empty())
    {
        // this is the default is no --command and no --file was specified
        // whether or not the user specified --interactive
        //
        f_interactive = true;
    }
    else
    {
        f_interactive = f_opts.is_defined("interactive");
    }

    int count(0);
    if(!f_command.empty())
    {
        ++count;
    }
    if(!f_file.empty())
    {
        ++count;
    }
    if(f_interactive)
    {
        ++count;
    }
    if(count >= 2)
    {
        std::cerr << "error: --command, --file, and --interactive are mutually exclusive." << std::endl;
        throw advgetopt::getopt_exit("error: the --command, --file, and --interactive command line options are mutually exclusive; only one of them can be specified.", 1);
    }
}


int cui::run()
{
    f_communicator = ed::communicator::instance();

    // we first initialize connections before the f_messenger will parse
    // additional command line parameters and we want that to happen early
    //
    if(!init_connections())
    {
        return 1;
    }

    if(f_interactive)
    {
        if(!init_console_connection())
        {
            return 1;
        }
    }
    if(!f_file.empty())
    {
        if(!init_file())
        {
            return 1;
        }
    }

SNAP_LOG_ERROR << "start communicator run()" << SNAP_LOG_SEND;
    if(ed::communicator::instance()->run())
    {
        return 0;
    }

    // run() returned with an error
    //
    std::cerr << "error: something went wrong in the ed::communicator::run() loop." << std::endl;
    return 1;
}


bool cui::init_connections()
{
    // capture Ctrl-C (SIGINT) to get a clean exit by default
    //
    f_interrupt = std::make_shared<interrupt>(this);
    if(!f_communicator->add_connection(f_interrupt))
    {
        std::cerr << "error: could not add interrupt to list of ed::communicator connections.\n";
        return false;
    }

    // add the messenger used to communicate with the communicator daemon
    // and other services as required
    //
    if(!f_communicator->add_connection(f_messenger))
    {
        std::cerr << "error: could not add messenger to list of ed::communicator connections.\n";
        return false;
    }

    // the following call actually connects the messenger to the
    // communicator daemon
    //
    f_messenger->finish_parsing();

    return true;
}


bool cui::init_console_connection()
{
    f_console_connection = std::make_shared<console_connection>(this);
    f_console_connection->ready();
    f_console_connection->set_key_bindings();
    f_console_connection->set_documentation_path(f_opts.get_string("documentation"));
    f_console_connection->reset_prompt();
    if(!ed::communicator::instance()->add_connection(f_console_connection))
    {
        std::cerr << "error: could not add CUI console to list of ed::communicator connections.\n";
        return false;
    }

    return true;
}


bool cui::init_file()
{
    snapdev::file_contents in(f_file);
    if(!in.read_all())
    {
        std::cerr
            << "error: could not properly read the input script \""
            << f_file
            << "\" -- "
            << in.last_error()
            << ".\n";
        return false;
    }
    f_command = in.contents();

    return true;
}


void cui::msg_prinbee_proxy_current_status(ed::message & msg)
{
    f_proxy_status = "unknown";
    if(msg.has_parameter(communicator::g_name_communicator_param_status))
    {
        f_proxy_status = msg.get_parameter(communicator::g_name_communicator_param_status);
    }
    if(msg.has_parameter(prinbee::g_name_prinbee_param_proxy_ip))
    {
        f_address = msg.get_parameter(prinbee::g_name_prinbee_param_proxy_ip);

        start_binary_connection();
    }
}


bool cui::msg_process_reply(
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
            f_ready = true;

            if(!f_interactive)
            {
                // TBD: can we really just call that function from here?
                //
                execute_commands(f_command);
                f_quit = true;
            }
        }
        else
        {
            // we cannot register, trying again will fail again, what
            // to do?!
            //
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


void cui::start_binary_connection()
{
    // already connected?
    //
    if(f_proxy_connection != nullptr)
    {
        SNAP_LOG_TRACE
            << "start_binary_connection: Proxy connection already allocated."
            << SNAP_LOG_SEND;
        return;
    }

    // did we receive the READY message?
    //
    if(!f_messenger->is_ready())
    {
        SNAP_LOG_TRACE
            << "start_binary_connection: messenger not ready."
            << SNAP_LOG_SEND;
        return;
    }

    // did we receive the FLUID_SETTINGS_READY message?
    //
    if(!f_messenger->is_registered())
    {
        SNAP_LOG_TRACE
            << "start_binary_connection: fluid settings not ready."
            << SNAP_LOG_SEND;
        return;
    }

    // did we receive the proxy STATUS message?
    //
    if(f_address.empty())
    {
        SNAP_LOG_TRACE
            << "start_binary_connection: no address to the Proxy service."
            << SNAP_LOG_SEND;
        return;
    }
    addr::addr const a(addr::string_to_addr(
              f_address
            , "127.0.0.1"
            , prinbee::CLIENT_BINARY_PORT
            , "tcp"));

    // the client is ready to connect to the local proxy
    //
    f_proxy_connection = std::make_shared<proxy_connection>(this, a);
    f_proxy_connection->add_callbacks();
    f_communicator->add_connection(f_proxy_connection);

    // now that we have a proxy connection, initialize the ping-pong timer
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
            return;
        }

        ping_pong_interval = std::clamp(ping_pong_interval, 1.0, 60.0 * 60.0) * 1'000'00.0;
        f_ping_pong_timer = std::make_shared<ping_pong_timer>(this, ping_pong_interval);
        if(!f_communicator->add_connection(f_ping_pong_timer))
        {
            SNAP_LOG_RECOVERABLE_ERROR
                << "could not add ping-pong timer to list of ed::communicator connections."
                << SNAP_LOG_SEND;
        }
    }
}


/** \brief Called whenever we receive the STOP command or equivalent.
 *
 * This function makes sure the prinbee cui exits as quickly as
 * possible. This means unregistering itself from the proxy &
 * communicator daemons.
 *
 * \param[in] quitting  Set to true if we received a QUITTING message (false
 * usually means we received a STOP message).
 */
void cui::stop(bool quitting)
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

    if(f_proxy_connection != nullptr)
    {
        f_communicator->remove_connection(f_proxy_connection);
        f_proxy_connection.reset();
    }

    if(f_ping_pong_timer != nullptr)
    {
        f_communicator->remove_connection(f_ping_pong_timer);
        f_ping_pong_timer.reset();
    }

    if(f_console_connection != nullptr)
    {
        f_communicator->remove_connection(f_console_connection);

        // IMPORTANT: we must delete the console connection to remove the
        //            stdin/stdout pipes we create in there
        //
        f_console_connection.reset();
    }

//{
//ed::connection::vector_t connections(f_communicator->get_connections());
//for(auto const & c : connections)
//{
//    SNAP_LOG_ERROR << "connection left: \"" << c->get_name() << "\"." << SNAP_LOG_SEND;
//}
//}
}


void cui::send_ping()
{
    // this happens if we don't get a proxy connection in time
    //
    if(f_proxy_connection == nullptr)
    {
        return;
    }

    if(f_proxy_connection->get_expected_ping() != 0)
    {
        std::uint32_t const count(f_proxy_connection->increment_no_pong_answer());
        if(count >= MAX_PING_PONG_FAILURES)
        {
            SNAP_LOG_ERROR
                << "connection never replied from our last "
                << MAX_PING_PONG_FAILURES
                << " PING signals; reconnecting."
                << SNAP_LOG_SEND;

            // TODO: actually implement...
            //
            throw prinbee::not_yet_implemented("easy in concept, we'll implement that later though...");

            // don't send a PING now
            //
            return;
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
}


std::string cui::define_prompt()
{
    // TODO: we want to show the name of the current context
    //       but to be really cool, we should allow the user
    //       to define the prompt
    //
    return "pbql> ";
}


void cui::execute_commands(std::string const & commands)
{
    // we have several types of commands, the PBQL ones and the CUI ones;
    // the CUI ones include the HELP and options related commands, which
    // are not available to scripts; these are handled by the callback
    // (user capture function) which is not setup in the parser if the
    // system is not interactive
    //
    // although that may change over time...
    //
    std::string filename(f_file);
    if(filename.empty())
    {
        if(f_command.empty())
        {
            filename = "<command>";
        }
        else
        {
            filename = "<input>";
        }
    }

    // note: the f_lexer and f_parser may get used in callbacks so we
    // save them in member variables, but they are still considered
    // local variables and thus get reset right after parsing
    //
    prinbee::pbql::input::pointer_t in(std::make_shared<prinbee::pbql::input>(commands, filename));
    f_lexer = std::make_shared<prinbee::pbql::lexer>();
    f_lexer->set_input(in);
    f_parser = std::make_shared<prinbee::pbql::parser>(f_lexer);
    f_parser->set_user_capture(std::bind(&cui::user_commands, this, std::placeholders::_1));
    try
    {
        f_cmds = f_parser->parse();
    }
    catch(prinbee::prinbee_exception const & e)
    {
        f_console_connection->output(e.what());
    }
    f_quit = f_parser->quit();
    f_parser.reset();
    f_lexer.reset();

    // execute the command(s)
    //
    if(!f_cmds.empty())
    {
        // note that some of the work is likely async so the `cmds` probably
        // needs to be a variable member which we reduce each time a command
        // was executed...
        //

        // ... TODO ...
    }
    else if(f_quit)
    {
        // special case where there was just a QUIT; command, then the
        // list of commands is empty and the f_quit flag is true
        //
        stop(false);
    }
}


bool cui::user_commands(std::string const & command)
{
    // command is an identifier so it cannot be an empty string
    //
    switch(command[0])
    {
    case 'C':
        if(command == "CLEAR")
        {
            return parse_clear();
        }
        break;

    case 'H':
        if(command == "HELP")
        {
            return parse_help();
        }
        break;

    }

    // did not recognized that command here either
    //
    return false;
}


std::string cui::get_messenger_status() const
{
    if(f_messenger == nullptr)
    {
        // by the time this function gets called, this should never happen
        //
        return "--";
    }

    if(!f_messenger->is_connected())
    {
        if(f_messenger->is_enabled())
        {
            return "waiting";
        }
        return "connecting";
    }

    if(!f_messenger->is_ready())
    {
        return "connected";
    }

    // now it's ready
    //
    return "registered";
}


std::string cui::get_fluid_settings_status() const
{
    if(f_messenger == nullptr)
    {
        // by the time this function gets called, this should never happen
        //
        return "--";
    }

    if(!f_messenger->is_connected())
    {
        return "connecting";
    }

    if(!f_messenger->is_ready())
    {
        return "connected";
    }

    // this means the fluid settings is connected and registered with us
    //
    if(!f_messenger->is_registered())
    {
        return "ready";
    }

    return "registered";
}


std::string cui::get_proxy_status() const
{
    if(f_proxy_connection == nullptr)
    {
        // the status "down" means we receive a PRINBEE_PROXY_CURRENT_STATUS
        // message and that means the proxy service is running but not yet
        // available to receive binary connections
        //
        if(f_proxy_status == "down")
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
            // message was not acknowledge positively)
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


snapdev::timespec_ex cui::get_last_ping() const
{
    if(f_proxy_connection == nullptr)
    {
        return snapdev::timespec_ex();
    }

    return f_proxy_connection->get_last_ping();
}


std::string cui::get_prinbee_status() const
{
    if(f_proxy_connection == nullptr
    || f_proxy_connection->get_last_ping() == snapdev::timespec_ex())
    {
        return "unknown";
    }

    // we do not yet transmit the status of each backend daemon to
    // the client; the proxy needs to do that and at the moment I
    // am not too sure what I want to send other than the loadavg
    //
    std::stringstream ss;
    ss << "TODO";

    return ss.str();
}


std::string cui::get_console_status() const
{
    if(f_console_connection == nullptr)
    {
        return "close";
    }

    // I think this is not sufficient if we already sent the last commands
    // but are still waiting for the ACK or ERR reply
    //
    if(f_cmds.empty())
    {
        return "open";
    }

    // it is still running commands
    //
    return "busy";
}


bool cui::parse_clear()
{
    f_parser->expect_semi_colon("HELP COMMANDS");
    f_console_connection->clear_output();

    return true;
}


bool cui::parse_help()
{
    prinbee::pbql::node::pointer_t n(f_lexer->get_next_token());
    if(n->get_token() == prinbee::pbql::token_t::TOKEN_SEMI_COLON)
    {
        f_console_connection->help("basic");
        return true;
    }

    if(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER)
    {
        std::string keyword(n->get_string_upper());
        switch(keyword[0])
        {
        case 'C':
            if(keyword == "COMMANDS")
            {
                f_parser->expect_semi_colon("HELP COMMANDS");
                f_console_connection->help("commands");
                return true;
            }
            if(keyword == "COMMAND")
            {
                n = f_lexer->get_next_token();
                if(n->get_token() != prinbee::pbql::token_t::TOKEN_IDENTIFIER)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << n->get_location().get_location()
                        << "expected a command name after HELP COMMAND not token '"
                        << to_string(n->get_token())
                        << "'.";
                    throw prinbee::invalid_token(msg.str());
                }

                // the command may include multiple words such as
                //   ALTER TABLE
                // and we want them in lowercase and separated with dashes (-)
                //
                std::string command(n->get_string_lower());
                for(n = f_lexer->get_next_token();
                    n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER;
                    n = f_lexer->get_next_token())
                {
                    command += '-';
                    command += n->get_string_lower();
                }

                // handle some special cases renaming the command
                // (or we could use soft links in the final installation?
                // although this probably uses much less disk space except
                // that very small soft links only use inode space, which is
                // still 256 bytes per soft link)
                //
                switch(command[0])
                {
                case 'b':
                    if(command == "begin"
                    || command == "begin-work"
                    || command == "begin-transaction")
                    {
                        command = "transaction";
                        break;
                    }
                    if(command == "bye")
                    {
                        command = "quit";
                        break;
                    }
                    break;

                case 'c':
                    if(command == "commit"
                    || command == "commit-work"
                    || command == "commit-transaction")
                    {
                        command = "transaction";
                    }
                    break;

                case 'e':
                    if(command == "exit")
                    {
                        command = "quit";
                        break;
                    }
                    break;

                case 'r':
                    if(command == "rollback"
                    || command == "rollback-work"
                    || command == "rollback-transaction")
                    {
                        command = "transaction";
                    }
                    break;

                }
                f_parser->expect_semi_colon("HELP COMMAND", n);
                // TODO: maybe add a test to verify that the token represents
                //       a command
                f_console_connection->help(command);
                return true;
            }
            break;

        }
    }

    return false;
}



} // namespace prinbee_cui
// vim: ts=4 sw=4 et
