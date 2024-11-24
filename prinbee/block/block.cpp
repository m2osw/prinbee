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


/** \file
 * \brief Base block implementation.
 *
 * The block base class handles the loading of the block in memory using
 * mmap() and gives information such as its type and location.
 */

// self
//
#include    "prinbee/block/block.h"

#include    "prinbee/exception.h"
#include    "prinbee/database/table.h"
#include    "prinbee/data/structure.h"


// snaplogger
//
#include    <snaplogger/message.h>


// C++
//
#include    <iostream>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



namespace
{


constexpr char const * g_errmsg_table = "block::~block() called with an f_data pointer, but f_table == nullptr.";
constexpr char const * g_errmsg_exception = "block::~block() tried to release the f_data by it threw an exception.";


}



block::block(struct_description_t const * descriptions, dbfile::pointer_t f, reference_t offset)
    : f_file(f)
    , f_offset(offset)
{
#ifdef _DEBUG
    // verify that the start of the descriptions is valid
    //
    if(descriptions == nullptr
    && descriptions[0].f_type != struct_type_t::STRUCT_TYPE_END)
    {
        throw logic_error("the array of structure descriptions cannot be empty.");
    }
#endif

    if(descriptions[0].f_type != struct_type_t::STRUCT_TYPE_MAGIC
    && descriptions[1].f_type != struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
    {
        throw logic_error("the structure description must start with a MAGIC and STRUCTURE_VERSION.");
    }

    // create the structure
    //
    f_structure = std::make_shared<structure>(descriptions);

    // get the current version
    //
    //f_version = f_structure->get_version("_structure_version");
}


block::~block()
{
    if(f_data != nullptr)
    {
        if(f_table == nullptr)
        {
            SNAP_LOG_FATAL
                << g_errmsg_table
                << SNAP_LOG_SEND;
            std::cerr << g_errmsg_table << std::endl;
            std::terminate();
        }

        try
        {
            f_table->get_dbfile()->release_data(f_data);
            //f_data = nullptr;
        }
        catch(page_not_found const & e)
        {
            SNAP_LOG_FATAL
                << g_errmsg_exception
                << " ("
                << e.what()
                << ")."
                << SNAP_LOG_SEND;
            std::cerr
                << g_errmsg_exception
                << " ("
                << e.what()
                << ")."
                << std::endl;
            std::terminate();
        }
    }
}


table_pointer_t block::get_table() const
{
    if(f_table == nullptr)
    {
        throw not_ready("block::get_table() called before the table was defined.");
    }

    return f_table;
}


void block::set_table(table_pointer_t table)
{
    if(f_table != nullptr)
    {
        throw defined_twice("block::set_table() called twice.");
    }

    f_table = table;
}


structure::pointer_t block::get_structure() const
{
    return f_structure;
}


// version is now integrated (burned) in the description
//
//structure::pointer_t block::get_structure(version_t version) const
//{
//    for(descriptions_by_version_t const * d = f_structure_descriptions;
//                                          d->f_description != nullptr;
//                                          ++d)
//    {
//        if(d->f_version == version)
//        {
//            return std::make_shared<structure>(f_structure_descriptions->f_description);
//        }
//    }
//
//    throw logic_error(
//              "Block of type \""
//            + to_string(get_dbtype())
//            + "\" has no structure version "
//            + version.to_string()
//            + ".");
//}


void block::clear_block()
{
    reference_t const offset(f_structure->get_static_size());
#ifdef _DEBUG
    if(offset == 0)
    {
        throw logic_error("the structure of the block_free_block block cannot be dynamic.");
    }
#endif
    std::uint32_t const data_size(get_table()->get_page_size() - offset);

    memset(data(offset), 0, data_size);
}


dbtype_t block::get_dbtype() const
{
    return *reinterpret_cast<dbtype_t const *>(data(0));
}


void block::set_dbtype(dbtype_t type)
{
    // TODO: add verifications (i.e. go from FREE to any or any to FREE
    //       and maybe a few others)
    //
    if(*reinterpret_cast<dbtype_t *>(data(0)) != type)
    {
        *reinterpret_cast<dbtype_t *>(data(0)) = type;

        reference_t const size(f_structure->get_static_size());
        memset(data(sizeof(dbtype_t))
             , 0
             , size - sizeof(dbtype_t));
    }
}


version_t block::get_structure_version() const
{
    //return f_version;
    return version_t();
}


void block::set_structure_version()
{
    // TODO: this is not available in the newer version, we have to always
    //       use the latest on a write... a read is a TODO at the moment
    //
    //f_structure->set_version("_structure_version", f_version);
    f_structure->set_version(g_system_field_name_structure_version, version_t());
}


reference_t block::get_offset() const
{
    return f_offset;
}


void block::set_data(data_t data)
{
    // the table retrieves the data pointer because it needs to determine
    // the block type (using the first 4 bytes) and so the data pointer
    // is already locked once and we can immediately save it in the block
    //
    f_data = data;
}


data_t block::data(reference_t offset)
{
    if(f_data == nullptr)
    {
        //f_data = get_table()->get_dbfile()->data(f_offset);
        throw logic_error("block::data() called before set_data().");
    }

    return f_data + (offset % get_table()->get_page_size());
}


const_data_t block::data(reference_t offset) const
{
    if(f_data == nullptr)
    {
        //f_data = get_table()->get_dbfile()->data(f_offset);
        throw logic_error("block::data() called before set_data().");
    }

    return f_data + (offset % get_table()->get_page_size());
}


void block::sync(bool immediate)
{
    get_table()->get_dbfile()->sync(f_data, immediate);
}


void block::from_current_file_version()
{
    version_t current_version(get_structure_version());
    // TODO: the version is not managed that way anymore
    if(/*f_structure_descriptions->f_version*/ version_t() == current_version)
    {
        // same version, no conversion necessary
        //
        return;
    }

    throw logic_error("from_current_file_version() not fully implemented yet.");
}



} // namespace prinbee
// vim: ts=4 sw=4 et
