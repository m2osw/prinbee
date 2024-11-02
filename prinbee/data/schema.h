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
 * \brief Schema header.
 *
 * The schema class manages one _database_ representation. A _database_
 * is composed of tables, user defined types, columns, cells, and indexes.
 *
 * Many settings are defined at the database, table, and index levels.
 *
 * For example, the table has a replication, compression, and compaction
 * settings.
 *
 * An index can include a filter (a WHERE clause in pbql), whether a
 * column is ascending or descending, an expression to use as a column,
 * etc.
 */

// self
//
#include    "prinbee/data/structure.h"


// basic-xml
//
#include    <basic-xml/xml.h>


// advgetopt
//
#include    <advgetopt/advgetopt.h>



namespace prinbee
{



typedef std::uint32_t                   schema_version_t;
typedef std::uint16_t                   language_id_t;
typedef std::uint32_t                   flag32_t;       // look into not using these, instead use the structure directly
typedef std::uint64_t                   flag64_t;
typedef std::uint16_t                   column_id_t;
typedef std::vector<column_id_t>        column_ids_t;
typedef std::uint32_t                   index_id_t;



char const *                            get_expiration_date_column_name();



enum class model_t
{
    TABLE_MODEL_CONTENT,
    TABLE_MODEL_DATA,
    TABLE_MODEL_LOG,
    TABLE_MODEL_QUEUE,
    TABLE_MODEL_SEQUENCIAL,
    TABLE_MODEL_SESSION,
    TABLE_MODEL_TREE,

    TABLE_MODEL_DEFAULT = TABLE_MODEL_CONTENT,
};

model_t             name_to_model(std::string const & name);


enum compare_t
{
    COMPARE_SCHEMA_EQUAL,
    COMPARE_SCHEMA_UPDATE,
    COMPARE_SCHEMA_DIFFER,
};



constexpr flag64_t                          TABLE_FLAG_SECURE       = (1ULL << 0);
constexpr flag64_t                          TABLE_FLAG_UNLOGGED     = (1ULL << 2);
//constexpr flag64_t                          TABLE_FLAG_TEMPORARY     = (1ULL << 3);
constexpr flag64_t                          TABLE_FLAG_TRANSLATABLE = (1ULL << 4);


// Special values
constexpr column_id_t                       COLUMN_NULL = 0;


// SAVED IN FILE, DO NOT CHANGE BIT LOCATIONS
constexpr flag32_t                          COLUMN_FLAG_LIMITED                 = (1ULL << 0);
constexpr flag32_t                          COLUMN_FLAG_REQUIRED                = (1ULL << 1);
constexpr flag32_t                          COLUMN_FLAG_BLOB                    = (1ULL << 2);
constexpr flag32_t                          COLUMN_FLAG_SYSTEM                  = (1ULL << 3);
constexpr flag32_t                          COLUMN_FLAG_REVISION_TYPE           = (3ULL << 4);   // TWO BITS (see COLUMN_REVISION_TYPE_...)
constexpr flag32_t                          COLUMN_FLAG_HIDDEN                  = (1ULL << 6);
constexpr flag32_t                          COLUMN_FLAG_VERSIONED               = (1ULL << 7);

// Revision Types (after the shift, TBD: should we keep the shift?)
constexpr flag32_t                          COLUMN_REVISION_TYPE_GLOBAL         = 0;
constexpr flag32_t                          COLUMN_REVISION_TYPE_BRANCH         = 1;
constexpr flag32_t                          COLUMN_REVISION_TYPE_REVISION       = 2;
//constexpr flag32_t                          COLUMN_REVISION_TYPE_unused         = 3; -- currently unused


// SAVED IN FILE, DO NOT CHANGE BIT LOCATIONS
constexpr flag32_t                          SCHEMA_SORT_COLUMN_DESCENDING       = (1LL << 0);
constexpr flag32_t                          SCHEMA_SORT_COLUMN_WITHOUT_NULLS    = (1LL << 1);
constexpr flag32_t                          SCHEMA_SORT_COLUMN_PLACE_NULLS_LAST = (1LL << 1);

constexpr std::uint32_t                     SCHEMA_SORT_COLUMN_DEFAULT_SIZE     = 256UL;


// SAVED IN FILE, DO NOT CHANGE BIT LOCATIONS
constexpr flag32_t                          SECONDARY_INDEX_FLAG_DISTRIBUTED        = (1LL << 0);
constexpr flag32_t                          SECONDARY_INDEX_FLAG_WITHOUT_NULLS      = (1LL << 0);
constexpr flag32_t                          SECONDARY_INDEX_FLAG_NULLS_NOT_DISTINCT = (1LL << 0);


constexpr std::uint8_t                      TABLE_DEFAULT_REPLICATION           = 1;


enum index_type_t
{
    INDEX_TYPE_INVALID = -1,

