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
#include    "console_connection.h"

#include    "cui.h"


// communicatord
//
//#include    <communicatord/communicator.h>
//#include    <communicatord/names.h>
//#include    <communicatord/version.h>


// eventdispatcher
//
#include    <eventdispatcher/cui_connection.h>
#include    <eventdispatcher/local_stream_client_message_connection.h>
#include    <eventdispatcher/local_dgram_server_message_connection.h>
#include    <eventdispatcher/tcp_client_message_connection.h>
#include    <eventdispatcher/udp_server_message_connection.h>


// edhttp
//
#include    <edhttp/uri.h>


// snaplogger
//
#include    <snaplogger/message.h>
#include    <snaplogger/options.h>


// libaddr
//
#include    <libaddr/addr_parser.h>
#include    <libaddr/iface.h>


// snapdev
//
#include    <snapdev/file_contents.h>
#include    <snapdev/gethostname.h>
#include    <snapdev/not_reached.h>
#include    <snapdev/not_used.h>
#include    <snapdev/pathinfo.h>
#include    <snapdev/stringize.h>


// advgetopt
//
#include    <advgetopt/advgetopt.h>
#include    <advgetopt/conf_file.h>
#include    <advgetopt/exception.h>


// C++
//
#include    <atomic>


// readline
//
#include    <readline/readline.h>
#include    <readline/history.h>


// last include
//
#include    <snapdev/poison.h>



/** \file
 * \brief A tool to send and receive messages to services to test them.
 *
 * This tool can be used to test various services and make sure they
 * work as expected, at least for their control feed. If they have
 * network connections that have nothing to do with communicator
 * messaging feeds, then it won't work well.
 *
 * The organization of this file is as follow:
 *
 * +------------------------+
 * |                        |
 * |        Base            |
 * |      (Connection)      |
 * |                        |
 * +------------------------+
 *        ^              ^
 *        |              |
 *        |              +----------------------------+
 *        |              |                            |
 *        |   +----------+-------------+   +----------+-------------+
 *        |   |                        |   |                        |
 *        |   |      GUI Object        |   |     CUI Object         |
 *        |   |                        |   |                        |
 *        |   +------------------------+   +------------------------+
 *        |        ^                                   ^
 *        |        |                                   |
 *        |        |       +---------------------------+
 *        |        |       |
 *     +--+--------+-------+----+
 *     |                        |
 *     |  Message Obj.          |
 *     |                        |
 *     +------------------------+
 */


