// Copyright (c) 2019-2025  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Proxy service transferring client's requests to the daemon.
 *
 * This daemon is used to send the data between clients and daemons. This
 * gives the client a chance to save its data in a local journal, have a
 * local memory cache, and properly distribute the messages to the
 * backend daemons (i.e. say you have 21 backends and a replication
 * factor of 3, you need to send the data to 3 backends, which ones?)
 */

// self
//
#include    "cui.h"


// eventdispatcher
//
#include    <eventdispatcher/signal_handler.h>


// libexcept
//
#include    <libexcept/file_inheritance.h>
#include    <libexcept/report_signal.h>


// advgetopt
//
#include    <advgetopt/exception.h>


// snaplogger
//
#include    <snaplogger/message.h>


// last include
//
#include    <snapdev/poison.h>



int main(int argc, char * argv[])
{
    ed::signal_handler::create_instance();
    libexcept::verify_inherited_files();
    libexcept::collect_stack_trace();
    libexcept::init_report_signal();

    std::string errmsg;
    try
    {
        prinbee_cui::cui prompt(argc, argv);
        return prompt.run();
    }
    catch(advgetopt::getopt_exit const & e)
    {
        return e.code();
    }
    catch(std::exception const & e)
    {
        errmsg = "pbql_cui:error: ";
        errmsg += e.what();
        errmsg += " (stack information may be available in the logs).";
        SNAP_LOG_FATAL << "uncaught exception: " << e.what() << SNAP_LOG_SEND_WITH_STACK_TRACE(e);
    }
    catch(...)
    {
        errmsg = "pbql_cui:error: unknown exception caught!";
        SNAP_LOG_FATAL << errmsg << SNAP_LOG_SEND;
    }

    if(isatty(STDERR_FILENO))
    {
        std::cerr << errmsg << std::endl;
    }

    return 1;
}



// vim: ts=4 sw=4 et
