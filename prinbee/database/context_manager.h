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
#pragma once

// self
//
//#include    "messenger.h"
//#include    "interrupt.h"
//#include    "connection_reference.h"


// prinbee
//
//#include    <prinbee/names.h>
//#include    <prinbee/network/binary_server.h>
#include    <prinbee/database/context.h>


// cppthread
//
#include    <cppthread/mutex.h>



namespace prinbee
{



class context_manager
{
public:
    typedef std::shared_ptr<context_manager>
                                pointer_t;

                                context_manager(context_manager const & rhs) = delete;
    context_manager &           operator = (context_manager const & rhs) = delete;

    static pointer_t            get_instance();
    static void                 set_user(std::string const & user);
    static std::string          get_user();
    static void                 set_group(std::string const & group);
    static std::string          get_group();

    advgetopt::string_list_t    get_context_list() const;
    context::pointer_t          create_context(
                                      std::string const & name
                                    , std::string const & description = std::string()
                                    , bool create = false);
    context::pointer_t          get_context(std::string const & name) const;

private:
                                context_manager();

    void                        load_contexts();

    prinbee::context::map_t     f_contexts = prinbee::context::map_t();
};



} // namespace prinbee
// vim: ts=4 sw=4 et
