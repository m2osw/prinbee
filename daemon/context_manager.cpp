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
 * \brief The context manager to load Prinbee's contexts.
 *
 * The context manager creates, loads, updates, and drops contexts in a
 * Prinbee's environment.
 */


// self
//
#include    "context_manager.h"

//#include    "connection_reference.h"
//#include    "node_client.h"
//#include    "node_listener.h"
//#include    "proxy_listener.h"
//#include    "direct_listener.h"



// prinbee
//
//#include    <prinbee/exception.h>
//#include    <prinbee/names.h>
//#include    <prinbee/version.h>


// communicatord
//
//#include    <communicatord/flags.h>
//#include    <communicatord/names.h>


// cluck
//
//#include    <cluck/cluck_status.h>


// cppprocess
//
//#include    <cppprocess/io_capture_pipe.h>
//#include    <cppprocess/process.h>


//// eventdispatcher
////
//#include    <eventdispatcher/names.h>


// libaddr
//
//#include    <libaddr/addr_parser.h>


// snapdev
//
//#include    <snapdev/gethostname.h>
//#include    <snapdev/hexadecimal_string.h>
//#include    <snapdev/stringize.h>
//#include    <snapdev/tokenize_string.h>
//#include    <snapdev/to_string_literal.h>


// snaplogger
//
//#include    <snaplogger/logger.h>
//#include    <snaplogger/options.h>
//#include    <snaplogger/severity.h>


// advgetopt
//
//#include    <advgetopt/advgetopt.h>
//#include    <advgetopt/exception.h>


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



class context_manager
{
public:
    typedef std::shared_ptr<context_manager>
                                pointer_t;

    context_manager &           operator = (context_manager const & rhs) = delete;

    static pointer_t            get_instance();

    prinbee::context::pointer_t get_context(std::string const & name) const;

private:
                                context_manager();

    void                        load_contexts();

    prinbee::context::map_t     f_context = prinbee::context::map_t();
};


context_manager::pointer_t      g_context_manager = context_manager::pointer_t();


context_manager::pointer_t context_manager::get_instance()
{
    if(g_context_manager == nullptr)
    {
        g_context_manager.reset(new context_manager);
        g_context_manager->load_contexts();
    }

    return g_context_manager;
}


prinbee::context::pointer_t context_manager::get_context(std::string const & name) const
{
    auto it(f_context.find(name));
    if(it == f_context.end())
    {
        return prinbee::context::pointer_t();
    }

    return it->second;
}


void context_manager::load_contexts()
{
    std::string const root_path(prinbee::get_contexts_root_path());
}



}
// no name namespace







prinbee::context::pointer_t get_context(std::string const & name)
{
    context_manager::pointer_t mgr(context_manager::get_instance());
    return mgr->get_context(name);
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
