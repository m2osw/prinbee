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


/** \file
 * \brief Streaming hash computation.
 *
 * The database library uses hashing for various reasons such as the
 * generation of a bloom filter or sorting large amount of items using
 * a map. This hash calculator can be used for that purpose.
 *
 * For primary keys, this library uses the murmur3 project to compute
 * a better distributed range of numbers.
 */

// self
//
#include    "prinbee/file/hash.h"



// snapdev
//
#include    <snapdev/not_reached.h>


// C++
//
#include    <iostream>


// C
//
#include    <string.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



/** \brief Init the hash with the specified seed.
 *
 * The hash number starts with the specified seed. By changing the seed
 * you can reuse the same class as if you were using several different
 * hash functions. This is how we create multiple hashes for bloom
 * filters. For maps, you may use zero as the seed which means the empty
 * key return 0 as its key.
 *
 * To get the current result, use the get() function. You can call the
 * get() function at any time and any number of time. It does not break
 * the processing of the streaming data and returns the expected result
 * as if the streamed data had been past all at once.
 *
 * \param[in] seed  The seed to start with.
 *
 * \sa get()
 */
hash::hash(hash_t seed)
    : f_hash(seed)
{
}


/** \brief Retrieve one byte.
 *
 * This function reads the next byte. If bytes were saved in the temporary
 * buffer, then it gets read from that buffer first. Once the temporary
 * buffer is empty, bytes get read from the input buffer as specified to
 * the add() function.
 *
 * To not eat a byte of data, one can call the peek_byte() instead.
 *
 * \warning
 * The function is expected to be called only when some data is available,
 * which means buffer_size() > 0 is true.
 *
 * \return The next byte of data.
 *
 * \sa add()
 * \sa peek_byte()
 * \sa get_64bits()
 * \sa buffer_size()
 */
hash_t hash::get_byte()
{
    if(f_temp_size > 0)
    {
        --f_temp_size;
        return f_temp[f_temp_size];
    }

    if(f_size > 0)
    {
        uint8_t const v(*f_buffer);
        ++f_buffer;
        --f_size;
        return v;
    }

    // this is an internal function which never gets called
    // if buffer_size() returns 0
    //
    snapdev::NOT_REACHED();     // LCOV_EXCL_LINE
}


/** \brief Read one byte without eating it.
 *
 * This function peeks at the byte at \p pos and returns it.
 *
 * The byte is read from the temporary buffer only. This is because the
 * function is expected to be called whenever you call the get() function
 * to retrieve the final hash result even if some temporary data is still
 * lingering in the hash object, but without disturbing that day in case
 * the user wants to add() more data to the hash.
 *
 * In other words, you can compute multiple hash for larger buffers.
 *
 * \param[in] pos  The position of the byte to retrieve.
 *
 * \return The byte at the specified \p pos position.
 */
hash_t hash::peek_byte(int pos) const
{
    if(static_cast<std::size_t>(pos) < f_temp_size)
    {
        // bytes are in reverse order in f_temp
        //
        return f_temp[f_temp_size - pos - 1];
    }

    // this is an internal function only used to get bytes that were not
    // yet used up by the get_byte() function. This means only the bytes
    // inside f_temp[] get used by the peek_byte() function
    //
    snapdev::NOT_REACHED();     // LCOV_EXCL_LINE
//    pos -= f_temp_size;
//
//    if(static_cast<std::size_t>(pos) < f_size)
//    {
//        return f_buffer[pos];
//    }
//
//    return 0;
}


/** \brief Get the current size of the buffers.
 *
 * This function computes the number of bytes currently available in the
 * input temporary and user specified buffers.
 *
 * \return Total size of data currently available.
 */
size_t hash::buffer_size() const
{
    return f_temp_size + f_size;
}


/** \brief Retrieve exactly 64 bits of data.
 *
 * This function retrieves 64 bits (8 bytes) of data and places the values
 * in \p v1 and \p v2. The function works in either big or little endian.
 *
 * \warning
 * This is an internal function and it is expected to be called only if
 * buffer_size() >= 8.
 *
 * \param[out] v1  The high value (as it would be in a 64 bit number in
 * big endian).
 * \param[out] v2  The low value (as it would be in a 64 bit number in
 * big endian).
 */
