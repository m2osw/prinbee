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
 * \brief The context implementation.
 *
 * A context is a collection of tables. The implementation reads all
 * the table schemata so the system is ready to accept commands to
 * read and write data to and from any of the tables. For this reason
 * we make use of a single file (SCHM) for all the schemata. There is
 * also one other file with complex types (CXTP) also called user
 * defined types.
 *
 * The schemata file also includes some information about the context.
 * This includes when the context was created and, if it happens, when
 * it was dropped. This is important because a node that is down at the
 * time a DROP happens would not know that the context was deleted and
 * as a result it would attempt to replicate it on all the other nodes.
 *
 * When you connect to a prinbee server, you pretty much automatically
 * get a copy of those two files so it is possible to check the validity
 * of the data locally before attempting to send it to the server.
 *
 * By default, it is expected that you only run with one single context
 * per node. Having more than one context on a single node may cause
 * issues that you cannot resolve easily (i.e. various types of conflicts
 * may arise between different contexts). However, there is nothing
 * preventing you from having more than one context.
 *
 * A single project may use multiple contexts because the type of data
 * found in each context is very different so the nodes will act differently
 * in each context and having that data separate is the best way to better
 * manage the data.
 *
 * \note
 * A context is pretty shallow. It manages a set of tables and that's
 * about it. Details on how the data is replicated, compressed,
 * compacted, filtered, indexed, etc. is found in a table. However, the
 * context is the object that keeps a copy of the table schemata including
 * their replication information, indexes, etc.
 */


// self
//
#include    "prinbee/database/context.h"

#include    "prinbee/names.h"


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/glob_to_list.h>
#include    <snapdev/hexadecimal_string.h>
#include    <snapdev/mkdir_p.h>


// C++
//
#include    <deque>


// C
//
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <unistd.h>
#include    <fcntl.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



namespace
{



constexpr char const * const    g_contexts_subpath = "contexts";
constexpr char const * const    g_context_filename = "context.pb";
constexpr char const * const    g_complex_types_filename = "complex-types.pb";
constexpr char const * const    g_tables_subpath = "tables";
constexpr char const * const    g_indexes_subpath = "indexes";
//constexpr char const * const    g_schemata_filename = "schemata.pb";


// context file definition
struct_description_t g_context_file_description[] =
{
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_magic)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::FILE_TYPE_CONTEXT))
    ),
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_structure_version)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(1, 0)
    ),
    // this file description structure is viewed as being part of the schema;
    // this version starts at 1 and goes up, it is used to make sure that two
    // ALTER CONTEXT from two different clients cannot update the context
    // from version N to version N+1; one has to wait for the other to be
    // done then it can apply the change with version N+2
    //
    prinbee::define_description(
          prinbee::FieldName(g_name_prinbee_fld_schema_version)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32) // schema_version_t (data/schema.h)
    ),
    prinbee::define_description(
          prinbee::FieldName(g_name_prinbee_fld_name)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P16STRING) // size is 100 x segment length and we can have 3 path segments + context name
    ),
    prinbee::define_description(
          prinbee::FieldName(g_name_prinbee_fld_description)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    prinbee::define_description(
          prinbee::FieldName(g_name_prinbee_fld_created_on)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_NSTIME)
    ),
    prinbee::define_description(
          prinbee::FieldName(g_name_prinbee_fld_last_updated_on)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_NSTIME)
    ),
    // the following is an identifier representing this very context;
    // to make that number unique across our entire cluster, we use time_t
    // and use the following sequence:
    //
    //     LOCK all_clusters
    //     SLEEP 1 second (this ensures that each context has a unique ID!)
    //     ID = (UINT64)NOW
    //
    prinbee::define_description(
          prinbee::FieldName(g_name_prinbee_fld_id)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT64)
    ),
    end_descriptions()
};



} // no name namespace



namespace detail
{



class context_impl
{
public:
                                        context_impl(context * c, context_setup const & setup);
                                        context_impl(context_impl const & rhs) = delete;

    context_impl &                      operator = (context_impl const & rhs) = delete;

