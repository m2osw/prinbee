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
#pragma once


/** \file
 * \brief Context file header.
 *
 * The context class manages a set of tables. This represents one _database_
 * in the SQL world. The context is pretty shallow otherwise. Most of our
 * settings are in the tables (i.e. replication, compression, compaction,
 * filters, indexes, etc. all of these things are part of the tables).
 */

// self
//
#include    "prinbee/database/table.h"



namespace prinbee
{



namespace detail
{
class context_impl;
}


constexpr std::size_t const                 MAX_CONTEXT_NAME_SEGMENTS = 3;
constexpr std::size_t const                 MAX_CONTEXT_NAME_SEGMENT_LENGTH = 100;

char const *                                get_contexts_subpath();
std::string                                 get_contexts_root_path();


class context_setup
{
public:
                                            context_setup();
                                            context_setup(std::string const & name);

    bool                                    is_valid() const;
    void                                    set_name(std::string const & name);
    std::string const &                     get_name() const;
    void                                    set_user(std::string const & user);
    std::string const &                     get_user() const;
    void                                    set_group(std::string const & group);
    std::string const &                     get_group() const;

private:
    std::string                             f_name = std::string();
    std::string                             f_user = get_prinbee_user();
    std::string                             f_group = get_prinbee_group();
};


class context
{
public:
    typedef std::shared_ptr<context>            pointer_t;
    typedef std::weak_ptr<context>              weak_pointer_t;
    typedef std::map<std::string, pointer_t>    map_t;

                                            ~context();

    static pointer_t                        create_context(context_setup const & setup);

    void                                    load_file(std::string const & filename, bool required);
    void                                    from_binary(virtual_buffer::pointer_t b);

    void                                    initialize();

    std::string                             get_name() const;
    table::pointer_t                        get_table(std::string const & name) const;
    table::map_t const &                    list_tables() const;
    std::string const &                     get_path() const;
    void                                    limit_allocated_memory();
    //std::size_t                             get_config_size(std::string const & name) const;
    //std::string                             get_config_string(std::string const & name, int idx) const;
    //long                                    get_config_long(std::string const & name, int idx) const;

private:
                                            context(context_setup const & setup);

    std::unique_ptr<detail::context_impl>   f_impl;
};



} // namespace prinbee
// vim: ts=4 sw=4 et
