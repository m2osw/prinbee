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
 * \brief The virtual buffer implementation.
 *
 * The virtual buffer allows us to access data which is not defined in one
 * straight memory buffer but instead scattered between blocks on disk and
 * memory buffers (when the amount of data increases we allocate temporary
 * memory buffers until we flush the data to file).
 *
 * It can also be used as a buffer in memory. An efficient way to manage
 * large amount of data by allocating separate buffers instead of resizing
 * buffers.
 */

// self
//
#include    "prinbee/data/virtual_buffer.h"

#include    "prinbee/exception.h"


// snaplogger
//
#include    <snaplogger/message.h>


// C++
//
#include    <iomanip>
#include    <iostream>
#include    <fstream>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



virtual_buffer::vbuf_t::vbuf_t()
{
}


virtual_buffer::vbuf_t::vbuf_t(block::pointer_t b, std::uint64_t offset, std::uint64_t size)
    : f_block(b)
    , f_offset(offset)
    , f_size(size)
{
}



virtual_buffer::virtual_buffer()
{
}


virtual_buffer::virtual_buffer(block::pointer_t b, std::uint64_t offset, std::uint64_t size)
{
    add_buffer(b, offset, size);
}


void virtual_buffer::load_file(std::string const & filename, bool required)
{
    if(f_modified
    || !f_buffers.empty())
    {
        throw logic_error(
                "virtual buffer was modified or is not empty, the load_file() only"
                " works on empty virtual buffers.");
    }

    std::ifstream in;
    in.open(filename, std::ios::in | std::ios::binary);
    if(!in.is_open())
    {
        if(required)
        {
            throw file_not_found(
                      "could not open file \""
                    + filename
                    + "\" for reading.");
        }
        return;
    }

    in.seekg(0, std::ios::end);
    std::ifstream::pos_type const size(in.tellg());
    if(std::ifstream::pos_type(-1) == size)
    {
        // TBD: in snapdev, the contents::read_all() function is able to
        //      read data from all sorts of files and we may want to support
        //      such here too; in that case we can read 4K buffers in a loop
        //
        throw io_error(
                  "could not retrieve size of file \""
                + filename
                + "\".");
    }
    in.seekg(0, std::ios::beg);

    vbuf_t file_data;
    file_data.f_data.reserve((size + 4095L) & -4096L);
    file_data.f_data.resize(size);
    file_data.f_size = size;

    in.read(reinterpret_cast<char *>(file_data.f_data.data()), size);

    if(in.bad())
    {
        throw io_error(
                  "I/O error reading file \""
                + filename
                + "\".");
    }

    f_buffers.push_back(file_data);

    f_total_size += size;
}


void virtual_buffer::add_buffer(block::pointer_t b, std::uint64_t offset, std::uint64_t size)
{
    if(f_modified)
    {
        throw logic_error(
                "virtual buffer was already modified, you cannot add"
                " another buffer until you commit this virtual buffer.");
    }

    vbuf_t const append(b, offset, size);
    f_buffers.push_back(append);

    f_total_size += size;
}


bool virtual_buffer::modified() const
{
    return f_modified;
}


std::size_t virtual_buffer::count_buffers() const
{
    return f_buffers.size();
}


std::uint64_t virtual_buffer::size() const
{
    return f_total_size;
}


std::uint64_t virtual_buffer::start_offset() const
{
    if(f_buffers.empty())
    {
        return 0;
    }

    return f_buffers.begin()->f_offset;
}


bool virtual_buffer::is_data_available(std::uint64_t offset, std::uint64_t size) const
{
    return offset + size <= f_total_size;
}


int virtual_buffer::pread(void * buf, std::uint64_t size, std::uint64_t offset, bool full) const
{
    if(size == 0)
    {
        return 0;
    }

    if(full
    && !is_data_available(offset, size))
    {
        throw invalid_size(
                "not enough data to read from virtual buffer. Requested to read "
                + std::to_string(size)
                + " bytes at "
                + std::to_string(offset)
                + ", when the buffer is "
                + std::to_string(f_total_size)
                + " bytes total (missing: "
                + std::to_string((offset + size) - f_total_size)
                + " bytes).");
    }

    std::uint64_t bytes_read(0);
    for(auto const & b : f_buffers)
    {
        if(offset >= b.f_size)
        {
            offset -= b.f_size;
        }
        else
        {
            auto sz(size);
            if(sz > b.f_size - offset)
            {
                sz = b.f_size - offset;
            }
            if(b.f_block != nullptr)
            {
                memcpy(buf, b.f_block->data() + b.f_offset + offset, sz);
            }
            else
            {
                memcpy(buf, b.f_data.data() + offset, sz);
            }
            size -= sz;
            bytes_read += sz;

            if(size == 0)
            {
                return bytes_read;
            }

            buf = reinterpret_cast<char *>(buf) + sz;

            offset = 0;
        }
    }

    return bytes_read;
}