    void                                initialize();
    void                                load_file(std::string const & filename, bool required);
    void                                from_binary(virtual_buffer::pointer_t b);
    void                                load_context(virtual_buffer::pointer_t b);
    void                                update(context_update const & new_info);
    void                                save_context();

    std::string                         get_name() const;
    table::pointer_t                    get_table(std::string const & name) const;
    table::map_t const &                list_tables() const;
    std::string const &                 get_path() const;
    schema_version_t                    get_schema_version() const;
    std::string const &                 get_description() const;
    snapdev::timespec_ex const &        get_created_on() const;
    snapdev::timespec_ex const &        get_last_updated_on() const;
    std::uint32_t                       get_id() const;
    //std::size_t                         get_config_size(std::string const & name) const;
    //std::string                         get_config_string(std::string const & name, int idx) const;
    //long                                get_config_long(std::string const & name, int idx) const;

private:
    std::string const &                 get_context_path();
    void                                verify_complex_types();
    void                                find_loop(std::string const & name, schema_complex_type::pointer_t type, std::size_t depth);

    context *                           f_context = nullptr;

    context_setup                       f_setup = context_setup(); // this includes the f_name of the context
    schema_version_t                    f_schema_version = 0;
    std::string                         f_description = std::string();
    snapdev::timespec_ex                f_created_on = snapdev::timespec_ex();
    snapdev::timespec_ex                f_last_updated_on = snapdev::timespec_ex();
    std::uint64_t                       f_id = 0;

    std::string                         f_context_path = std::string();
    std::string                         f_tables_path = std::string();
    //std::string                         f_indexes_path = std::string();
    int                                 f_lock = -1;        // TODO: lock the context so only one prinbee daemon can run against it
    table::map_t                        f_tables = table::map_t();
    //index::map_t                        f_indexes = index::map_t();
    schema_complex_type::map_pointer_t  f_schema_complex_types = schema_complex_type::map_pointer_t();
    schema_table::map_by_name_t         f_schema_tables_by_name_and_version = schema_table::map_by_name_t();
};


context_impl::context_impl(context * c, context_setup const & setup)
    : f_context(c)
    , f_setup(setup)
{
    if(!setup.is_valid())
    {
        throw invalid_parameter("the context_setup is not valid.");
    }
    f_schema_complex_types = std::make_shared<schema_complex_type::map_t>();
}


std::string const & context_impl::get_context_path()
{
    if(f_context_path.empty())
    {
        // build the path the first time we get called
        //
        f_context_path = snapdev::pathinfo::canonicalize(get_contexts_root_path(), f_setup.get_name());
    }

    return f_context_path;
}


/** \brief Initialize a context from its file on disk.
 *
 * This function is used by the prinbee daemon which handles the data on
 * disk.
 *
 * For all others, you are expected to use the from binary function
 * with the data you receive through a binary message.
 */
void context_impl::initialize()
{
    SNAP_LOG_CONFIGURATION
        << "initialize context \""
        << f_setup.get_name()
        << "\"."
        << SNAP_LOG_SEND;

    // the full path to the data (a.k.a. tables) is built from three
    // different paths and sub-paths so call get_context_path().
    //
    f_tables_path = snapdev::pathinfo::canonicalize(get_context_path(), g_tables_subpath);

    // make sure the folders exist
    //
    if(snapdev::mkdir_p(f_tables_path
            , false
            , 0700
            , f_setup.get_user()
            , f_setup.get_group()) != 0)
    {
        throw io_error(
              "could not create or access the directory of the tables at \""
            + f_tables_path
            + "\".");
    }

//    // the full path to the secondary indexes is similar to the tables path
//    // just using .../indexes/ instead of .../tables/
//    //
//    f_indexes_path = snapdev::pathinfo::canonicalize(get_context_path(), g_indexes_subpath);
//
//    // make sure the folders exist
//    //
//    if(snapdev::mkdir_p(f_indexes_path
//            , false
//            , 0700
//            , f_setup.get_user()
//            , f_setup.get_group()) != 0)
//    {
//        throw io_error(
//              "could not create or access the directory of the secondary indexes at \""
//            + f_indexes_path
//            + "\".");
//    }

    // load the context file itself; this includes info like when the
    // context was created and its current version
    //
    load_file(snapdev::pathinfo::canonicalize(get_context_path(), g_context_filename), false);

    // complex types are common to all tables (so they can appear in any
    // one of them) so these are saved in a file at the top; it also gets
    // read first since that list is passed down to each table object
    //
    load_file(snapdev::pathinfo::canonicalize(get_context_path(), g_complex_types_filename), false);

    // in order to easily share a context between computers (which we do)
    // we just keep it all in one file
    //
    // WRONG: actually we have a separate schema per version for each table;
    //        we only share the newest version with clients; and use the older
    //        versions to convert data internally
    //
    //load_file(snapdev::pathinfo::canonicalize(get_context_path(), g_schemata_filename), false);

#if 1
    //basic_xml::node::deque_t table_extensions;

    //size_t const max(f_opts->size("table_schema_path"));

    //SNAP_LOG_NOTICE
    //    << "Reading context "
    //    << max
    //    << " XML schemata."
    //    << SNAP_LOG_SEND;

    // TODO: create + load of a table could be done by a worker thread
    //

    //basic_xml::xml::map_t xml_files;

    // the first loop goes through all the files
    // it reads the XML and parses the complex types
    //
    //for(size_t idx(0); idx < max; ++idx)
    //{
        //std::string const path(f_opts->get_string("table_schema_path", idx));

    snapdev::glob_to_list<std::list<std::string>> order_list;
    if(!order_list.read_path<
              snapdev::glob_to_list_flag_t::GLOB_FLAG_ONLY_DIRECTORIES
            , snapdev::glob_to_list_flag_t::GLOB_FLAG_EMPTY>(snapdev::pathinfo::canonicalize(f_tables_path, "*")))
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << "could not read directory \""
            << f_tables_path
            << "\" for table schemata.";
        throw io_error(msg.str());
    }

