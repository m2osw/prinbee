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


// cppthread
//
#include    <cppthread/guard.h>


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



cppthread::mutex                g_mutex = cppthread::mutex();
std::string                     g_context_user = std::string();
std::string                     g_context_group = std::string();
context_manager::pointer_t      g_context_manager = context_manager::pointer_t();



} // no name namespace



context_manager::context_manager()
{
}


context_manager::pointer_t context_manager::get_instance()
{
    cppthread::guard lock(*cppthread::g_system_mutex);

    if(g_context_manager == nullptr)
    {
        g_context_manager.reset(new context_manager);
        g_context_manager->load_contexts();
    }

    return g_context_manager;
}


/** \brief Go through the directories to find context.pb files.
 *
 * This function searches for all the contexts defined on this computer.
 * It uses the context root path and searches for files named "context.pb".
 *
 * \note
 * There is no need to have a guard in this function since it is already
 * guarded in get_instance().
 *
 * \sa get_instance()
 */
void context_manager::load_contexts()
{
    std::string const root_path(get_contexts_root_path());

    snapdev::glob_to_list<std::list<std::string>> list;
    if(!list.read_path<
              snapdev::glob_to_list_flag_t::GLOB_FLAG_RECURSIVE
            , snapdev::glob_to_list_flag_t::GLOB_FLAG_EMPTY>(snapdev::pathinfo::canonicalize(root_path, get_context_filename())))
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
        for(auto const & name : list)
        {
            create_context(name);
        }
    }
}


void context_manager::set_user(std::string const & user)
{
    cppthread::guard lock(g_mutex);
    g_context_user = user;
}


std::string context_manager::get_user()
{
    cppthread::guard lock(g_mutex);
    return g_context_user;
}


void context_manager::set_group(std::string const & group)
{
    cppthread::guard lock(g_mutex);
    g_context_group = group;
}


std::string context_manager::get_group()
{
    cppthread::guard lock(g_mutex);
    return g_context_group;
}


advgetopt::string_list_t context_manager::get_context_list() const
{
    advgetopt::string_list_t result;

    for(auto c : f_contexts)
    {
        result.push_back(c.first);
    }

    return result;
}


prinbee::context::pointer_t context_manager::create_context(
      std::string const & name
    , std::string const & description
    , bool create)
{
    context_setup setup(name);
    std::string ownership(get_user());
    if(!ownership.empty())
    {
        setup.set_user(ownership);
    }
    ownership = get_group();
    if(!ownership.empty())
    {
        setup.set_group(ownership);
    }

    // now add it to the list making sure it is unique first
    //
    cppthread::guard lock(g_mutex);

    auto it(f_contexts.find(setup.get_name()));
    if(it != f_contexts.end())
    {
        return it->second;
    }

    // load/create
    //
    context::pointer_t c(context::create_context(setup));
    c->initialize();

    // if "create" is true, also write the (new) context to disk
    //
    if(create)
    {
        context_update update;
        update.set_schema_version(1);
        update.set_description(description);
        c->update(update);
    }

    f_contexts[c->get_name()] = c;

    return c;
}


prinbee::context::pointer_t context_manager::get_context(std::string const & name) const
{
    auto it(f_contexts.find(name));
    if(it == f_contexts.end())
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




} // namespace prinbee
// vim: ts=4 sw=4 et
