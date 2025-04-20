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
 * \brief Block Primary Index implementation.
 *
 * The Primary Index is used to very quickly kill one layer in our search
 * without doing a search. Instead this index makes use of the last few
 * bits of the Murmur3 hash to instantly pick a block reference to use to
 * do the search of the data by primary key.
 *
 * In other words, this feature cuts down the search by a factor equal to
 * the number of `reference_t` we can fit in one block. To give you an idea,
 * searching among one million items using a binary search, you need up to 20
 * iterations (assuming all one million items are in one table ready to be
 * searched). When using the Primary Index, we cut down the 1 million by 
 * at least 512 (when your block is 4Kb which is the smallest possible)
 * which means we end up having to search about 1,954 items, which reduces
 * the binary search iterations to about 11.
 *
 * Obviously, in our case we use blocks so the search uses a B+tree and
 * it can take time to load said blocks, the number of items per block
 * defines a level which varies, etc. so the number of iterations can
 * vary wildly.
 *
 * \note
 * We use pages that have a size which is a multiple of the system page
 * size (so a power of 2) but with the header it breaks the possibility
 * to use the entire page. For this one (`PIDX`), it would be a
 * particularly bad one since we would waste 50% of the page. Since we
 * have a single one of those pages (there is only one primary index)
 * we save index zero in the header instead. That way the header is still
 * in the block and we still support 100% of the alloted space.
 * \note
 * If we want to support multiple Primary Indexes (i.e. for the Branch and
 * the Revision sub-indexes) then we probably want to look into an easy way
 * to get the "Reference Zero". Right now it's hard coded to only get the
 * primary key "Reference Zero".
 *
 * \todo
 * On small tables, this step can be made optional. However, adding this
 * block later means having to rebuild the entire Primary Index.
 */

// self
//
#include    "prinbee/block/block_primary_index.h"

#include    "prinbee/database/table.h"
#include    "prinbee/file/file_table.h"
#include    "prinbee/utils.h"


// snapdev
//
#include    <snapdev/log2.h>


// C++
//
#include    <iostream>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{





namespace
{



// 'PIDX' -- primary index
constexpr struct_description_t g_description[] =
{
    define_description(
          FieldName(g_system_field_name_magic)
        , FieldType(struct_type_t::STRUCT_TYPE_MAGIC)
        , FieldDefaultValue(to_string(dbtype_t::FILE_TYPE_PRIMARY_INDEX))
    ),
    define_description(
          FieldName(g_system_field_name_structure_version)
        , FieldType(struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , FieldVersion(0, 1)
    ),
    // all the space gets used, no room for an array here
    //define_description(
    //      FieldName("index")
    //    , FieldType(struct_type_t::STRUCT_TYPE_ARRAY32)
    //    , FieldDescription(g_index_description) -- this is just one STRUCT_TYPE_REFERENCE per entry
    //),
    end_descriptions()
};



}
// no name namespace




block_primary_index::block_primary_index(dbfile::pointer_t f, reference_t offset)
    : block(g_description, f, offset)
{
}


std::uint8_t block_primary_index::get_size() const
{
    // this is calculate in memory and the snap::log2() is just two or three
    // assembly instructions so it's dead fast (it's inline).
    //
    return static_cast<std::uint8_t>(std::min(snapdev::log2(f_table->get_page_size()) - snapdev::log2(sizeof(reference_t)), 32));
}


std::uint32_t block_primary_index::key_to_index(buffer_t key) const
{
    // retrieve `size` bits from the end of key
    //
    // note: at this time we consider that the maximum number of bits
    //       is going to be 20, so we can use 32 bit numbers
    //
    std::uint8_t const size(get_size());
    std::uint32_t bytes(divide_rounded_up(size, 8));
    std::size_t idx(key.size());
    if(bytes > idx)
    {
        bytes = idx;
    }
    std::uint32_t shift(0);
    std::uint32_t k(0);
    while(bytes > 0)
    {
        --bytes;
        --idx;
        k |= key[idx] << shift;
        shift += 8;
    }

    std::uint32_t mask(static_cast<std::uint32_t>(-1));
    mask <<= size;
    return k & ~mask;
}


reference_t block_primary_index::get_top_index(buffer_t const & key) const
{
    std::uint32_t const k(key_to_index(key));
    if(k == 0)
    {
        // this position is where we have the header and version for this
        // block so we have to use a different location, we use the header
        //
        file_table::pointer_t header(std::static_pointer_cast<file_table>(get_table()->get_block(0)));
        return header->get_primary_index_reference_zero();
    }
    else
    {
        return reinterpret_cast<reference_t const *>(data())[k];
    }
}


void block_primary_index::set_top_index(buffer_t const & key, reference_t offset)
{
    std::uint32_t const k(key_to_index(key));
    if(k == 0)
    {
        // this position is where we have the header and version for this
        // block so we have to use a different location, we use the header
        //
        file_table::pointer_t header(std::static_pointer_cast<file_table>(get_table()->get_block(0)));
        header->set_primary_index_reference_zero(offset);
    }
    else
    {
        reinterpret_cast<reference_t *>(data())[k] = offset;
    }
}



} // namespace prinbee
// vim: ts=4 sw=4 et
