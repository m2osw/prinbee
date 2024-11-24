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

// self
//
#include    "catch_main.h"


// prinbee
//
#include    <prinbee/data/virtual_buffer.h>


// advgetopt
//
#include    <advgetopt/options.h>


// last include
//
#include    <snapdev/poison.h>



CATCH_TEST_CASE("virtual_buffer", "[virtual-buffer]")
{
    CATCH_START_SECTION("virtual_buffer: simple write + read")
    {
        prinbee::virtual_buffer::pointer_t v(std::make_shared<prinbee::virtual_buffer>());
        CATCH_REQUIRE(v->size() == 0);
        CATCH_REQUIRE(v->count_buffers() == 0);

        constexpr std::size_t const buf_size(1024);
        char buf[buf_size];
        for(std::size_t i(0); i < sizeof(buf); ++i)
        {
            buf[i] = rand();
        }
        CATCH_REQUIRE(v->pwrite(buf, sizeof(buf), 0, true) == sizeof(buf));

        CATCH_REQUIRE(v->size() == sizeof(buf));
        CATCH_REQUIRE(v->count_buffers() == 1);  // one write means at most 1 buffer

        char saved[buf_size];
        CATCH_REQUIRE(v->pread(saved, sizeof(saved), 0, true) == sizeof(saved));

        CATCH_REQUIRE(sizeof(buf) == sizeof(saved));
        CATCH_REQUIRE(memcmp(buf, saved, sizeof(buf)) == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("virtual_buffer: write once + read many times")
    {
        prinbee::virtual_buffer::pointer_t v(std::make_shared<prinbee::virtual_buffer>());
        CATCH_REQUIRE(v->size() == 0);
        CATCH_REQUIRE(v->count_buffers() == 0);

        constexpr size_t const buf_size(1024);
        char buf[buf_size];
        for(size_t i(0); i < sizeof(buf); ++i)
        {
            buf[i] = rand();
        }
        CATCH_REQUIRE(v->pwrite(buf, sizeof(buf), 0, true) == sizeof(buf));

        CATCH_REQUIRE(v->size() == sizeof(buf));
        CATCH_REQUIRE(v->count_buffers() == 1);  // one write means at most 1 buffer

        for(size_t i(0); i < sizeof(buf); ++i)
        {
            char c;
            CATCH_REQUIRE(v->pread(&c, sizeof(c), i, true) == sizeof(c));
            CATCH_REQUIRE(buf[i] == c);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("virtual_buffer: short write + read several times")
    {
        prinbee::virtual_buffer::pointer_t v(std::make_shared<prinbee::virtual_buffer>());
        CATCH_REQUIRE(v->size() == 0);
        CATCH_REQUIRE(v->count_buffers() == 0);

        constexpr size_t const buf_size(1024);
        char buf[buf_size];
        for(size_t i(0); i < sizeof(buf); ++i)
        {
            buf[i] = rand();
        }
        CATCH_REQUIRE(v->pwrite(buf, sizeof(buf), 0, true) == sizeof(buf));

        CATCH_REQUIRE(v->size() == sizeof(buf));
        CATCH_REQUIRE(v->count_buffers() == 1);  // one write means at most 1 buffer

        // update the first 4 bytes
        //
        buf[0] = rand();
        buf[1] = rand();
        buf[2] = rand();
        buf[3] = rand();
        CATCH_REQUIRE(v->pwrite(buf, 4, 0, false) == 4);

        CATCH_REQUIRE(v->size() == sizeof(buf));
        CATCH_REQUIRE(v->count_buffers() == 1);  // overwrite does not add more buffers

        for(size_t i(0); i < sizeof(buf); ++i)
        {
            char c;
            CATCH_REQUIRE(v->pread(&c, sizeof(c), i, true) == sizeof(c));
            CATCH_REQUIRE(buf[i] == c);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("virtual_buffer: simple write + read + erase + read")
    {
        prinbee::virtual_buffer::pointer_t v(std::make_shared<prinbee::virtual_buffer>());
        CATCH_REQUIRE(v->size() == 0);
        CATCH_REQUIRE(v->count_buffers() == 0);

        constexpr std::size_t const buf_size(1024 * 8);
        char buf[buf_size];
        for(std::size_t i(0); i < sizeof(buf); ++i)
        {
            buf[i] = rand();
        }
        CATCH_REQUIRE(v->pwrite(buf, sizeof(buf), 0, true) == sizeof(buf));

        CATCH_REQUIRE(v->size() == sizeof(buf));
        CATCH_REQUIRE(v->count_buffers() == 1);  // one write means at most 1 buffer

        char saved[buf_size];
        CATCH_REQUIRE(v->pread(saved, sizeof(saved), 0, true) == sizeof(saved));

        CATCH_REQUIRE(sizeof(buf) == sizeof(saved));
        CATCH_REQUIRE(memcmp(buf, saved, sizeof(buf)) == 0);

        // erase 1024 bytes at offset 4096
        //
        CATCH_REQUIRE(v->perase(1024, 4096) == 1024);
        CATCH_REQUIRE(v->pread(saved, 4096, 0, true) == 4096);
        CATCH_REQUIRE(memcmp(buf, saved, 4096) == 0);
        CATCH_REQUIRE(v->pread(saved, 3072, 4096, true) == 3072);
        CATCH_REQUIRE(memcmp(buf + 4096 + 1024, saved, 3072) == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("virtual_buffer: many writes + read + (erase + read) * N")
    {
        prinbee::virtual_buffer::pointer_t v(std::make_shared<prinbee::virtual_buffer>());
        CATCH_REQUIRE(v->size() == 0);
        CATCH_REQUIRE(v->count_buffers() == 0);

        // create a buffer of many Kb (at least 150Kb up to 512Kb
        //
        std::uint64_t buf_size(0);
        for(;;)
        {
            SNAP_CATCH2_NAMESPACE::random(buf_size);
            buf_size %= 512 * 1024;
            ++buf_size;
            if(buf_size >= 150 * 1024)
            {
                break;
            }
        }
        prinbee::buffer_t buf;
        buf.reserve(buf_size);
        for(std::uint64_t i(0); i < buf_size; ++i)
        {
            buf.push_back(rand());
        }

        // write the buffer in small chunks so that way we get "many"
        // virtual buffers instead of one large one
        //
        std::uint64_t written(0);
        while(written < buf_size)
        {
            std::uint64_t sz(0);
            SNAP_CATCH2_NAMESPACE::random(sz);
            sz %= 1024;
            ++sz; // 1 to 1024
            sz = std::min(buf_size - written, sz);
            CATCH_REQUIRE(v->pwrite(buf.data() + written, sz, written, true) == static_cast<int>(sz));
            written += sz;
            CATCH_REQUIRE(v->size() == written);
        }

        CATCH_REQUIRE(v->size() == buf_size);
        CATCH_REQUIRE(v->count_buffers() > 1);

        // verify we can read the whole lot of data and it is equal to buffer
        //
        prinbee::buffer_t saved(buf_size);
        CATCH_REQUIRE(v->pread(saved.data(), buf_size, 0, true) == static_cast<int>(buf_size));
        CATCH_REQUIRE(buf == saved);

        // erase the whole buffer a little bit at a time and verify the
        // result each time
        //
        while(v->size() > 0)
        {
            std::uint64_t sz(0);
            SNAP_CATCH2_NAMESPACE::random(sz);
            sz %= 512;
            ++sz; // 1 to 512 bytes to delete
            sz = std::min(sz, v->size());

            std::uint64_t offset(0);
            if(v->size() > sz)
            {
                SNAP_CATCH2_NAMESPACE::random(offset);
                offset %= v->size() - sz;
            }

            if(sz + offset == v->size())
            {
                // a larger size at the end has no effect because we adjust
                // it to v->size() internally
                //
                std::uint64_t const extra(rand() % 4096 + 1);
SNAP_LOG_WARNING << "--- perase(" << sz << " + " << extra << " [" << sz + extra << "], " << offset << "); ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE(v->perase(sz + extra, offset) == static_cast<int>(sz));
            }
            else
            {
SNAP_LOG_WARNING << "--- perase(" << sz << ", " << offset << "); ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE(v->perase(sz, offset) == static_cast<int>(sz));
            }

            // also apply the erase to our local buffer
            //
            buf.erase(buf.begin() + offset, buf.begin() + offset + sz);

            // get a copy of the full buffer and compare, it must be 100% equal
            //
            prinbee::buffer_t latest(buf.size());
SNAP_LOG_WARNING << "--- read result (" << buf.size() << ") ..." << SNAP_LOG_SEND;
            CATCH_REQUIRE(v->pread(latest.data(), buf.size(), 0, true) == static_cast<int>(buf.size()));
            //CATCH_REQUIRE(buf == latest); -- output for this one is awful
            CATCH_REQUIRE_LARGE_BUFFER(buf.data(), buf.size(), latest.data(), latest.size());
        }
        //CATCH_REQUIRE(v->perase(1024, 4096) == 1024);
        //CATCH_REQUIRE(v->pread(saved, 4096, 0, true) == 4096);
        //CATCH_REQUIRE(memcmp(buf, saved, 4096) == 0);
        //CATCH_REQUIRE(v->pread(saved, 3072, 4096, true) == 3072);
        //CATCH_REQUIRE(memcmp(buf + 4096 + 1024, saved, 3072) == 0);
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