    INDEX_TYPE_SECONDARY,                   // user defined secondary index
    INDEX_TYPE_INDIRECT,                    // indirect index, based on OID
    INDEX_TYPE_PRIMARY,                     // primary index, using primary key
    INDEX_TYPE_EXPIRATION,                  // expiration index (TBD)
    INDEX_TYPE_TREE,                        // tree index, based on a path
};

index_type_t                                index_name_to_index_type(std::string const & name);
std::string                                 index_type_to_index_name(index_type_t type);


constexpr std::size_t                       MAX_COMPLEX_TYPE_REFERENCE_DEPTH = 10;


class schema_complex_type
{
public:
    typedef std::shared_ptr<schema_complex_type>
                                            pointer_t;
    typedef std::map<std::string, pointer_t>
                                            map_t;
    typedef std::shared_ptr<map_t>          map_pointer_t;

                                            //schema_complex_type();
                                            schema_complex_type(advgetopt::conf_file::pointer_t config, std::string const & name);

    std::string                             get_name() const;
    bool                                    is_enum() const;
    std::size_t                             get_size() const;
    void                                    set_type_name(int idx, std::string const & type_name);
    std::string                             get_type_name(int idx) const;
    void                                    set_type(int idx, struct_type_t type);
    struct_type_t                           get_type(int idx) const;
    std::int64_t                            get_enum_value(int idx) const;

private:
    struct field_t
    {
        typedef std::vector<field_t>        vector_t;

        std::string             f_name = std::string();
        std::string             f_type_name = std::string();    // used on load and for STRUCT_TYPE_STRUCTURE
        struct_type_t           f_type = struct_type_t::STRUCT_TYPE_VOID;
        std::int64_t            f_enum_value = 0;
    };

    std::string                             f_name = std::string();
    std::string                             f_description = std::string();
    std::string                             f_compare = std::string();
    std::string                             f_validation_script = std::string();
    bool                                    f_is_enum = false;
    struct_type_t                           f_enum_type = struct_type_t::STRUCT_TYPE_INT16;
    field_t::vector_t                       f_fields = field_t::vector_t();
    schema_complex_type::map_pointer_t      f_complex_types = schema_complex_type::map_pointer_t();
};


schema_complex_type::map_pointer_t          load_complex_types(std::string const & filename);




class schema_table;
typedef std::shared_ptr<schema_table>       schema_table_pointer_t;
typedef std::weak_ptr<schema_table>         schema_table_weak_pointer_t;

class schema_column
{
public:
    typedef std::shared_ptr<schema_column>      pointer_t;
    typedef std::map<column_id_t, pointer_t>    map_by_id_t;
    typedef std::map<std::string, pointer_t>    map_by_name_t;

                                            schema_column(
                                                      schema_table_pointer_t table
                                                    , advgetopt::conf_file::pointer_t config
                                                    , std::string const & column_id);
                                            schema_column(
                                                      schema_table_pointer_t table
                                                    , structure::pointer_t s);
                                            schema_column(
                                                      schema_table_pointer_t table
                                                    , std::string const & name
                                                    , struct_type_t type
                                                    , flag32_t flags);