namespace prinbee_cui
{



constexpr char const *  g_history_file = "~/.message_history";
console_connection *    g_console = nullptr;


int show_status(int count, int c)
{
    snapdev::NOT_USED(count, c);

    if(g_console != nullptr)
    {
        g_console->open_close_status_window();
    }

    return 0;
}


console_connection::console_connection(cui * c)
    : cui_connection(g_history_file)
    , f_cui(c)
{
    if(g_console != nullptr)
    {
        throw std::logic_error("there can be only one console_connection");
    }
    g_console = this;
    prompt_to_output_command("> ");
    set_name("prinbee console");
}


void console_connection::set_documentation_path(std::string const & path)
{
    f_documentation_path = path;
}


void console_connection::reset_prompt()
{
    set_prompt(f_cui->define_prompt());
}


void console_connection::process_command(std::string const & command)
{
    // Note: "command" may include multiple commands separated by
    //       semi-colons hence the plural in the f_cui function name
    //
    f_cui->execute_commands(command);

    // reset the prompt in case something changed (i.e. connected to a context)
    //
    reset_prompt();
}


void console_connection::process_quit()
{
    f_cui->stop(false);

    // remove the pipes for stdout and stderr
    //
    // WARNING: this must be done AFTER we disconnected from the
    //          ncurses which is done above (at this point the
    //          connection was deleted though! weird...)
    //
    cui_connection::process_quit(); 
}


void console_connection::process_help()
{
    output("> HELP;");
    help("basic");
}


// example from the communicatord tools/message.cpp file
//bool console_connection::execute_command(std::string const & command)
//{
//    // QUIT;
//    //
//    // request to quit the process, equivalent to Ctrl-D
//    //
//// that should be a statistic in a stats window...
////output("# of con: " + std::to_string(ed::communicator::instance()->get_connections().size()));
//    if(command == "/quit")
//    {
//        // the "/quit" internal command
//        //
//        process_quit();
//        return false;
//    }
//
//    // /help
//    //
//    // print out help screen
//    //
//    if(command == "/help"
//    || command == "/?"
//    || command == "?")
//    {
//        help();
//        return false;
//    }
//
//    if(command == "/pbql_help")
//    {
//        help_pbql();
//        return false;
//    }
//
//    network_connection::pointer_t c(f_connection.lock());
//    if(c == nullptr)
//    {
//        output("You are disconnected. Most commands will not work anymore.");
//        return false;
//    }
//
//    // /connect <scheme>://<IP>:<port> | <scheme>:///<path>
//    //
//    // connect to service listening at <IP> on port <port>
//    //
//    if(command.compare(0, 9, "/connect ") == 0)
//    {
//        if(c->set_address(command.substr(9)))
//        {
//            c->connect();
//        }
//        return true;
//    }
//
//    // /disconnect
//    //
//    // remove the existing connection
//    //
//    if(command == "/disconnect")
//    {
//        c->disconnect();
//        return true;
//    }
//
//    // "/.*" is not a valid message beginning, we suspect that the user
//    // misstyped a command and thus generate an error instead
//    //
//    if(command[0] == '/')
//    {
//        output("error: unknown command: \"" + command + "\".");
//        return false;
//    }
//
//    if(!c->has_prompt())
//    {
//        output("error: message not sent, we are not connected.");
//        return false;
//    }
//
//    // by default, if not an internal command, we consider `command`
//    // to be a message and therefore we just send it
//    //
//    c->send_message(command);
//    return false;
//}


void console_connection::set_status_window_key_binding()
{
    if(rl_bind_keyseq("\\eOQ" /* F2 */, &show_status) != 0)
    {
        std::cerr << "error: status window key (^[OQ a.k.a. F2) binding failed.\n";
    }
}


void console_connection::open_close_status_window()
{
    if(f_win_status != nullptr)
    {
output("> hide status;");
        del_panel(f_pan_status);
        f_pan_status = nullptr;
        delwin(f_win_status);
        f_win_status = nullptr;
        //refresh();
        update_panels();
        return;
    }
output("> show status;");

    // TODO: gather the screen size and make sure it fits
    //
    int width = 80;
    int height = 12;
    f_win_status = newwin(height - 4, width - 4, 3, 12);
    if(f_win_status == nullptr)
    {
        output("error: couldn't create status window.");
        return;
    }
    f_pan_status = new_panel(f_win_status);
    if(f_pan_status == nullptr)
    {
        output("error: could not create status panel");
        delwin(f_win_status);
        f_win_status = nullptr;
        return;
    }

    wborder(f_win_status, 0, 0, 0, 0, 0, 0, 0, 0);
    mvwprintw(f_win_status, 0, 2, " Status ");

    update_status();
}


void console_connection::update_status()
{
    if(f_win_status != nullptr)
    {
        {
            std::string msg(" Communicator: ");
            msg += f_cui->get_messenger_status();
            mvwprintw(f_win_status, 1, 2, "%s", msg.c_str());
        }

        {
            std::string msg("Fluid Service: ");
            msg += f_cui->get_fluid_settings_status();
            mvwprintw(f_win_status, 2, 2, "%s", msg.c_str());
        }

        {
            std::string msg("        Proxy: ");
            msg += f_cui->get_proxy_status();
            mvwprintw(f_win_status, 3, 2, "%s", msg.c_str());
        }

        {
            std::string msg("    Last Ping: ");
            snapdev::timespec_ex const last_ping(f_cui->get_last_ping());
            if(last_ping == snapdev::timespec_ex())
            {
                msg += "never";
            }
            else
            {
                msg += last_ping.to_string("%Y/%m/%d %T", true);
            }
            mvwprintw(f_win_status, 4, 2, "%s", msg.c_str());
        }

        {
            std::string msg("      Pinbree: ");
            msg += f_cui->get_prinbee_status();
            mvwprintw(f_win_status, 5, 2, "%s", msg.c_str());
        }

        {
            std::string msg("      Console: ");
            msg += f_cui->get_console_status();
            mvwprintw(f_win_status, 6, 2, "%s", msg.c_str());
        }

        //if(wrefresh(f_win_status) != OK)
        //{
        //    output("error: wrefresh() to status message window failed.");
        //    return;
        //}
        update_panels();
    }
}


void console_connection::ready()
{
    output("Ready.\nType HELP; or F1 for basic help screen.");
}


void console_connection::help(std::string const & section_name)
{
    // the .hlp extensions are just text files, nothing fancy
    // although it could include console commands to change color, etc.
    // (not too sure how that would work within ncurses at the moment...
    // although I have an output() function with color support.)
    //
    std::string const filename(snapdev::pathinfo::canonicalize(f_documentation_path, section_name) + ".hlp");

    snapdev::file_contents in(filename);
    if(!in.read_all())
    {
        std::cerr
            << "error: could not read the help section \""
            << section_name
            << "\" from file \""
            << filename
            << "\" -- "
            << in.last_error()
            << ".\n";
        return;
    }

    // the function expects the file contents to be "correct"
    //
    output(in.contents());

    //output("Help:");
    //output("Internal commands start with a  slash (/). Supported commands:");
    //output("  /connect <scheme>://<ip>:<port> | <scheme>:///<path> -- connect to specified URI");
    //output("    i.e. /connect cd://192.168.2.1:4004");
    //output("  /disconnect -- explicitly disconnect any existing connection");
    //output("  /help or /? or ? or <F1> key -- print this help screen");
    //output("  /quit -- leave tool");
    //output("  <F2> key -- create a message in a popup window");
    //output("  ... -- message to send to current connection (/msg_help for more)");
    //output("    a message is composed of:");
    //output("      ['<'<server>:<service>' '][<server>:<service>'/']command[' '<name>=<value>';'...]");
    //output("    where the first <server>:<service> is the origin (\"sent from\")");
    //output("    where the second <server>:<service> is the destination");
    //output("    where <name>=<value> pairs are parameters (can be repeated)");
}


//void console_connection::help_pbql()
//{
//    // TODO: actually, we want that in separate .hlp files
//    //
//    output("Help:");
//    output("PBQL commands:");
//    output("  CREATE CONTEXT <name> ...;");
//    output("  SHOW CONTEXT;");
//}



} // namespace prinbee_cui
// vim: ts=4 sw=4 et
