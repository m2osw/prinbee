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
 * \brief Index Pointer block implementation.
 *
 * In a secondary index, one key match may not be unique. When that happens,
 * the list of rows that match the secondary index is listed in an
 * Index Pointer block. The address in the `EIDX` points to an array of
 * a list of pointers (`oid_t`, really).
 *
 * \todo
 * Determine how to properly grow such lists because that's not too easy
 * in the way it is defined now.
 */

// self
//
#include    "prinbee/block/block_index_pointers.h"



// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



namespace
{



// 'IDXP' -- index pointers
constexpr struct_description_t g_description[] =
{
    define_description(
          FieldName(g_system_field_name_magic)
        , FieldType(struct_type_t::STRUCT_TYPE_MAGIC)
        , FieldDefaultValue(to_string(dbtype_t::BLOCK_TYPE_INDEX_POINTERS))
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




block_index_pointers::block_index_pointers(dbfile::pointer_t f, reference_t offset)
    : block(g_description, f, offset)
{
}






} // namespace prinbee
// vim: ts=4 sw=4 et