    if(order_list.empty())
    {
        SNAP_LOG_DEBUG
            << "no tables found in context \""
            << f_setup.get_name()
            << "\" (full tables path: \""
            << f_tables_path
            << "\")."
            << SNAP_LOG_SEND;
    }
    else
    {
        for(auto const & table_dir : order_list)
        {
            table::pointer_t t(std::make_shared<table>(f_context, table_dir, f_schema_complex_types));
            f_tables[t->get_name()] = t;
        }

// old code for reference
//        for(auto const & filename : list)
//        {
//std::cerr << "--- reading table schema " << filename << "\n";
//            basic_xml::xml::pointer_t x(std::make_shared<basic_xml::xml>(filename));
//            xml_files[filename] = x;
//
//            basic_xml::node::pointer_t root(x->root());
//            if(root == nullptr)
//            {
//                SNAP_LOG_WARNING
//                    << filename
//                    << ": Problem reading table schema. The file will be ignored."
//                    << SNAP_LOG_SEND;
//                continue;
//            }
//
//            if(root->tag_name() != "keyspaces"
//            && root->tag_name() != "context")
//            {
//                SNAP_LOG_WARNING
//                    << filename
//                    << ": XML table declarations must be a \"keyspaces\" or \"context\". \""
//                    << root->tag_name()
//                    << "\" is not acceptable."
//                    << SNAP_LOG_SEND;
//                continue;
//            }
//
//            // complex types are defined outside of tables which allows us
//            // to use the same complex types in different tables
//            //
//            for(auto child(root->first_child()); child != nullptr; child = child->next())
//            {
//                if(child->tag_name() == "complex-type")
//                {
//                    std::string const name(child->attribute("name"));
//                    if(name_to_struct_type(name) != INVALID_STRUCT_TYPE)
//                    {
//                        SNAP_LOG_WARNING
//                            << filename
//                            << ": The name of a complex type cannot be the name of a system type. \""
//                            << name
//                            << "\" is not acceptable."
//                            << SNAP_LOG_SEND;
//                        continue;
//                    }
//                    if(f_schema_complex_types->find(name) != f_schema_complex_types->end())
//                    {
//                        SNAP_LOG_WARNING
//                            << filename
//                            << ": The complex type named \""
//                            << name
//                            << "\" is defined twice. Only the very first instance is used."
//                            << SNAP_LOG_SEND;
//                        continue;
//                    }
//                    (*f_schema_complex_types)[name] = std::make_shared<schema_complex_type>(child);
//                }
//            }
//        }
//
//        for(auto const & x : xml_files)
//        {
//            for(auto child(x.second->root()->first_child()); child != nullptr; child = child->next())
//            {
//std::cerr << "--- child tag name " << child->tag_name() << "\n";
//                if(child->tag_name() == "table")
//                {
//                    table::pointer_t t(std::make_shared<table>(f_context, child, f_schema_complex_types));
//                    f_tables[t->name()] = t;
//
//                    dbfile::pointer_t dbfile(t->get_dbfile());
//                    dbfile->set_table(t);
//                    dbfile->set_sparse(t->is_sparse());
//                }
//                else if(child->tag_name() == "table-extension")
//                {
//                    // we collect those and process them in a second
//                    // pass after we loaded all the XML files because
//                    // otherwise the table may still be missing
//                    //
//                    table_extensions.push_back(child);
//                }
//                else if(child->tag_name() == "complex-type")
//                {
//                    // already worked on; ignore in this loop
//                }
//                else
//                {
//                    // generate an error for unknown tags or ignore?
//                    //
//                    SNAP_LOG_WARNING
//                        << "Unknown tag \""
//                        << child->tag_name()
//                        << "\" within a <context> tag ignored."
//                        << SNAP_LOG_SEND;
//                }
//            }
//        }
//
//        SNAP_LOG_NOTICE
//            << "Adding "
//            << table_extensions.size()
//            << " XML schema extensions."
//            << SNAP_LOG_SEND;
//
//        for(auto const & e : table_extensions)
//        {
//            std::string const name(e->attribute("name"));
//            table::pointer_t t(get_table(name));
//            if(t == nullptr)
//            {
//                SNAP_LOG_WARNING
//                    << "Unknown table \""
//                    << e->tag_name()
//                    << "\" within a <table-extension>, tag ignored."
//                    << SNAP_LOG_SEND;
//                continue;
//            }
//            t->load_extension(e);
//        }
//
//        SNAP_LOG_NOTICE
//            << "Verify "
//            << f_tables.size()
//            << " table schema"
//            << (f_tables.size() == 1 ? "" : "ta")
//            << "."
//            << SNAP_LOG_SEND;
//
//        for(auto & t : f_tables)
//        {
//            t.second->get_schema();
//        }
    }
#endif

// at some point I was thinking that indxes could be separate from tables,
// but the OID they reference is from a specific table so we might as well
// keep them there; this also simplifies the DROP TABLE which ends up having
// to delete all the indexes defined against that table
//
//    snapdev::glob_to_list<std::list<std::string>> index_list;
//    if(!index_list.read_path<
//              snapdev::glob_to_list_flag_t::GLOB_FLAG_ONLY_DIRECTORIES
//            , snapdev::glob_to_list_flag_t::GLOB_FLAG_EMPTY>(snapdev::pathinfo::canonicalize(f_indexes_path, "*")))
//    {
//        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
//        msg << "could not read directory \""
//            << f_indexes_path
//            << "\" for secondary indexes.";
//        throw io_error(msg.str());
//    }
//
//    if(index_list.empty())
//    {
//        SNAP_LOG_DEBUG
//            << "no secondary indexes found in context \""
//            << f_setup.get_name()
//            << "\" (full indexes path: \""
//            << f_indexes_path
//            << "\")."
//            << SNAP_LOG_SEND;
//    }
//    else
//    {
//        for(auto const & index_dir : index_list)
//        {
//            index::pointer_t idx(std::make_shared<index>(f_context, index_dir));
//            f_indexes[idx->get_name()] = idx;
//        }
//    }


