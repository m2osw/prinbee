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
 * \brief Header for any of the blocks.
 *
 * All the blocks start with the exact same header. At the moment this is
 * the magic (i.e. `FREE`) and the version of that block structure for
 * easy forward compatibility handling.
 */

// self
//
#include    "prinbee/data/structure.h"



namespace prinbee
{



namespace detail
{


// all blocks start with this header which defines the block type
// and its version;
//
// the version allows us to read old versions without special handling
// written by hand each time; instead we get structures just like the
// normal structure, only that older version may include additional or
// less fields than the new version; the system will convert the old
// version to the new version automatically and if a change is made,
// it gets saved (otherwise the change only happens in memory)
//
// TODO: remove the need for this definition (i.e. we have a definition
//       for each file already with their own header)
//
constexpr struct_description_t g_block_header[] =
{
    define_description(
          FieldName("magic")    // dbtype_t such as SDBT, BLOB, SCHM
        , FieldType(struct_type_t::STRUCT_TYPE_UINT32)
    ),
    define_description(
          FieldName("version")  // this is the version of this block's structure NOT THE VERSION OF THE SCHEMA (nor the version of the database)
        , FieldType(struct_type_t::STRUCT_TYPE_VERSION)
    ),
    end_descriptions()
};


} // namespace detail

} // namespace prinbee
// vim: ts=4 sw=4 et
