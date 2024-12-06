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
 * \brief Database file implementation.
 *
 * Each table uses one or more files. Each file is handled by a dbfile
 * object and a corresponding set of blocks.
 */

// self
//
#include    "prinbee/bigint/add_sub.h"
#include    "prinbee/bigint/uint512.h"

#include    "prinbee/exception.h"


// C++
//
#include    <cstring>
#include    <iomanip>
#include    <iostream>
#include    <string>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{





int512_t::int512_t()
{
}


int512_t::int512_t(int512_t const & rhs)
{
    f_value[0] = rhs.f_value[0];
    f_value[1] = rhs.f_value[1];
    f_value[2] = rhs.f_value[2];
    f_value[3] = rhs.f_value[3];
    f_value[4] = rhs.f_value[4];
    f_value[5] = rhs.f_value[5];
    f_value[6] = rhs.f_value[6];
    f_high_value = rhs.f_high_value;
}


int512_t::int512_t(uint512_t const & rhs)
{
    f_value[0] = rhs.f_value[0];
    f_value[1] = rhs.f_value[1];
    f_value[2] = rhs.f_value[2];
    f_value[3] = rhs.f_value[3];
    f_value[4] = rhs.f_value[4];
    f_value[5] = rhs.f_value[5];
    f_value[6] = rhs.f_value[6];
    f_high_value = static_cast<std::int64_t>(rhs.f_value[7]);
}


int512_t::int512_t(std::initializer_list<std::uint64_t> rhs)
{
    if(rhs.size() > std::size(f_value) + 1)
    {
        throw out_of_range(
                  "rhs array too large for int512_t constructor ("
                + std::to_string(rhs.size())
                + " > "
                + std::to_string(std::size(f_value) + 1)
                + ").");
    }

    std::copy(rhs.begin(), rhs.end(), f_value);
    if(rhs.size() < std::size(f_value))
    {
        std::memset(f_value + rhs.size(), 0, (std::size(f_value) - rhs.size()) * sizeof(f_value[0]));
    }
}


int512_t::int512_t(std::int64_t rhs)
{
    f_value[0] = static_cast<std::uint64_t>(rhs);

    // sign extend rhs to the other entries
    //
    f_value[1] =
    f_value[2] =
    f_value[3] =
    f_value[4] =
    f_value[5] =
    f_value[6] = rhs < 0 ? 0xFFFFFFFFFFFFFFFFULL : 0ULL;
    f_high_value = rhs < 0 ? -1LL : 0LL;
}


int512_t & int512_t::zero()
{
    f_value[0] = 0;
    f_value[1] = 0;
    f_value[2] = 0;
    f_value[3] = 0;
    f_value[4] = 0;
    f_value[5] = 0;
    f_value[6] = 0;
    f_high_value = 0;
    return *this;
}


bool int512_t::is_zero() const
{
    return f_value[0] == 0
        && f_value[1] == 0
        && f_value[2] == 0
        && f_value[3] == 0
        && f_value[4] == 0
        && f_value[5] == 0
        && f_value[6] == 0
        && f_high_value == 0;
}


int int512_t::compare(int512_t const & rhs) const
{
    if(f_high_value != rhs.f_high_value)
    {
        return f_high_value > rhs.f_high_value ? 1 : -1;
    }

    for(int idx(6); idx >= 0; --idx)
    {
        if(f_value[idx] != rhs.f_value[idx])
        {
            return f_value[idx] > rhs.f_value[idx] ? 1 : -1;
        }
    }

    return 0;
}


std::size_t int512_t::bit_size() const
{
    int512_t p;
    if(is_negative())
    {
        p -= *this;
        if(p.is_negative())
        {
            return 512;
        }
    }
    else
    {
        p = *this;
    }

    if(p.f_high_value != 0)
    {
        return (64 - __builtin_clzll(p.f_high_value)) + 7 * 64;
    }

    std::size_t result(512 - 63);
    for(int idx(6); idx >= 0; --idx)
    {
        result -= 64;
        if(p.f_value[idx] != 0)
        {
            return (__builtin_clzll(p.f_value[idx]) ^ 0x3F) + result;
        }
    }

    return 0;
}


void int512_t::lsl(int count)
{
    if(count < 0)
    {
        throw out_of_range(
                  "lsl() cannot be called with a negative value ("
                + std::to_string(count)
                + ").");
    }

    // note: with a processor, the count is truncated to the total number of
    //       bits available -- wondering we should be doing the same here
    //
    if(count >= 512)
    {
        zero();
        return;
    }

    if(count == 0)
    {
        // nothing to do
        //
        return;
    }

    int move(count / 64);
    int const shift(count % 64);
    if(move > 0)
    {
        // WARNING: the following moves the f_high_value as well
        //
        int const pos(8 - move);
        if(move < 8)
        {
            memmove(f_value + move, f_value, pos * 8);
        }
        memset(f_value, 0, move * 8);
    }
    if(shift != 0)
    {
        int remainder(64 - shift);

        std::uint64_t extra(0);
        while(move < 7)
        {
            std::uint64_t const next(f_value[move] >> remainder);
            f_value[move] = (f_value[move] << shift) | extra;
            extra = next;
            ++move;
        }
        f_high_value = (f_high_value << shift) | extra;
    }
}