    SNAP_LOG_INFORMATION
        << "Context \""
        << f_setup.get_name()
        << "\" ready."
        << SNAP_LOG_SEND;
}


void context_impl::load_file(std::string const & filename, bool required)
{
    virtual_buffer::pointer_t b(std::make_shared<virtual_buffer>());
    b->load_file(filename, required);
    from_binary(b);
}


void context_impl::from_binary(virtual_buffer::pointer_t b)
{
    dbtype_t type(dbtype_t::DBTYPE_UNKNOWN);
    int const r(b->pread(&type, sizeof(type), 0, false));
    if(r == 0)
    {
        // file is empty (?!?) or non-existent (which happens on a create)
        //
        return;
    }
    if(r != sizeof(type))
    {
        throw invalid_size("could not read file type; expected a context or a complex type file.");
    }

    switch(type)
    {
    case dbtype_t::FILE_TYPE_COMPLEX_TYPE:
        schema_complex_type::load_complex_types(f_schema_complex_types, b);
        verify_complex_types();
        break;

    case dbtype_t::FILE_TYPE_CONTEXT:
        load_context(b);
        break;

    //case dbtype_t::FILE_TYPE_SCHEMA: -- incorrect; this is per table version instead
    //    {
    //        // I think this is invalid; we receive a TBL message of some sort
    //        // instead of a CTXT for a schema...
    //        //
    //        schema_table::pointer_t schema(std::make_shared<schema_table>());
    //        schema->set_complex_types(f_schema_complex_types);
    //        schema->from_binary(b);
    //        f_schema_tables_by_name_and_version[schema->get_name()] = schema;
    //    }
    //    break;

    default:
        throw invalid_type(
                  "invalid type found in binary buffer (0x"
                + snapdev::int_to_hex(static_cast<std::uint32_t>(type, false, 8))
                + ").");

    }

    //FILE_TYPE_TABLE                 = DBTYPE_NAME("PTBL"),      // Prinbee Table
    //FILE_TYPE_PRIMARY_INDEX         = DBTYPE_NAME("PIDX"),      // Primary Index (a.k.a. OID Index)
    //FILE_TYPE_INDEX                 = DBTYPE_NAME("INDX"),      // User Defined Index (key -> OID)
    //FILE_TYPE_BLOOM_FILTER          = DBTYPE_NAME("BLMF"),      // Bloom Filter
    //FILE_TYPE_SCHEMA                = DBTYPE_NAME("SCHM"),      // Table Schema [DONE]
    //FILE_TYPE_COMPLEX_TYPE          = DBTYPE_NAME("CXTP"),      // User Defined Types [DONE]
}


