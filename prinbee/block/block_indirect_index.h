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
 * \brief Block representing the database file header.
 *
 */

// self
//
#include    "prinbee/data/structure.h"



namespace prinbee
{



/** \brief Address used to represent a missing address.
 *
 * Whenever accessing a reference with an out of bounds index (OID), the
 * get_reference() may return a FILE_ADDR_MISSING (depending on the
 * \p must_exist flag).
 *
 * When this happens, the caller can react by adding a new block as
 * required.
 *
 * \todo
 * If required in blocks other than the (top) indirect index, move this
 * to the structure.h along the reference_t and NULL_FILE_ADDR definition.
 */
constexpr reference_t           MISSING_FILE_ADDR = static_cast<reference_t>(1);


class block_indirect_index
    : public block
{
public:
    typedef std::shared_ptr<block_indirect_index>
                                pointer_t;

                                block_indirect_index(dbfile::pointer_t f, reference_t offset);

    static size_t               get_start_offset();
    size_t                      get_max_count() const;

    reference_t                 get_reference(oid_t & id, bool must_exist) const;
    void                        set_reference(oid_t & id, reference_t offset);

private:
    std::uint64_t               get_position(oid_t id, reference_t const * & refs) const;

    mutable std::uint32_t       f_start_offset = 0;
    mutable size_t              f_count = 0;
};



} // namespace prinbee
// vim: ts=4 sw=4 et