    void                                    from_structure(structure::pointer_t s);
    bool                                    is_expiration_date_column() const;
    compare_t                               compare(schema_column const & rhs) const;

    schema_table_pointer_t                  get_table() const;

    column_id_t                             get_id() const;
    void                                    set_id(column_id_t id);
    std::string                             get_name() const;
    struct_type_t                           get_type() const;
    flag32_t                                get_flags() const;
    std::string                             get_encrypt_key_name() const;
    std::int32_t                            get_internal_size_limit() const;
    buffer_t                                get_default_value() const;
    buffer_t                                get_minimum_value() const;
    buffer_t                                get_maximum_value() const;
    std::uint32_t                           get_minimum_size() const;
    std::uint32_t                           get_maximum_size() const;
    buffer_t                                get_validation_script() const;

private:
    column_id_t                             f_id = column_id_t();
    std::string                             f_name = std::string();
    struct_type_t                           f_type = struct_type_t();
    flag32_t                                f_flags = flag32_t();
    std::string                             f_encrypt_key_name = std::string();
    std::int32_t                            f_internal_size_limit = -1; // -1 = no limit; if size > f_internal_size_limit, save in external file
    buffer_t                                f_default_value = buffer_t();
    buffer_t                                f_minimum_value = buffer_t();
    buffer_t                                f_maximum_value = buffer_t();
    std::uint32_t                           f_minimum_size = 0;
    std::uint32_t                           f_maximum_size = 0;
    buffer_t                                f_validation_script = buffer_t();

    // not saved on disk
    //
    schema_table_weak_pointer_t             f_schema_table = schema_table_weak_pointer_t();
    std::string                             f_description = std::string();
};




class schema_sort_column
{
public:
    typedef std::shared_ptr<schema_sort_column>
                                            pointer_t;
    typedef std::vector<pointer_t>          vector_t;

    void                                    from_config(std::string const & column_definition);

    compare_t                               compare(schema_sort_column const & rhs) const;

    column_id_t                             get_column_id() const;
    void                                    set_column_id(column_id_t column_id);
    flag32_t                                get_flags() const;
    void                                    set_flags(flag32_t flags);
    bool                                    is_ascending() const;
    bool                                    accept_null_columns() const;
    bool                                    place_nulls_last() const;
    std::uint32_t                           get_size() const;
    void                                    set_size(std::uint32_t size);

private:
    column_id_t                             f_column_id = column_id_t();
    flag32_t                                f_flags = 0;
    std::uint32_t                           f_size = SCHEMA_SORT_COLUMN_DEFAULT_SIZE;
};


class schema_secondary_index
{
public:
    typedef std::shared_ptr<schema_secondary_index>
                                            pointer_t;
    typedef std::map<std::string, pointer_t>
                                            map_by_name_t;
    typedef std::map<index_id_t, pointer_t> map_by_id_t;

    void                                    from_config(
                                                  advgetopt::conf_file::pointer_t config
                                                , std::string const & index_id);

    compare_t                               compare(schema_secondary_index const & rhs) const;

    index_id_t                              get_id() const;
    std::string                             get_name() const;
    void                                    set_name(std::string const & name);

    flag32_t                                get_flags() const;
    void                                    set_flags(flag32_t flags);
    bool                                    get_distributed_index() const;
    void                                    set_distributed_index(bool distributed);

    size_t                                  get_column_count() const;
    //advgetopt::string_list_t const &        get_sort_columns() const;
    schema_sort_column::pointer_t           get_sort_column(int idx) const;
    void                                    add_sort_column(schema_sort_column::pointer_t sc);

