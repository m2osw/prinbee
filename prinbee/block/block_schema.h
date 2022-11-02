// Copyright (c) 2019-2022  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Block representing the schema.
 *
 * This block is used to represent the schema of the table. If the schema
 * is pretty large, multiple blocks can be chained together. The schema
 * itself is defined in the schema.cpp/h file.
 */

// self
//
#include    "prinbee/data/structure.h"




namespace prinbee
{



class block_schema
    : public block
{
public:
    typedef std::shared_ptr<block_schema>       pointer_t;

                                block_schema(dbfile::pointer_t f, reference_t offset);

    size_t                      get_structure_size();
    uint32_t                    get_size();
    void                        set_size(uint32_t size);
    reference_t                 get_next_schema_block();
    void                        set_next_schema_block(reference_t offset);

    virtual_buffer::pointer_t   get_schema() const;
    void                        set_schema(virtual_buffer::pointer_t schema);

private:
};



} // namespace prinbee
// vim: ts=4 sw=4 et