void hash::get_64bits(hash_t & v1, hash_t & v2)
{
    if(f_temp_size == 0 && f_size >= 8)
    {
        // faster this way
        //
        v1 = (f_buffer[0] << 24)
           + (f_buffer[1] << 16)
           + (f_buffer[2] <<  8)
           + (f_buffer[3] <<  0);

        v2 = (f_buffer[4] << 24)
           + (f_buffer[5] << 16)
           + (f_buffer[6] <<  8)
           + (f_buffer[7] <<  0);

        f_buffer += 8;
        f_size -= 8;

        return;
    }

    if(buffer_size() >= 8)
    {
        v1 = (get_byte() << 24)
           + (get_byte() << 16)
           + (get_byte() <<  8)
           + (get_byte() <<  0);

        v2 = (get_byte() << 24)
           + (get_byte() << 16)
           + (get_byte() <<  8)
           + (get_byte() <<  0);

        return;
    }

    // the function is not expected to be called when buffer_size() < 8
    //
    snapdev::NOT_REACHED();     // LCOV_EXCL_LINE
}


/** \brief Peek for up to 64 bits of data.
 *
 * When calling get(), the function attempts to peek at cached data in
 * the temporary buffer assuming buffer_size() > 0. This function is responsible
 * to peed at those bytes.
 *
 * \note
 * The original function here does not shift the low/high value
 * as if it were in big endian. Instead it reads the available bytes in
 * the low bytes of \p v1 and \p v2. So there is a form of mix between
 * big and little endian in this one function.
 *
 * \param[out] v1  The high value (as it would be in a 64 bit number in
 * big endian).
 * \param[out] v2  The low value (as it would be in a 64 bit number in
 * big endian).
 */
void hash::peek_64bits(hash_t & v1, hash_t & v2) const
{
    switch(buffer_size())
    {
    case 7:
        v1 = (peek_byte(0) << 24)
           + (peek_byte(1) << 16)
           + (peek_byte(2) <<  8)
           + (peek_byte(3) <<  0);

        v2 = (peek_byte(4) << 16)
           + (peek_byte(5) <<  8)
           + (peek_byte(6) <<  0);
        break;

    case 6:
        v1 = (peek_byte(0) << 24)
           + (peek_byte(1) << 16)
           + (peek_byte(2) <<  8)
           + (peek_byte(3) <<  0);

        v2 = (peek_byte(4) <<  8)
           + (peek_byte(5) <<  0);
        break;

    case 5:
        v1 = (peek_byte(0) << 24)
           + (peek_byte(1) << 16)
           + (peek_byte(2) <<  8)
           + (peek_byte(3) <<  0);

        v2 = (peek_byte(4) <<  0);
        break;

    case 4:
        v1 = (peek_byte(0) << 24)
           + (peek_byte(1) << 16)
           + (peek_byte(2) <<  8)
           + (peek_byte(3) <<  0);

        v2 = 0;
        break;

    case 3:
        v1 = (peek_byte(0) << 16)
           + (peek_byte(1) <<  8)
           + (peek_byte(2) <<  0);

        v2 = 0;
        break;

    case 2:
        v1 = (peek_byte(0) <<  8)
           + (peek_byte(1) <<  0);

        v2 = 0;
        break;

    case 1:
        v1 = (peek_byte(0) <<  0);

        v2 = 0;
        break;

    default:                            // LCOV_EXCL_LINE
        snapdev::NOT_REACHED();         // LCOV_EXCL_LINE

    }
}