void context_impl::load_context(virtual_buffer::pointer_t b)
{
    structure::pointer_t s(std::make_shared<prinbee::structure>(g_context_file_description));
    s->set_virtual_buffer(b, 0);

    // we already have the name in the f_setup, so just verify
    // (TODO: if not the same, maybe we should have a process to auto-fix it?)
    //
    std::string const name(s->get_string(g_name_prinbee_fld_name));
    if(name != f_setup.get_name())
    {
        // this is considered an error, but how are we going to fix it unless
        // we can read the whole file?
        //
        SNAP_LOG_ERROR
                << "the name of the context \""
                << f_setup.get_name()
                << "\" does not match the name found in the file \""
                << name
                << "\"."
                << SNAP_LOG_SEND;
    }

    // this version is just a 32 unsigned integer
    //
    f_schema_version = s->get_uinteger(g_name_prinbee_fld_schema_version);

    f_description = s->get_string(g_name_prinbee_fld_description);
    f_created_on = s->get_nstime(g_name_prinbee_fld_created_on);
    f_last_updated_on = s->get_nstime(g_name_prinbee_fld_last_updated_on);
    f_id = s->get_uinteger(g_name_prinbee_fld_id); // 64 bits
}


void context_impl::update(context_update const & new_info)
{
    bool updated(f_created_on == snapdev::timespec_ex());

    if(f_id == 0)
    {
        updated = true;

        // make sure that each identifier is distinct by sleeping one second
        //
        // Note: it works because we have a cluster wide lock
        //
        sleep(1);
        f_id = time(nullptr);
    }

    schema_version_t const schema_version(new_info.get_schema_version());
    if(schema_version != 0
    && schema_version != f_schema_version)
    {
        if(schema_version < f_schema_version)
        {
            SNAP_LOG_MINOR
                << "attempted to save context with a smaller version aborted ("
                << schema_version
                << " < "
                << f_schema_version
                << ")."
                << SNAP_LOG_SEND;
            return;
        }

        updated = true;
        f_schema_version = schema_version;
    }

    std::string const & name(new_info.get_name());
    if(!name.empty()
    && name != f_setup.get_name())
    {
        updated = true;

        // a rename is complicated since we need to change the name of
        // each folder in the path -- the first time, though, we can
        // just save and really we do not want to throw...
        //
        //throw not_yet_implemented("context renaming is not yet implemented.");

        f_setup.set_name(name);
    }

    std::string const & description(new_info.get_description());
    if(!description.empty()
    && description != f_description)
    {
        updated = true;
        f_description = description;
    }

    if(updated)
    {
        f_last_updated_on = snapdev::now();
        if(f_created_on == snapdev::timespec_ex())
        {
            f_created_on = f_last_updated_on;
        }

        save_context();
    }
}


