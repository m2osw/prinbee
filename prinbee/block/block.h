// Copyright (c) 2019-2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Base block implementation.
 *
 * The block base class handles the loading of the block in memory using
 * mmap() and gives information such as its type and location.
 */

// self
//
#include    "prinbee/data/dbfile.h"


// C++
//
#include    <map>


namespace prinbee
{


class table;
typedef std::shared_ptr<table>      table_pointer_t;

class version_t;

struct struct_description_t;

class structure;
typedef std::shared_ptr<structure>  structure_pointer_t;



class block
    : public std::enable_shared_from_this<block>
{
public:
    typedef std::shared_ptr<block>              pointer_t;
    typedef std::map<reference_t, pointer_t>    map_t;

                                block(block const & rhs) = delete;
                                ~block();

    block &                     operator = (block const & rhs) = delete;

    table_pointer_t             get_table() const;
    void                        set_table(table_pointer_t table);
    structure_pointer_t         get_structure() const;
    void                        clear_block();

    dbtype_t                    get_dbtype() const;
    void                        set_dbtype(dbtype_t type);
    version_t                   get_structure_version() const;
    void                        set_structure_version();
    reference_t                 get_offset() const;
    void                        set_data(data_t data);
    data_t                      data(reference_t offset = 0);
    const_data_t                data(reference_t offset = 0) const;
    void                        sync(bool immediate);

    void                        from_current_file_version();

protected:
                                block(struct_description_t const * structure_description, dbfile::pointer_t f, reference_t offset);

    table_pointer_t             f_table = table_pointer_t(); // TODO: we probably need a weak pointer here
    dbfile::pointer_t           f_file = dbfile::pointer_t();
    structure_pointer_t         f_structure = structure_pointer_t();        // newest version (i.e. max(version) from all available descriptions)
    //version_t                   f_structure_version = version_t(); -- at the moment, this creates a loop
    reference_t                 f_offset = reference_t();

    mutable data_t              f_data = nullptr;
};



} // namespace prinbee
// vim: ts=4 sw=4 et