/** \brief Add data to the hash.
 *
 * This function is called to add the data to the hash. If you already have
 * the entire buffer in memory, you can call the function just once like
 * so:
 *
 * \code
 *     hash h(seed);
 *     h.add(all, sizeof(all));
 *     hash_t const result(h.get());
 * \endcode
 *
 * If you are streaming the data, you can call the function for each
 * block of data you are streaming and one last time with the partial
 * (or complete) last block like so:
 *
 * \code
 *     hash h(seed);
 *     for(...)
 *     {
 *         h.add(block, sizeof(block));
 *     }
 *     h.add(last_block, sizeof(last_block));
 *     hash_t const result(h.get());
 * \endcode
 *
 * To retrieve the results, use the get() function.
 *
 * \note
 * The original hash function was taken from:
 * https://github.com/ArashPartow/bloom
 * and modified to work incrementally.
 *
 * \param[in] v  A pointer to a buffer of bytes.
 * \param[in] buffer_size  The number of butes in \p v.
 */
void hash::add(uint8_t const * v, std::size_t buffer_size)
{
    f_total_size += buffer_size;

    f_buffer = v;
    f_size = buffer_size;

    process_buffer();

#ifdef _DEBUG
    // additional safety measure to make sure it does not get re-used
    //
    f_buffer = nullptr;
#endif
}


/** \brief Function processing the user buffer.
 *
 * This function is the heart of this hash implementation. It reads 64 bits
 * of data, updates the hash as required, and repeat that process until the
 * input buffer has less than 8 bytes of data.
 *
 * If 1 to 7 bytes of data are still available in the input buffer, they
 * get saved in the temporary buffer for the next time the add() function
 * gets called.
 */
void hash::process_buffer()
{
    while(buffer_size() >= 8)
    {
        hash_t v1(0);
        hash_t v2(0);
        get_64bits(v1, v2);

        f_hash ^= (f_hash <<  7) ^ (v1 * (f_hash >> 3))
             ^ (~((f_hash << 11) + (v2 ^ (f_hash >> 5))));
    }

    if(f_temp_size > 0 && f_size > 0)
    {
        memmove(f_temp + f_size, f_temp, f_temp_size);
    }

    f_temp_size += f_size;
    for(int in(0); f_size > 0; ++in)
    {
        --f_size;
        f_temp[in] = f_buffer[f_size];
    }
}


/** \brief Get the hash as it currently stands.
 *
 * This function retrieves the hash computed so far. This value can be
 * retrieved at any time (i.e. if you want to get a hash at every 4Kb
 * of data, it is possible with this implementation).
 *
 * \note
 * The function may do further computation so it is a good idea to save
 * the value in a variable and avoid calling this function repeatitively.
 * To avoid further computations, make sure to add() buffers that add up
 * to a size which is an exact multiple of 8 bytes.
 *
 * \return The current hash.
 */
hash_t hash::get() const
{
    hash_t h(f_hash);

    std::size_t sz(buffer_size());
    if(sz > 0)
    {
        hash_t loop(0);
        hash_t v1;
        hash_t v2;
        peek_64bits(v1, v2);

        if(sz >= 4)
        {
            h ^= ~((h << 11) + (v1 ^ (h >> 5)));
            ++loop;

            sz -= 4;
            v1 = v2;
        }

        if(sz >= 3)
        {
            v2 = v1 >> 8;
            if(loop != 0)
            {
                h ^= (h <<  7) ^  v2 * (h >> 3);
            }
            else
            {
                h ^= ~((h << 11) + (v2 ^ (h >> 5)));
            }
            ++loop;

            sz = 1;
            v1 &= 255;
        }
        else if(sz == 2)
        {
            if(loop != 0)
            {
                h ^= (h <<  7) ^  v1 * (h >> 3);
            }
            else
            {
                h ^= ~((h << 11) + (v1 ^ (h >> 5)));
            }
            //++loop; -- not necessary, we won't reuse it

            sz = 0;
        }

        if(sz > 0)
        {
            h += (v1 ^ (h * 0xA5A5A5A5)) + loop;
        }
    }

    return h;
}


/** \brief Retrieve the number of bytes used to compute this hash.
 *
 * This function returns the total number of bytes that were used so
 * far to compute the hash. This is a pratical way to have the total
 * number of bytes of data you add()-ed to this hash object.
 *
 * \sa add()
 */
std::size_t hash::size() const
{
    return f_total_size;
}



} // namespace prinbee
// vim: ts=4 sw=4 et
