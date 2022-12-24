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
 * \brief This file is used to generate a Bloom Filter.
 *
 * The Bloom Filter is an external file because it would otherwise require
 * many linked blocks and that would not be efficient at all. Also this
 * will simplify the code used to grow the size over time for tables
 * that require it.
 *
 * The class defines how the data is organized in the file. The first
 * 1Kb are used to define the table and the rest is the actual
 * Bloom Filter. In most case, we will lazily load the file using
 * mmap() against the entire file.
 */

// self
//
#include    "prinbee/data/structure.h"



namespace prinbee
{



class file_bloom_filter
    : public block
{
public:
    typedef std::shared_ptr<file_bloom_filter>       pointer_t;

                                file_bloom_filter(dbfile::pointer_t f, reference_t offset);


private:
};



} // namespace prinbee
// vim: ts=4 sw=4 et