void int512_t::asr(int count)
{
    if(count < 0)
    {
        throw out_of_range(
                  "asr() cannot be called with a negative value ("
                + std::to_string(count)
                + ").");
    }

    // note: with a processor, the count is truncated to the total number of
    //       bits available -- wondering we should be doing the same here
    //
    bool const negative(is_negative());
    if(count >= 512)
    {
        if(negative)
        {
            memset(f_value, -1, sizeof(f_value));
            f_high_value = -1;
        }
        else
        {
            zero();
        }
        return;
    }

    if(count == 0)
    {
        // nothing to do
        //
        return;
    }

    int const move(count / 64);
    int const shift(count % 64);
    int pos(8 - move);
    if(move > 0)
    {
        // WARNING: the following moves the f_high_value as well
        //
        if(move < 8)
        {
            memmove(f_value, f_value + move, pos * 8);
        }
        if(negative)
        {
            memset(f_value + pos, -1, move * 8);
        }
        else
        {
            memset(f_value + pos, 0, move * 8);
        }
    }
    if(shift != 0)
    {
        int remainder(64 - shift);

        std::uint64_t extra(0);
        if(pos == 7)
        {
            --pos;
            extra = f_high_value << remainder;
            f_high_value = f_high_value >> shift;
        }
        while(pos > 0)
        {
            --pos;
            std::uint64_t const next(f_value[pos] << remainder);
            f_value[pos] = (f_value[pos] >> shift) | extra;
            extra = next;
        }
    }
}


int512_t int512_t::operator - () const
{
    int512_t neg;
    neg -= *this;
    return neg;
}


int512_t & int512_t::operator += (int512_t const & rhs)
{
    add512(f_value, rhs.f_value);       // the add includes the high value
    return *this;
}


int512_t & int512_t::operator += (std::int64_t rhs)
{
    int512_t const b(rhs);
    return *this += b;
}


int512_t & int512_t::operator -= (int512_t const & rhs)
{
    sub512(f_value, rhs.f_value);       // the sub includes the high value
    return *this;
}


int512_t & int512_t::operator -= (std::int64_t rhs)
{
    int512_t const b(rhs);
    return *this -= b;
}


int512_t & int512_t::operator <<= (int shift)
{
    lsl(shift);
    return *this;
}


int512_t int512_t::operator << (int shift) const
{
    int512_t result(*this);
    result.lsl(shift);
    return result;
}


int512_t & int512_t::operator >>= (int shift)
{
    asr(shift);
    return *this;
}


int512_t int512_t::operator >> (int shift) const
{
    int512_t result(*this);
    result.asr(shift);
    return result;
}


int512_t int512_t::operator & (int512_t const & rhs) const
{
    int512_t r(*this);
    return r &= rhs;
}


int512_t int512_t::operator & (std::int64_t rhs) const
{
    int512_t r(*this);
    return r &= rhs;
}


int512_t & int512_t::operator &= (int512_t const & rhs)
{
    for(int i(0); i < 7; ++i)
    {
        f_value[i] &= rhs.f_value[i];
    }
    f_high_value &= rhs.f_high_value;
    return *this;
}


int512_t int512_t::operator | (int512_t const & rhs) const
{
    int512_t r(*this);
    return r |= rhs;
}


int512_t int512_t::operator | (std::int64_t rhs) const
{
    int512_t r(*this);
    return r |= rhs;
}


int512_t & int512_t::operator |= (int512_t const & rhs)
{
    for(int i(0); i < 7; ++i)
    {
        f_value[i] |= rhs.f_value[i];
    }
    f_high_value |= rhs.f_high_value;
    return *this;
}


int512_t int512_t::operator ^ (int512_t const & rhs) const
{
    int512_t r(*this);
    return r |= rhs;
}


int512_t int512_t::operator ^ (std::int64_t rhs) const
{
    int512_t r(*this);
    return r |= rhs;
}


int512_t & int512_t::operator ^= (int512_t const & rhs)
{
    for(int i(0); i < 7; ++i)
    {
        f_value[i] ^= rhs.f_value[i];
    }
    f_high_value ^= rhs.f_high_value;
    return *this;
}


int512_t & int512_t::operator ++ ()
{
    return *this += 1;
}


int512_t & int512_t::operator -- ()
{
    return *this -= 1;
}


