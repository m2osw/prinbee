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
 * \brief Block representing the database file header.
 *
 */

// self
//
//#include    "prinbee/data/structure.h"
#include    "prinbee/data/schema.h"



namespace prinbee
{



class block_schema_list
    : public block
{
public:
    typedef std::shared_ptr<block_schema_list>       pointer_t;

                                block_schema_list(dbfile::pointer_t f, reference_t offset);

    uint32_t                    get_count() const;
    void                        set_count(uint32_t id);

    reference_t                 get_schema(version_t const & version) const;
    void                        add_schema(schema_table::pointer_t schema);

private:
};



} // namespace prinbee
// vim: ts=4 sw=4 et
