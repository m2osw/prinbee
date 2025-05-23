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
#include    "prinbee/data/structure.h"



namespace prinbee
{



class block_top_index
    : public block
{
public:
    typedef std::shared_ptr<block_top_index>       pointer_t;

                                block_top_index(dbfile::pointer_t f, reference_t offset);

    std::uint32_t               get_count() const;
    void                        set_count(std::uint32_t id);
    std::uint32_t               get_size() const;
    void                        set_size(std::uint32_t size);

    reference_t                 find_index(buffer_t key) const;
    std::uint32_t               get_position() const;

private:
    mutable std::uint32_t       f_position = 0;
};



} // namespace prinbee
// vim: ts=4 sw=4 et
