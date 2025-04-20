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
#include    "prinbee/block/block_free_block.h"

#include    "prinbee/database/table.h"


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



namespace
{



// 'FREE'
constexpr struct_description_t g_description[] =
{
    define_description(
          FieldName(g_system_field_name_magic)
        , FieldType(struct_type_t::STRUCT_TYPE_MAGIC)
        , FieldDefaultValue(to_string(dbtype_t::BLOCK_TYPE_FREE_BLOCK))
    ),
    define_description(
          FieldName(g_system_field_name_structure_version)
        , FieldType(struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , FieldVersion(0, 1)
    ),
    define_description(
          FieldName("next_free_block")
        , FieldType(struct_type_t::STRUCT_TYPE_REFERENCE)
    ),
    // the rest of these blocks are all zeroes
    end_descriptions()
};



}
// no name namespace





block_free_block::block_free_block(dbfile::pointer_t f, reference_t offset)
    : block(g_description, f, offset)
{
}


reference_t block_free_block::get_next_free_block() const
{
    return static_cast<reference_t>(f_structure->get_uinteger("next_free_block"));
}


void block_free_block::set_next_free_block(reference_t offset)
{
    f_structure->set_uinteger("next_free_block", offset);
}



} // namespace prinbee
// vim: ts=4 sw=4 et