void context_impl::save_context()
{
    structure::pointer_t s(std::make_shared<prinbee::structure>(g_context_file_description));
    s->init_buffer();

    s->set_string(g_name_prinbee_fld_name, f_setup.get_name());
    s->set_uinteger(g_name_prinbee_fld_schema_version, f_schema_version);
    s->set_string(g_name_prinbee_fld_description, f_description);
    s->set_nstime(g_name_prinbee_fld_created_on, f_created_on);
    s->set_nstime(g_name_prinbee_fld_last_updated_on, f_last_updated_on);
    s->set_uinteger(g_name_prinbee_fld_id, f_id);

    reference_t offset(0);
    virtual_buffer::pointer_t b(s->get_virtual_buffer(offset));
    b->save_file(snapdev::pathinfo::canonicalize(get_context_path(), g_context_filename));
}


void context_impl::verify_complex_types()
{
    // in case the user made updates directly in our .ini, we could
    // have loops, make sure that's not the case
    //
    for(auto & type : *f_schema_complex_types)
    {
        if(type.second->is_enum())
        {
            continue;
        }

        find_loop(type.first, type.second, 1);
    }
}


void context_impl::find_loop(std::string const & name, schema_complex_type::pointer_t type, std::size_t depth)
{
    if(depth >= MAX_COMPLEX_TYPE_REFERENCE_DEPTH)
    {
        throw invalid_name(
                "too many complex type references from \""
              + name
              + "\"; the limit is currently "
              + std::to_string(MAX_COMPLEX_TYPE_REFERENCE_DEPTH)
              + ".");
    }

    std::size_t const size(type->get_size());
    for(std::size_t idx(0); idx < size; ++idx)
    {
        if(type->get_type(idx) != struct_type_t::STRUCT_TYPE_STRUCTURE)
        {
            continue;
        }

        std::string const & field_name(type->get_type_name(idx));
        if(field_name == name)
        {
            throw invalid_name(
                    "complex type loop found in \""
                  + type->get_name()
                  + "\".");
        }

        auto it(f_schema_complex_types->find(field_name));
        if(it == f_schema_complex_types->end())
        {
            // this should never happen
            //
            throw logic_error(
                    "complex type \""
                  + field_name
                  + "\" not found.");
        }
        find_loop(name, it->second, depth + 1);
    }
}


std::string context_impl::get_name() const
{
    return f_setup.get_name();
}


table::pointer_t context_impl::get_table(std::string const & name) const
{
    auto it(f_tables.find(name));
    if(it == f_tables.end())
    {
        return table::pointer_t();
    }

    return it->second;
}


table::map_t const & context_impl::list_tables() const
{
    return f_tables;
}


std::string const & context_impl::get_path() const
{
    return f_setup.get_name();
}