int virtual_buffer::pwrite(void const * buf, std::uint64_t size, std::uint64_t offset, bool allow_growth)
{
    if(size == 0)
    {
        return 0;
    }

    if(!allow_growth
    && !is_data_available(offset, size))
    {
        throw invalid_size(
                  "not enough space to write to virtual buffer. Requested to write "
                + std::to_string(size)
                + " bytes at offset "
                + std::to_string(offset)
                + ", when the buffer is "
                + std::to_string(f_total_size)
                + " bytes only.");
    }

    std::uint8_t const * in(reinterpret_cast<std::uint8_t const *>(buf));
    std::uint64_t bytes_written(0);

    auto check_modified = [&]()
        {
            if(!f_modified && bytes_written != 0)
            {
                f_modified = true;
            }
        };

    for(auto & b : f_buffers)
    {
        if(offset >= b.f_size)
        {
            offset -= b.f_size;
        }
        else
        {
            auto sz(size);
            if(sz > b.f_size - offset)
            {
                sz = b.f_size - offset;
            }
            if(b.f_block != nullptr)
            {
                memcpy(b.f_block->data() + offset, in, sz);
            }
            else
            {
                memcpy(b.f_data.data() + offset, in, sz);
            }
            size -= sz;
            bytes_written += sz;

            if(size == 0)
            {
                check_modified();
                return bytes_written;
            }

            in += sz;

            offset = 0;
        }
    }

    // TBD: this should never happen
    //
    if(size == 0)
    {
        check_modified();
        return bytes_written;
    }

    if(!f_buffers.empty()
    && f_buffers.back().f_block == nullptr)
    {
        std::uint64_t const available(f_buffers.back().f_data.capacity() - f_buffers.back().f_size);
        if(available > 0)
        {
            auto const sz(std::min(available, size));
            f_buffers.back().f_data.resize(f_buffers.back().f_size + sz);
            memcpy(f_buffers.back().f_data.data() + f_buffers.back().f_size, in, sz);
            size -= sz;
            bytes_written += sz;
            f_buffers.back().f_size += sz;
            f_total_size += sz;

            if(size == 0)
            {
                check_modified();
                return bytes_written;
            }

            in += sz;
        }
    }

    // TBD: we may want to allocate multiple buffers of 4Kb instead of
    //      a buffer large enough for this data? At the same time, we
    //      can't always save exactly 4Kb of data in the blocks anyway...
    //      but with each block having the same size, we could have a
    //      vector to access any one block (perase() and pinsert() are
    //      a bigger issue in that case, though)
    //
    //      however, we could probably use a form of binary search to
    //      reduce the number of iterations; yet, after _many_ insertions
    //      it is likely that such a search would not be working very well
    //      and either way we'd need to update offsets through the entire
    //      list of items
    //
    //      on the other hand maybe we could use a larger buffer such
    //      as 64Kb at once to avoid too many allocations total
    //      (or use a hint / user settings / stats / ...)
    //
    vbuf_t append;
    append.f_data.reserve((size + 4095) & -4096);
    append.f_data.resize(size);
    append.f_size = size;

    memcpy(append.f_data.data(), in, size);

    f_buffers.push_back(append);

    bytes_written += size;
    f_total_size += size;

    check_modified();
    return bytes_written;
}


int virtual_buffer::pinsert(void const * buf, std::uint64_t size, std::uint64_t offset)
{
    // avoid an insert if possible
    //
    if(size == 0)
    {
        return 0;
    }

    if(offset >= f_total_size)
    {
        return pwrite(buf, size, offset, true);
    }

    std::uint8_t const * in(reinterpret_cast<std::uint8_t const *>(buf));

    // insert has to happen... search the buffer where it will happen
    //
    for(auto b(f_buffers.begin()); b != f_buffers.end(); ++b)
    {
        if(offset >= b->f_size)
        {
            offset -= b->f_size;
        }
        else
        {
            if(b->f_block != nullptr)
            {
                // if inserting within a block, we have to break the block
                // in two
                {
                    vbuf_t append;
                    append.f_block = b->f_block;
                    append.f_size = b->f_size - offset;
                    append.f_offset = b->f_offset + offset;
                    f_buffers.insert(b + 1, append);
                }

                b->f_size = offset;

                {
                    vbuf_t append;
                    append.f_size = size;
                    memcpy(append.f_data.data(), in, size);
                    f_buffers.insert(b + 1, append);
                }
            }
            else
            {
                b->f_data.insert(b->f_data.begin() + offset, in, in + size);
                b->f_size += size;
            }
            f_total_size += size;
            f_modified = true;
            return size;
        }
    }

    if(offset == 0)
    {
        // append at the end
        //
        if(!f_buffers.empty()
        && f_buffers.back().f_block == nullptr)
        {
            f_buffers.back().f_data.insert(f_buffers.back().f_data.end(), in, in + size);
        }
        else
        {
            vbuf_t append;
            append.f_size = size;
            memcpy(append.f_data.data(), in, size);
            f_buffers.push_back(append);
        }
        f_total_size += size;
        f_modified = true;
        return size;
    }

    throw logic_error(
              "reached the end of the pinsert() function. Offset should be 0, it is "
            + std::to_string(offset)
            + " instead, which should never happen.");
}


