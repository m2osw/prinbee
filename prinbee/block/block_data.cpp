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
 * \brief Database file implementation.
 *
 * Each table uses one or more files. Each file is handled by a dbfile
 * object and a corresponding set of blocks.
 */

// self
//
#include    "prinbee/block/block_data.h"

#include    "prinbee/database/table.h"


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



namespace detail
{
}


namespace
{


// 'DATA'
constexpr struct_description_t const g_description[] =
{
    define_description(
          FieldName(g_system_field_name_magic)
        , FieldType(struct_type_t::STRUCT_TYPE_MAGIC)
        , FieldDefaultValue(to_string(dbtype_t::BLOCK_TYPE_DATA))
    ),
    define_description(
          FieldName(g_system_field_name_structure_version)
        , FieldType(struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , FieldVersion(0, 1)
    ),
    end_descriptions()
};


}
// no name namespace


block_data::block_data(dbfile::pointer_t f, reference_t offset)
    : block(g_description, f, offset)
{
}


std::uint8_t * block_data::data_start()
{
    return data() + HEADER_SIZE;
}


std::uint32_t block_data::block_total_space(table::pointer_t t)
{
    return t->get_page_size() - HEADER_SIZE;
}



} // namespace prinbee
// vim: ts=4 sw=4 et