schema_version_t context_impl::get_schema_version() const
{
    return f_schema_version;
}


std::string const & context_impl::get_description() const
{
    return f_description;
}


snapdev::timespec_ex const & context_impl::get_created_on() const
{
    return f_created_on;
}


snapdev::timespec_ex const & context_impl::get_last_updated_on() const
{
    return f_last_updated_on;
}


std::uint32_t context_impl::get_id() const
{
    return f_id;
}


//size_t context_impl::get_config_size(std::string const & name) const
//{
//    return f_opts->size(name);
//}


/** \brief Retrieve a context.conf file parameter.
 *
 * This function is used to access a parameter in the context configuration
 * file.
 *
 * For example, we retrieve the murmur3 seed from that file. This way each
 * installation can make use of a different value.
 *
 * \param[in] name  The name of the parameter to retrieve.
 * \param[in] idx  The index to the data to retrieve.
 *
 * \return The parameter's value as a string.
 */
//std::string context_impl::get_config_string(std::string const & name, int idx) const
//{
//    return f_opts->get_string(name, idx);
//}


//long context_impl::get_config_long(std::string const & name, int idx) const
//{
//    return f_opts->get_long(name, idx);
//}



} // namespace detail
















context_setup::context_setup()
{
}


context_setup::context_setup(std::string const & name)
{
    set_name(name);
}


bool context_setup::is_valid() const
{
    // TODO: verify that the user name/id and group name/id are recognized
    //
    return !f_name.empty();
}


void context_setup::set_name(std::string const & name)
{
    if(name.empty())
    {
        throw invalid_parameter("the context name cannot be an empty string.");
    }
    if(snapdev::pathinfo::is_absolute(name)
    || name.back() == '/')
    {
        throw invalid_parameter(
              "a context name cannot start or end with '/', \""
            + name
            + "\" is not valid.");
    }

    std::vector<std::string> segments;
    snapdev::tokenize_string(segments, name, "/", true);
    if(segments.size() > MAX_CONTEXT_NAME_SEGMENTS)
    {
        throw invalid_parameter(
              "a context name cannot include that many '/', \""
            + name
            + "\" is not valid (limit is "
            + std::to_string(MAX_CONTEXT_NAME_SEGMENTS)
            + ").");
    }

    // the following ensures that each segment in the context name is
    // considered to be a valid name
    //
    // as a side effect, this means we make sure that there is no "." and
    // ".." segments since periods are not allowed in our names
    //
    for(auto const & s : segments)
    {
        if(!validate_name(s.c_str(), MAX_CONTEXT_NAME_SEGMENT_LENGTH))
        {
            throw invalid_name(
                    "context name segment \""
                  + s
                  + "\" is not considered valid.");
        }
    }

    // save the canonicalized version
    //
    f_name = snapdev::join_strings(segments, "/");
}


std::string const & context_setup::get_name() const
{
    return f_name;
}


void context_setup::set_user(std::string const & user)
{
    if(user.empty())
    {
        throw invalid_parameter("the user name cannot be an empty string.");
    }

    f_user = user;
}


std::string const & context_setup::get_user() const
{
    return f_user;
}


void context_setup::set_group(std::string const & group)
{
    if(group.empty())
    {
        throw invalid_parameter("the group name cannot be an empty string.");
    }

    f_group = group;
}


std::string const & context_setup::get_group() const
{
    return f_group;
}










void context_update::set_schema_version(schema_version_t version)
{
    f_schema_version = version;
}


schema_version_t context_update::get_schema_version() const
{
    return f_schema_version;
}


void context_update::set_name(std::string const & name)
{
    f_name = name;
}


std::string const & context_update::get_name() const
{
    return f_name;
}


void context_update::set_description(std::string const & description)
{
    f_description = description;
}


std::string const & context_update::get_description() const
{
    return f_description;
}










context::context(context_setup const & setup)
    : f_impl(std::make_unique<detail::context_impl>(this, setup))
{
}


// required for the std::unique_ptr<>() of the impl
context::~context()
{
}


