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
 * \brief Table file header.
 *
 * The table class handles the settings of the table and the files where the
 * data is saved.
 *
 * The _main file_ is used to save the schema. That's where we read it from.
 * This file also includes all the table settings, information about indexes,
 * etc.
 *
 * The table knows how to find all the files, create dbfile objects and
 * request those files to load blocks and thus the settings and data saved
 * in those files.
 *
 * A table is owned by a context.
 */

// self
//
#include    "prinbee/data/schema.h"
#include    "prinbee/database/cursor.h"



namespace prinbee
{



namespace detail
{
class table_impl;
} // detail namespace



constexpr size_t const                          BLOCK_HEADER_SIZE = 4 + 4;  // magic + version (32 bits each)

class context;
class dbfile;
typedef std::shared_ptr<dbfile>                 dbfile_pointer_t;
class block;
typedef std::shared_ptr<block>                  block_pointer_t;



class table
    : public std::enable_shared_from_this<table>
{
public:
    typedef std::shared_ptr<table>              pointer_t;
    typedef std::weak_ptr<table>                weak_pointer_t;
    typedef std::map<std::string, pointer_t>    map_t;

                                                table(
                                                      context * c
                                                    , std::string const & table_dir
                                                    , schema_complex_type::map_pointer_t complex_types);

    pointer_t                                   get_pointer() const;
    pointer_t                                   get_pointer();
    dbfile_pointer_t                            get_dbfile() const;

    // schema management
    //
    //void                                        load_extension(advgetopt::conf_file::pointer_t s);
    schema_version_t                            get_schema_version() const;
    std::string                                 get_name() const;
    model_t                                     get_model() const;
    column_ids_t                                get_primary_key() const;
    schema_column::pointer_t                    get_column(std::string const & name, schema_version_t version = schema_version_t()) const;
    schema_column::pointer_t                    get_column(column_id_t id, schema_version_t version = schema_version_t()) const;
    schema_column::map_by_id_t                  get_columns_by_id(schema_version_t version = schema_version_t()) const;
    schema_column::map_by_name_t                get_columns_by_name(schema_version_t version = schema_version_t()) const;
    bool                                        is_secure() const;
    std::string                                 get_description() const;
    size_t                                      get_size() const; // total size of the file right now
    size_t                                      get_page_size() const; // size of one block in bytes including the magic
    schema_table::pointer_t                     get_schema(schema_version_t version = schema_version_t());

    // block management
    //
    block_pointer_t                             get_block(reference_t offset);
    block_pointer_t                             allocate_new_block(dbtype_t type);
    void                                        free_block(block_pointer_t block, bool clear_block = true);

    // row management
    //
    row_pointer_t                               row_new() const;
    cursor::pointer_t                           row_select(conditions const & cond);
    bool                                        row_commit(row_pointer_t row);
    bool                                        row_insert(row_pointer_t row);
    bool                                        row_update(row_pointer_t row);

private:
    friend cursor;

    void                                        read_rows(cursor::pointer_t cursor);

    std::shared_ptr<detail::table_impl>         f_impl;
};



} // namespace prinbee
// vim: ts=4 sw=4 et
