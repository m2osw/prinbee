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
 * \brief Block representing free space that can be allocated.
 *
 * This block is a _free_ blok meaning that it is not current used for
 * anything. It is part of the list of free blocks (linked list).
 */

// self
//
#include    "prinbee/data/structure.h"



namespace prinbee
{



class table;
typedef std::shared_ptr<table>      table_pointer_t;

class block_free_block
    : public block
{
public:
    typedef std::shared_ptr<block_free_block>       pointer_t;

                                block_free_block(dbfile::pointer_t f, reference_t offset);

    reference_t                 get_next_free_block() const;
    void                        set_next_free_block(reference_t offset);

private:
};



} // namespace prinbee
// vim: ts=4 sw=4 et