    buffer_t                                get_filter() const;
    void                                    set_filter(buffer_t const & filter);

private:
    index_id_t                              f_id = index_id_t();
    std::string                             f_name = std::string();
    std::string                             f_description = std::string();
    schema_sort_column::vector_t            f_sort_columns = schema_sort_column::vector_t();
    buffer_t                                f_key_script = buffer_t();
    buffer_t                                f_filter = buffer_t();
    flag32_t                                f_flags = flag32_t();
};




class schema_table
    : public std::enable_shared_from_this<schema_table>
{
public:
    typedef std::shared_ptr<schema_table>   pointer_t;
    typedef std::map<schema_version_t, pointer_t>
                                            map_by_version_t;

                                            schema_table();

    void                                    set_complex_types(schema_complex_type::map_pointer_t complex_types);

    void                                    from_config(std::string const & name, std::string const & filename);
    //void                                    load_extension(advgetopt::conf_file::pointer_t s);
    compare_t                               compare(schema_table const & rhs) const;

    void                                    from_binary(virtual_buffer::pointer_t b);
    virtual_buffer::pointer_t               to_binary() const;

    schema_version_t                        get_schema_version() const;
    void                                    set_schema_version(schema_version_t version);
    time_t                                  get_added_on() const;
    std::string                             get_name() const;
    model_t                                 get_model() const;
    bool                                    is_secure() const;
    column_ids_t                            get_primary_key() const;
    void                                    assign_column_ids(pointer_t existing_schema = pointer_t());
    bool                                    has_expiration_date_column() const;
    schema_column::pointer_t                get_expiration_date_column() const;
    schema_column::pointer_t                get_column(std::string const & name) const;
    schema_column::pointer_t                get_column(column_id_t id) const;
    schema_column::map_by_id_t              get_columns_by_id() const;
    schema_column::map_by_name_t            get_columns_by_name() const;
    schema_secondary_index::pointer_t       get_secondary_index(std::string const & name) const;
    schema_complex_type::pointer_t          get_complex_type(std::string const & name) const;

    std::string                             get_description() const;

private:
    void                                    from_config_name(std::string const & name);
    void                                    from_config_version(std::string const & filename);
    advgetopt::conf_file::pointer_t         from_config_load_config();
    void                                    from_config_load_table(advgetopt::conf_file::pointer_t config);
    void                                    from_config_load_columns(advgetopt::conf_file::pointer_t config);
    void                                    from_config_load_primary_key(advgetopt::conf_file::pointer_t config);
    void                                    from_config_load_indexes(advgetopt::conf_file::pointer_t config);
    //void                                    process_columns(basic_xml::node::pointer_t column_definitions);
    void                                    process_secondary_indexes(basic_xml::node::deque_t secondary_indexes);

    schema_complex_type::map_pointer_t      f_complex_types = schema_complex_type::map_pointer_t();
    schema_version_t                        f_version = schema_version_t();
    std::string                             f_name = std::string();
    std::string                             f_filename = std::string();
    std::string                             f_description = std::string();
    std::uint8_t                            f_replication = 1;
    std::uint16_t                           f_versioned_rows = 1;
    model_t                                 f_model = model_t::TABLE_MODEL_CONTENT;
    flag64_t                                f_flags = flag64_t();
    std::int64_t                            f_blob_limit = 0;

    time_t                                  f_added_on = time(nullptr);
    //advgetopt::string_list_t                f_primary_key_names = advgetopt::string_list_t();
    column_ids_t                            f_primary_key = column_ids_t();
    schema_secondary_index::map_by_name_t   f_secondary_indexes = schema_secondary_index::map_by_name_t();
    schema_secondary_index::map_by_id_t     f_secondary_indexes_by_id = schema_secondary_index::map_by_id_t();
    schema_column::map_by_name_t            f_columns_by_name = schema_column::map_by_name_t();
    schema_column::map_by_id_t              f_columns_by_id = schema_column::map_by_id_t();

    // only memory parameters
    //
    reference_t                             f_schema_offset = NULL_FILE_ADDR;
};



} // namespace prinbee
// vim: ts=4 sw=4 et
