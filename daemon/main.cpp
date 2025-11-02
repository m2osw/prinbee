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
 * \brief Daemon managing prinbee data in the backend.
 *
 * This daemon is prinbee's database backend. It manages the files with
 * each context schema, the data, indexes, etc. Everything that runs in
 * the prinbee backend.
 *
 * To tweak things in the backend, use one of the front ends. We provide
 * a CLI with the pbql language to make it easy to start and various
 * client classes in the prinbee library.
 */

// self
//
#include    "prinbeed.h"


// eventdispatcher
//
#include    <eventdispatcher/signal_handler.h>


// libexcept
//
#include    <libexcept/file_inheritance.h>


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

    std::string errmsg;
    try
    {
        prinbee_daemon::prinbeed daemon(argc, argv);
        daemon.finish_initialization();
        return daemon.run();
    }
    catch(advgetopt::getopt_exit const & e)
    {
        return e.code();
    }
    catch(std::exception const & e)
    {
        errmsg = "prinbeed:error: ";
        errmsg += e.what();
        errmsg += " (stack information may be available in the logs).";
        SNAP_LOG_FATAL << "uncaught exception: " << e.what() << SNAP_LOG_SEND_WITH_STACK_TRACE(e);
    }
    catch(...)
    {
        errmsg = "prinbeed:error: unknown exception caught!";
        SNAP_LOG_FATAL << errmsg << SNAP_LOG_SEND;
    }

    if(isatty(STDERR_FILENO))
    {
        std::cerr << errmsg << std::endl;
    }

    return 1;
}



// vim: ts=4 sw=4 et
