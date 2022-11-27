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

// self
//
#include    "catch_main.h"


// prinbee
//
#include    <prinbee/file/hash.h>


// advgetopt
//
#include    <advgetopt/options.h>



namespace
{



// hash function taken from: https://github.com/ArashPartow/bloom
//
prinbee::hash_t compute_hash(uint8_t const * v, std::size_t size, prinbee::hash_t const seed)
{
    prinbee::hash_t hash(seed);
    prinbee::hash_t loop(0);

    for(; size >= 8; v += 8, size -= 8)
    {
        prinbee::hash_t i1((v[0] << 24) + (v[1] << 16) + (v[2] << 8) + v[3]);
        prinbee::hash_t i2((v[4] << 24) + (v[5] << 16) + (v[6] << 8) + v[7]);

        hash ^= (hash <<  7) ^  i1 * (hash >> 3) ^
             (~((hash << 11) + (i2 ^ (hash >> 5))));
    }

    if(size >= 4)
    {
        prinbee::hash_t i((v[0] << 24) + (v[1] << 16) + (v[2] << 8) + v[3]);
        hash ^= (~((hash << 11) + (i ^ (hash >> 5))));
        ++loop;
        size -= 4;
        v += 4;
    }

    if(size >= 2)
    {
        prinbee::hash_t i((v[0] << 8) + v[1]);
        if(loop != 0)
        {
            hash ^=    (hash <<  7) ^  i * (hash >> 3);
        }
        else
        {
            hash ^= (~((hash << 11) + (i ^ (hash >> 5))));
        }
        ++loop;
        size -= 2;
        v += 2;
    }

    if(size > 0)
    {
        hash += (v[0] ^ (hash * 0xA5A5A5A5)) + loop;
    }

    return hash;
}



}
// no name namespace


CATCH_TEST_CASE("hash", "[hash] [valid]")
{
    CATCH_START_SECTION("hash")
    {
        for(int count(0); count < 100; ++count)
        {
            std::size_t const size(rand() % 65536 + 32768);

            std::vector<std::uint8_t> buffer(size);
            for(std::size_t idx(0); idx < size; ++idx)
            {
                buffer[idx] = rand();
            }

            // try hash entire buffer at once
            {
                prinbee::hash_t const seed(rand());
                prinbee::hash_t const expected(compute_hash(buffer.data(), size, seed));
                prinbee::hash h(seed);
                h.add(buffer.data(), size);
                CATCH_REQUIRE(h.get() == expected);
            }

            // try a little at a time
            {
                prinbee::hash_t const seed(rand());
                prinbee::hash h(seed);
                CATCH_REQUIRE(h.size() == 0);
                std::size_t processed(0);
                while(processed < size)
                {
                    std::size_t incr(rand() % 256 + 1);
                    if(processed + incr > size)
                    {
                        incr = size - processed;
                    }
                    h.add(buffer.data() + processed, incr);
                    processed += incr;
                    CATCH_REQUIRE(h.size() == processed);
                    prinbee::hash_t const expected(compute_hash(buffer.data(), processed, seed));

                    prinbee::hash once(seed);
                    once.add(buffer.data(), processed);
                    CATCH_REQUIRE(once.get() == expected);

                    CATCH_REQUIRE(h.get() == expected);
                }
            }

//hash_t compute_hash(uint8_t const * v, std::size_t size, hash_t hash)
//
//        hash(hash_t seed);
//void    add(std::uint8_t const * v, std::size_t size);
//hash_t  get() const;

        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