int virtual_buffer::perase(std::uint64_t size, std::uint64_t offset)
{
    if(size == 0)
    {
SNAP_LOG_ERROR << "--- perase CASE 1 -- size == 0" << SNAP_LOG_SEND;
        return 0;
    }

    if(offset >= f_total_size)
    {
SNAP_LOG_ERROR << "--- perase CASE 2 -- offset (" << offset
<< ") >= total size (" << f_total_size << ")" << SNAP_LOG_SEND;
        return 0;
    }

    // clamp the amount of data we can erase
    //
    if(size > f_total_size - offset)
    {
SNAP_LOG_ERROR << "--- perase CASE 3 -- adjust size" << SNAP_LOG_SEND;
        size = f_total_size - offset;
    }

    // since we are going to erase/add some buffers (eventually)
    // we need to use our own iterator
    //
    std::uint64_t bytes_erased(0);
    for(auto it(f_buffers.begin()); it != f_buffers.end() && size > 0; )
    {
        auto next(it + 1);
        if(offset >= it->f_size)
        {
//SNAP_LOG_ERROR << "--- perase CASE 4 -- skip early block" << SNAP_LOG_SEND;
            offset -= it->f_size;
        }
        else
        {
            if(offset + size >= it->f_size)
            {
                if(offset == 0)
                {
SNAP_LOG_ERROR << "--- perase CASE 5 -- delete whole block" << SNAP_LOG_SEND;
                    // remove this entry entirely
                    //
                    size -= it->f_size;
                    f_total_size -= it->f_size;
                    bytes_erased += it->f_size;
                    next = f_buffers.erase(it);
//if(it != next)
//{
//throw logic_error("it != next after a buffer erase?!");
//}
                }
                else
                {
SNAP_LOG_ERROR << "--- perase CASE 6 -- delete end of data" << SNAP_LOG_SEND;
                    // remove end of this block
                    //
                    std::uint64_t const sz(it->f_size - offset);
                    it->f_size = offset;
                    size -= sz;
                    f_total_size -= sz;
                    bytes_erased += sz;
                    offset = 0;
                }
            }
            else
            {
                if(offset == 0)
                {
                    // remove the start of this block
                    //
                    if(it->f_block != nullptr)
                    {
SNAP_LOG_ERROR << "--- perase CASE 7 -- block involved?!?" << SNAP_LOG_SEND;
                        it->f_offset += size;
                    }
                    else
                    {
SNAP_LOG_ERROR << "--- perase CASE 8 -- delete start of data size=" << size << " vs f_size=" << it->f_size << " vs data.size=" << it->f_data.size() << SNAP_LOG_SEND;
                        //it->f_data.erase(it->f_data.begin(), it->f_data.begin() + size); -- this is much heavier if the implementation decides to realloc()
                        memmove(it->f_data.data(), it->f_data.data() + size, it->f_size - size);
                    }
                    it->f_size -= size;
                    f_total_size -= size;
                    bytes_erased += size;
                    size = 0;
                }
                else
                {
                    // remove data from the middle of the block
                    //
                    if(it->f_block != nullptr)
                    {
                        if(offset + size >= it->f_size)
                        {
SNAP_LOG_ERROR << "--- perase CASE 9 -- block involved?!?" << SNAP_LOG_SEND;
                            it->f_offset += offset;
                            it->f_size -= size;
                        }
                        else
                        {
SNAP_LOG_ERROR << "--- perase CASE 10 -- block involved?!?" << SNAP_LOG_SEND;
                            vbuf_t append;
                            append.f_block = it->f_block;
                            append.f_size = it->f_size - size - offset;
                            append.f_offset = it->f_offset + size + offset;
                            f_buffers.insert(it, append);

                            it->f_size = offset;
                        }
                    }
                    else
                    {
SNAP_LOG_ERROR << "--- perase CASE 11 -- delete inside of block: offset=" << offset << ", offset+size=" << offset + size << ", data size=" << it->f_data.size() << SNAP_LOG_SEND;
                        //it->f_data.erase(it->f_data.begin() + offset, it->f_data.begin() + offset + size);
                        memmove(it->f_data.data() + offset, it->f_data.data() + offset + size, it->f_size - offset - size);
                        it->f_size -= size;
                        f_total_size -= size;
                        bytes_erased += size;
                        size = 0;
                    }
                }
                // here size is always zero since
                //
#ifdef _DEBUG
                if(size != 0)
                {
                    throw logic_error("size != 0 -- there is a bug in perase().");
                }
#endif
                break;
            }
        }
        it = next;
    }

    f_modified = bytes_erased != 0;
    return bytes_erased;
}