context::pointer_t context::create_context(context_setup const & setup)
{
    pointer_t result;
    result.reset(new context(setup));
    return result;
}


/** \brief Load the schema from disk.
 *
 * When you create the context, you can pass a setup structure including
 * a name which defines the location of the schema on disk. This function
 * uses that information to read the user complex type and the table
 * schemata.
 *
 * If you are not running the prinbee server, then you cannot use this
 * function since the data is protected. Instead, you have to call
 * the from_binary() from the data sent to your client.
 */
void context::initialize()
{
    f_impl->initialize();
}


/** \brief Load a file and parse it through from_binary().
 *
 * This helper function is used to read an entire file and then 
 * parse it through the from_binary() function. This is useful
 * to load a schema from disk.
 *
 * This function is used from the initialize() function to load
 * the complex types and tables of a schema.
 *
 * On a client, we usually receive the schema data through messages
 * and process those calling the from_binary() function instead.
 *
 * \exception file_not_found
 * The function generates an exception if the \p required parameter
 * is true and we could not open the file for reading.
 *
 * \exception io_error
 * When an error occurs while reading the file, then this exception
 * is raised.
 *
 * \note
 * Other exception can occur if the data inside the file is not
 * recognized by the from_binary() function.
 *
 * \param[in] b  The buffer the data is to be read from.
 *
 * \sa from_binary()
 * \sa virtual_buffer::load_file()
 */
void context::load_file(std::string const & filename, bool required)
{
    f_impl->load_file(filename, required);
}


/** \brief Load one items from a binary buffer.
 *
 * This function reads the magic characters of the buffer from the start
 * (first four bytes). If the magic is a known file type, then the
 * context loads the data as if reading it from that file.
 *
 * Types that are currently supported:
 *
 * * Complex Types
 *
 * \raise invalid_size
 * The input buffer cannot be empty. It also has to properly represent
 * the structure that corresponds to its type.
 *
 * \raise invalid_type
 * If the magic found in the buffer is not recognized, then this exception
 * is raised.
 *
 * \param[in] b  The buffer the data is to be read from.
 */
void context::from_binary(virtual_buffer::pointer_t b)
{
    f_impl->from_binary(b);
}


std::string context::get_name() const
{
    return f_impl->get_name();
}


table::pointer_t context::get_table(std::string const & name) const
{
    return f_impl->get_table(name);
}


table::map_t const & context::list_tables() const
{
    return f_impl->list_tables();
}


std::string const & context::get_path() const
{
    return f_impl->get_path();
}


schema_version_t context::get_schema_version() const
{
    return f_impl->get_schema_version();
}


std::string const & context::get_description() const
{
    return f_impl->get_description();
}


snapdev::timespec_ex const & context::get_created_on() const
{
    return f_impl->get_created_on();
}


snapdev::timespec_ex const & context::get_last_updated_on() const
{
    return f_impl->get_last_updated_on();
}


std::uint32_t context::get_id() const
{
    return f_impl->get_id();
}


void context::update(context_update const & new_info)
{
    f_impl->update(new_info);
}


/** \brief Signal that a new allocation was made.
 *
 * This function is just a signal sent to the memory manager so it knows
 * it should check and see whether too much memory is currently used and
 * attempt to release some memory.
 *
 * \note
 * The memory manager runs in a separate thread.
 *
 * \todo
 * Actually implement the function.
 */
void context::limit_allocated_memory()
{
}


//std::size_t context::get_config_size(std::string const & name) const
//{
//    return f_impl->get_config_size(name);
//}
//
//
//std::string context::get_config_string(std::string const & name, int idx) const
//{
//    return f_impl->get_config_string(name, idx);
//}
//
//
//long context::get_config_long(std::string const & name, int idx) const
//{
//    return f_impl->get_config_long(name, idx);
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


/** \brief Get the filename used for a context file.
 *
 * This function returns the filename of a context file.
 *
 * \return The filename of a context file.
 */
char const * get_context_filename()
{
    return g_context_filename;
}



} // namespace prinbee
// vim: ts=4 sw=4 et
