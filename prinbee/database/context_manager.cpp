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
#include    <snapdev/glob_to_list.h>
//#include    <snapdev/hexadecimal_string.h>
//#include    <snapdev/stringize.h>
//#include    <snapdev/tokenize_string.h>
//#include    <snapdev/to_string_literal.h>


// snaplogger
//
//#include    <snaplogger/logger.h>
#include    <snaplogger/message.h>
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






namespace prinbee
{


namespace
{



constexpr char const * const    g_contexts_subpath = "contexts";
std::string                     g_context_user = std::string();
std::string                     g_context_group = std::string();



} // no name namespace



context_manager::pointer_t      g_context_manager = context_manager::pointer_t();


context_manager::context_manager()
{
}


context_manager::pointer_t context_manager::get_instance()
{
    if(g_context_manager == nullptr)
    {
        g_context_manager.reset(new context_manager);
        g_context_manager->load_contexts();
    }

    return g_context_manager;
}


void context_manager::load_contexts()
{
    std::string const root_path(get_contexts_root_path());

    snapdev::glob_to_list<std::list<std::string>> list;
    if(!list.read_path<
              snapdev::glob_to_list_flag_t::GLOB_FLAG_ONLY_DIRECTORIES
            , snapdev::glob_to_list_flag_t::GLOB_FLAG_EMPTY>(snapdev::pathinfo::canonicalize(root_path, "*")))
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << "could not read directory \""
            << root_path
            << "\" for a list of contexts.";
        throw io_error(msg.str());
    }

    if(list.empty())
    {
        SNAP_LOG_DEBUG
            << "no context found under \""
            << root_path
            << "\"."
            << SNAP_LOG_SEND;
    }
    else
    {
        for(auto const & context_dir : list)
        {
            context_setup setup(context_dir);
            if(!g_context_user.empty())
            {
                setup.set_user(g_context_user);
            }
            if(!g_context_group.empty())
            {
                setup.set_group(g_context_group);
            }
            context::pointer_t c(context::create_context(setup));
            c->initialize();
            f_context[c->get_name()] = c;
        }
    }
}


void context_manager::set_user(std::string const & user)
{
    g_context_user = user;
}


void context_manager::set_group(std::string const & group)
{
    g_context_group = group;
}


advgetopt::string_list_t context_manager::get_context_list() const
{
    advgetopt::string_list_t result;

    for(auto c : f_context)
    {
        result.push_back(c.first);
    }

    return result;
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








// TBD: do we really need this one?!
//context::pointer_t get_context(std::string const & name)
//{
//    context_manager::pointer_t mgr(context_manager::get_instance());
//    return mgr->get_context(name);
//}



/** \brief The sub-path added to the root path to access contexts.
 *
 * The sub-path is used to save the set of contexts within a sub-folder so
 * we can better organized the data.
 *
 * \note
 * This parameter cannot be changed using a setting. It is on purpose hard
 * coded in this file.
 *
 * \return The sub-path to the contexts.
 */
char const * get_contexts_subpath()
{
    return g_contexts_subpath;
}


/** \brief Get the path to the root of the contexts.
 *
 * This function returns the path to the root path.
 *
 * It is possible to change this folder using the `prinbee_path` option
 * of the prinbee daemon. This is particularly useful to run unit and
 * integration tests.
 *
 * \return The root path to the set of contexts.
 */
std::string get_contexts_root_path()
{
    return snapdev::pathinfo::canonicalize(get_prinbee_path(), get_contexts_subpath());
}



} // namespace prinbee
// vim: ts=4 sw=4 et