int virtual_buffer::pshift(std::int64_t size, std::uint64_t offset, std::uint8_t in)
{
    if(size == 0)
    {
        return 0;
    }

    char buf[4096];
    std::uint64_t bytes_moved(0);
    if(size > 0)
    {
        throw invalid_size("Shift right not yet implemented.");
    }
    else
    {
        //          +---------------+
        //          |               |
        //          v               |
        // +-------+----------------+--------+
        //         ^                         ^
        //         |<--------------->        |
        //         |        size             +--- f_total_size
        //         |
        //         +--- offset
        //

        // TODO: optimize with a memmove() + memset() when there is a single vbuf

        size = -size;
        std::size_t total_size(f_total_size - offset - size);
        while(total_size > 0)
        {
            int const sz(std::min(sizeof(buf), total_size));
            if(pread(buf, sz, offset + size, true) != sz)
            {
                // LCOV_EXCL_START
                throw io_error(
                      "expected to read "
                    + std::to_string(sz)
                    + " bytes from virtual buffer.");
                // LCOV_EXCL_STOP
            }
            if(pwrite(buf, sz, offset, false) != sz)
            {
                // LCOV_EXCL_START
                throw io_error(
                      "expected to write "
                    + std::to_string(sz)
                    + " bytes from virtual buffer.");
                // LCOV_EXCL_STOP
            }
            offset += sz;
            total_size -= sz;
            bytes_moved += sz;
        }
        memset(buf, in, std::min(sizeof(buf), f_total_size - offset));
        while(offset < f_total_size)
        {
            int const sz(std::min(sizeof(buf), f_total_size - offset));
            if(pwrite(buf, sz, offset, false) != sz)
            {
                // LCOV_EXCL_START
                throw io_error(
                      "expected to write "
                    + std::to_string(sz)
                    + " bytes from virtual buffer.");
                // LCOV_EXCL_STOP
            }
            offset += sz;
            bytes_moved += sz;
        }
    }

    return bytes_moved;
}



std::ostream & operator << (std::ostream & out, virtual_buffer const & v)
{
    // using a separate stringstream makes it more multi-threading impervious
    // (because flags are not shared well otherwise)
    //
    std::stringstream ss;

    ss << std::hex << std::setfill('0');

    std::uint8_t buf[16];
    char const * newline("");
    std::uint64_t sz(v.size());
    prinbee::reference_t p(0);
    for(; p < sz; ++p)
    {
        char const * sep(" ");
        if(p % 16 == 0)
        {
            if(*newline != '\0')
            {
                ss << "  ";
                for(prinbee::reference_t offset(0); offset < 16; ++offset)
                {
                    if(buf[offset] >= ' ' && buf[offset] < 0x7F)
                    {
                        ss << static_cast<char>(buf[offset]);
                    }
                    else
                    {
                        ss << '.';
                    }
                }
            }
            if(sz > 65536)
            {
                ss << newline << std::setw(8) << p << ": ";
            }
            else
            {
                ss << newline << std::setw(4) << p << ": ";
            }
            newline = "\n";
        }
        else if(p % 16 == 8)
        {
            sep = "  ";
        }

        std::uint8_t c;
        if(v.pread(&c, sizeof(c), p) != 1)
        {
            throw io_error("Expected to read 1 more byte from virtual buffer.");
        }
        buf[p % 16] = c;

        ss << sep << std::setw(2) << static_cast<int>(c);
    }

    prinbee::reference_t q(p % 16);
    if(q != 0)
    {
        for(prinbee::reference_t r(q); r < 16; ++r)
        {
            ss << "   ";
        }

        ss << "  ";
        for(prinbee::reference_t offset(0); offset < q; ++offset)
        {
            if(buf[offset] >= ' ' && buf[offset] < 0x7F)
            {
                ss << static_cast<char>(buf[offset]);
            }
            else
            {
                ss << '.';
            }
        }
    }

    ss << '\n';
    return out << ss.str();
}



} // namespace prinbee
// vim: ts=4 sw=4 et