bool int512_t::operator == (int512_t const & rhs) const
{
    return f_value[0] == rhs.f_value[0]
        && f_value[1] == rhs.f_value[1]
        && f_value[2] == rhs.f_value[2]
        && f_value[3] == rhs.f_value[3]
        && f_value[4] == rhs.f_value[4]
        && f_value[5] == rhs.f_value[5]
        && f_value[6] == rhs.f_value[6]
        && f_high_value == rhs.f_high_value;
}


bool int512_t::operator == (std::int64_t rhs) const
{
    if(rhs < 0)
    {
        return f_value[0] == static_cast<std::uint64_t>(rhs)
            && f_value[1] == static_cast<std::uint64_t>(-1)
            && f_value[2] == static_cast<std::uint64_t>(-1)
            && f_value[3] == static_cast<std::uint64_t>(-1)
            && f_value[4] == static_cast<std::uint64_t>(-1)
            && f_value[5] == static_cast<std::uint64_t>(-1)
            && f_value[6] == static_cast<std::uint64_t>(-1)
            && f_high_value == -1;
    }
    else
    {
        return f_value[0] == static_cast<std::uint64_t>(rhs)
            && f_value[1] == 0
            && f_value[2] == 0
            && f_value[3] == 0
            && f_value[4] == 0
            && f_value[5] == 0
            && f_value[6] == 0
            && f_high_value == 0;
    }
}


bool int512_t::operator != (int512_t const & rhs) const
{
    return f_value[0] != rhs.f_value[0]
        || f_value[1] != rhs.f_value[1]
        || f_value[2] != rhs.f_value[2]
        || f_value[3] != rhs.f_value[3]
        || f_value[4] != rhs.f_value[4]
        || f_value[5] != rhs.f_value[5]
        || f_value[6] != rhs.f_value[6]
        || f_high_value != rhs.f_high_value;
}


bool int512_t::operator != (std::int64_t rhs) const
{
    if(rhs < 0)
    {
        return f_value[0] != static_cast<std::uint64_t>(rhs)
            || f_value[1] != static_cast<std::uint64_t>(-1)
            || f_value[2] != static_cast<std::uint64_t>(-1)
            || f_value[3] != static_cast<std::uint64_t>(-1)
            || f_value[4] != static_cast<std::uint64_t>(-1)
            || f_value[5] != static_cast<std::uint64_t>(-1)
            || f_value[6] != static_cast<std::uint64_t>(-1)
            || f_high_value != -1;
    }
    else
    {
        return f_value[0] != static_cast<std::uint64_t>(rhs)
            || f_value[1] != 0
            || f_value[2] != 0
            || f_value[3] != 0
            || f_value[4] != 0
            || f_value[5] != 0
            || f_value[6] != 0
            || f_high_value != 0;
    }
}


bool int512_t::operator < (int512_t const & rhs) const
{
    return compare(rhs) < 0;
}


bool int512_t::operator < (std::int64_t rhs) const
{
    if(rhs < 0)
    {
        if(is_positive())
        {
            return false;
        }
        if(f_value[1] != 0xFFFFFFFFFFFFFFFFULL
        || f_value[2] != 0xFFFFFFFFFFFFFFFFULL
        || f_value[3] != 0xFFFFFFFFFFFFFFFFULL
        || f_value[4] != 0xFFFFFFFFFFFFFFFFULL
        || f_value[5] != 0xFFFFFFFFFFFFFFFFULL
        || f_value[6] != 0xFFFFFFFFFFFFFFFFULL
        || f_high_value != -1)
        {
            return true;
        }
        return f_value[0] < static_cast<std::uint64_t>(rhs);
    }
    else
    {
        if(is_negative())
        {
            return true;
        }

        if(f_value[1] != 0
        || f_value[2] != 0
        || f_value[3] != 0
        || f_value[4] != 0
        || f_value[5] != 0
        || f_value[6] != 0
        || f_high_value != 0)
        {
            return false;
        }
        return f_value[0] < static_cast<std::uint64_t>(rhs);
    }
}


bool int512_t::operator <= (int512_t const & rhs) const
{
    return compare(rhs) <= 0;
}


bool int512_t::operator > (int512_t const & rhs) const
{
    return compare(rhs) > 0;
}


bool int512_t::operator >= (int512_t const & rhs) const
{
    return compare(rhs) >= 0;
}


std::string int512_t::to_string(int base, bool introducer, bool uppercase) const
{
    uint512_t v(*this);
    std::string result;
    if(is_negative())
    {
        result += '-';
        v = -v;
    }
    result += v.to_string(base, introducer, uppercase);
    return result;
} // LCOV_EXCL_LINE


std::string to_string(int512_t const & v)
{
    return v.to_string();
}



} // namespace prinbee
// vim: ts=4 sw=4 et
