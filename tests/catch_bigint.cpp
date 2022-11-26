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

#include    "num.hpp"


// prinbee
//
#include    <prinbee/bigint/bigint.h>
#include    <prinbee/exception.h>


// advgetopt
//
#include    <advgetopt/options.h>



namespace
{





}
// no name namespace


CATCH_TEST_CASE("bigint", "[bigint] [valid]")
{
    CATCH_START_SECTION("bigint zero()")
    {
        for(int count(0); count < 10; ++count)
        {
            prinbee::uint512_t a;
            for(int n(0); n < 10; ++n)
            {
                for(std::size_t i(0); i < 8; ++i)
                {
                    a.f_value[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
                }
                CATCH_REQUIRE(a.zero().is_zero());
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bigint bit_size & lsr")
    {
        for(int count(0); count < 10; ++count)
        {
            prinbee::uint512_t a;
            prinbee::uint512_t b;
            for(int n(0); n < 10; ++n)
            {
                for(std::size_t i(0); i < 8; ++i)
                {
                    a.f_value[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
                    b.f_value[i] = 0;
                }

                prinbee::uint512_t copy(a);
                copy.lsr(0);
                CATCH_REQUIRE(a == copy);
                copy.lsl(0);
                CATCH_REQUIRE(a == copy);

                a.f_value[7] |= 1ULL << 63;
                b.f_value[0] |= 1ULL;

                CATCH_REQUIRE(a != b);

                // compute shifts at once and verify in the loop below
                //
                prinbee::uint512_t r_shifted[512];
                prinbee::uint512_t l_shifted[512];
                for(int size(512); size > 0; --size)
                {
                    r_shifted[size - 1] = a;
                    r_shifted[size - 1].lsr(512 - size + 1);

                    l_shifted[size - 1] = b;
                    l_shifted[size - 1].lsl(512 - size + 1);
                }

                for(int size(512); size > 0; --size)
                {
                    CATCH_REQUIRE(a.bit_size() == static_cast<std::size_t>(size));
                    CATCH_REQUIRE(b.bit_size() == static_cast<std::size_t>(512 - size + 1));

                    if(size == 512)
                    {
                        // we use -a in this case so the size is ??? from 'a'
                        // so I check with b which has a known size
                        //
                        prinbee::int512_t c(b);
                        CATCH_REQUIRE(c.bit_size() == static_cast<std::size_t>(1));
                        CATCH_REQUIRE(c.abs() == c);
                        CATCH_REQUIRE(c == 1LL);
                        CATCH_REQUIRE_FALSE(c == 2LL);
                        CATCH_REQUIRE(c != 2LL);

                        c = -c;
                        CATCH_REQUIRE(c.bit_size() == static_cast<std::size_t>(1));
                        CATCH_REQUIRE(c.abs() == -c);
                        CATCH_REQUIRE(c == -1LL);
                        CATCH_REQUIRE_FALSE(c != -1LL);

                        // at this stage a and b are still not possibly equal
                        //
                        prinbee::int512_t d(a);
                        CATCH_REQUIRE_FALSE(c == d);
                        CATCH_REQUIRE(c != d);

                        c = b;
                        c.f_high_value = 1;
                        CATCH_REQUIRE_FALSE(c == 1LL);
                        CATCH_REQUIRE(c != 1LL);

                        c = -c;
                        CATCH_REQUIRE_FALSE(c == -1LL);
                        CATCH_REQUIRE(c != -1LL);
                    }
                    else
                    {
                        prinbee::int512_t c(a);
                        CATCH_REQUIRE(c.bit_size() == static_cast<std::size_t>(size));

                        if(size > 256)
                        {
                            prinbee::int512_t d(b);
                            CATCH_REQUIRE(c > d);
                            CATCH_REQUIRE(c >= d);
                            CATCH_REQUIRE(c >= c);
                            CATCH_REQUIRE_FALSE(c < d);
                            CATCH_REQUIRE_FALSE(c <= d);
                            CATCH_REQUIRE(c >= c);
                        }

                        {
                            prinbee::int512_t d(a);
                            CATCH_REQUIRE(c == d);
                            CATCH_REQUIRE_FALSE(c != d);
                            ++d.f_high_value;
                            CATCH_REQUIRE_FALSE(c == d);
                            CATCH_REQUIRE(c != d);
                        }

                        if(size == 1)
                        {
                            // in this case b is 1 << 511 which represents a
                            // negative number "which remains negative" and
                            // that's treated as a special case
                            //
                            prinbee::int512_t neg(b);
                            CATCH_REQUIRE_FALSE(neg.is_positive());
                            CATCH_REQUIRE(neg.is_negative());
                            CATCH_REQUIRE(neg.bit_size() == 512ULL);
                            CATCH_REQUIRE(neg != 1LL);
                            CATCH_REQUIRE(neg != -1LL);

                            // there is no valid representation of the
                            // absolute value in this case...
                            //
                            CATCH_REQUIRE(neg.abs().is_negative());
                        }
                        else
                        {
                            prinbee::int512_t pos(b);
                            CATCH_REQUIRE(pos.is_positive());
                            CATCH_REQUIRE_FALSE(pos.is_negative());
                        }
                    }

                    a.lsr(1);
                    b.lsl(1);

                    CATCH_REQUIRE(a == r_shifted[size - 1]);
                    CATCH_REQUIRE(b == l_shifted[size - 1]);
                }

                CATCH_REQUIRE(a.is_zero());
                CATCH_REQUIRE(a.bit_size() == 0);

                CATCH_REQUIRE(b.is_zero());
                CATCH_REQUIRE(b.bit_size() == 0);

                {
                    prinbee::int512_t c(a);
                    CATCH_REQUIRE(c.bit_size() == 0);
                }
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bigint large shifts")
    {
        prinbee::uint512_t a;
        prinbee::uint512_t b;
        for(int n(512); n < 520; ++n)
        {
            for(std::size_t i(0); i < 8; ++i)
            {
                a.f_value[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
                b.f_value[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
            }

            a.lsr(n);
            CATCH_REQUIRE(a.is_zero());

            b.lsl(n);
            CATCH_REQUIRE(b.is_zero());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bigint copying")
    {
        for(int count(0); count < 10; ++count)
        {
            prinbee::uint512_t a;
            prinbee::int512_t b;
            for(int n(0); n < 10; ++n)
            {
                for(std::size_t i(0); i < 8; ++i)
                {
                    a.f_value[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
                    b.f_value[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
                }

                prinbee::uint512_t a1(a);
                CATCH_REQUIRE(a.f_value[0] == a1.f_value[0]);
                CATCH_REQUIRE(a.f_value[1] == a1.f_value[1]);
                CATCH_REQUIRE(a.f_value[2] == a1.f_value[2]);
                CATCH_REQUIRE(a.f_value[3] == a1.f_value[3]);
                CATCH_REQUIRE(a.f_value[4] == a1.f_value[4]);
                CATCH_REQUIRE(a.f_value[5] == a1.f_value[5]);
                CATCH_REQUIRE(a.f_value[6] == a1.f_value[6]);
                CATCH_REQUIRE(a.f_value[7] == a1.f_value[7]);

                CATCH_REQUIRE(a >= a1);
                CATCH_REQUIRE_FALSE(a > a1);
                CATCH_REQUIRE(a <= a1);
                CATCH_REQUIRE_FALSE(a < a1);

                prinbee::int512_t a2(a);
                CATCH_REQUIRE(a.f_value[0] == a2.f_value[0]);
                CATCH_REQUIRE(a.f_value[1] == a2.f_value[1]);
                CATCH_REQUIRE(a.f_value[2] == a2.f_value[2]);
                CATCH_REQUIRE(a.f_value[3] == a2.f_value[3]);
                CATCH_REQUIRE(a.f_value[4] == a2.f_value[4]);
                CATCH_REQUIRE(a.f_value[5] == a2.f_value[5]);
                CATCH_REQUIRE(a.f_value[6] == a2.f_value[6]);
                CATCH_REQUIRE(a.f_value[7] == static_cast<std::uint64_t>(a2.f_high_value));

                prinbee::uint512_t a3({
                        a.f_value[0],
                        a.f_value[1],
                        a.f_value[2],
                        a.f_value[3],
                        a.f_value[4],
                        a.f_value[5],
                        a.f_value[6],
                        a.f_value[7],
                    });
                CATCH_REQUIRE(a.f_value[0] == a3.f_value[0]);
                CATCH_REQUIRE(a.f_value[1] == a3.f_value[1]);
                CATCH_REQUIRE(a.f_value[2] == a3.f_value[2]);
                CATCH_REQUIRE(a.f_value[3] == a3.f_value[3]);
                CATCH_REQUIRE(a.f_value[4] == a3.f_value[4]);
                CATCH_REQUIRE(a.f_value[5] == a3.f_value[5]);
                CATCH_REQUIRE(a.f_value[6] == a3.f_value[6]);
                CATCH_REQUIRE(a.f_value[7] == a3.f_value[7]);

                prinbee::uint512_t a4({
                        a.f_value[4],
                        a.f_value[5],
                        a.f_value[6],
                        a.f_value[7],
                    });
                CATCH_REQUIRE(a.f_value[4] == a4.f_value[0]);
                CATCH_REQUIRE(a.f_value[5] == a4.f_value[1]);
                CATCH_REQUIRE(a.f_value[6] == a4.f_value[2]);
                CATCH_REQUIRE(a.f_value[7] == a4.f_value[3]);
                CATCH_REQUIRE(0 == a4.f_value[4]);
                CATCH_REQUIRE(0 == a4.f_value[5]);
                CATCH_REQUIRE(0 == a4.f_value[6]);
                CATCH_REQUIRE(0 == a4.f_value[7]);

                prinbee::uint512_t a5;
                a5 = a;
                CATCH_REQUIRE(a.f_value[0] == a5.f_value[0]);
                CATCH_REQUIRE(a.f_value[1] == a5.f_value[1]);
                CATCH_REQUIRE(a.f_value[2] == a5.f_value[2]);
                CATCH_REQUIRE(a.f_value[3] == a5.f_value[3]);
                CATCH_REQUIRE(a.f_value[4] == a5.f_value[4]);
                CATCH_REQUIRE(a.f_value[5] == a5.f_value[5]);
                CATCH_REQUIRE(a.f_value[6] == a5.f_value[6]);
                CATCH_REQUIRE(a.f_value[7] == a5.f_value[7]);

                prinbee::uint512_t a6;
                a6 = b;
                CATCH_REQUIRE(b.f_value[0] == a6.f_value[0]);
                CATCH_REQUIRE(b.f_value[1] == a6.f_value[1]);
                CATCH_REQUIRE(b.f_value[2] == a6.f_value[2]);
                CATCH_REQUIRE(b.f_value[3] == a6.f_value[3]);
                CATCH_REQUIRE(b.f_value[4] == a6.f_value[4]);
                CATCH_REQUIRE(b.f_value[5] == a6.f_value[5]);
                CATCH_REQUIRE(b.f_value[6] == a6.f_value[6]);
                CATCH_REQUIRE(static_cast<std::uint64_t>(b.f_high_value) == a6.f_value[7]);

                prinbee::uint512_t b1(b);
                CATCH_REQUIRE(b.f_value[0] == b1.f_value[0]);
                CATCH_REQUIRE(b.f_value[1] == b1.f_value[1]);
                CATCH_REQUIRE(b.f_value[2] == b1.f_value[2]);
                CATCH_REQUIRE(b.f_value[3] == b1.f_value[3]);
                CATCH_REQUIRE(b.f_value[4] == b1.f_value[4]);
                CATCH_REQUIRE(b.f_value[5] == b1.f_value[5]);
                CATCH_REQUIRE(b.f_value[6] == b1.f_value[6]);
                CATCH_REQUIRE(static_cast<std::uint64_t>(b.f_high_value) == b1.f_value[7]);

                prinbee::int512_t b2(b);
                CATCH_REQUIRE(b.f_value[0] == b2.f_value[0]);
                CATCH_REQUIRE(b.f_value[1] == b2.f_value[1]);
                CATCH_REQUIRE(b.f_value[2] == b2.f_value[2]);
                CATCH_REQUIRE(b.f_value[3] == b2.f_value[3]);
                CATCH_REQUIRE(b.f_value[4] == b2.f_value[4]);
                CATCH_REQUIRE(b.f_value[5] == b2.f_value[5]);
                CATCH_REQUIRE(b.f_value[6] == b2.f_value[6]);
                CATCH_REQUIRE(b.f_high_value == b2.f_high_value);

                prinbee::uint512_t b3({
                        b.f_value[0],
                        b.f_value[1],
                        b.f_value[2],
                        b.f_value[3],
                        b.f_value[4],
                        b.f_value[5],
                        b.f_value[6],
                        static_cast<std::uint64_t>(b.f_high_value),
                    });
                CATCH_REQUIRE(b.f_value[0] == b3.f_value[0]);
                CATCH_REQUIRE(b.f_value[1] == b3.f_value[1]);
                CATCH_REQUIRE(b.f_value[2] == b3.f_value[2]);
                CATCH_REQUIRE(b.f_value[3] == b3.f_value[3]);
                CATCH_REQUIRE(b.f_value[4] == b3.f_value[4]);
                CATCH_REQUIRE(b.f_value[5] == b3.f_value[5]);
                CATCH_REQUIRE(b.f_value[6] == b3.f_value[6]);
                CATCH_REQUIRE(b.f_high_value == static_cast<std::int64_t>(b3.f_value[7]));

                prinbee::int512_t b4({
                        b.f_value[4],
                        b.f_value[5],
                        b.f_value[6],
                        static_cast<std::uint64_t>(b.f_high_value),
                    });
                CATCH_REQUIRE(b.f_value[4] == b4.f_value[0]);
                CATCH_REQUIRE(b.f_value[5] == b4.f_value[1]);
                CATCH_REQUIRE(b.f_value[6] == b4.f_value[2]);
                CATCH_REQUIRE(static_cast<std::uint64_t>(b.f_high_value) == b4.f_value[3]);
                CATCH_REQUIRE(0 == b4.f_value[4]);
                CATCH_REQUIRE(0 == b4.f_value[5]);
                CATCH_REQUIRE(0 == b4.f_value[6]);
                CATCH_REQUIRE(0 == b4.f_high_value);

                prinbee::int512_t b5;
                b5 = b;
                CATCH_REQUIRE(b.f_value[0] == b5.f_value[0]);
                CATCH_REQUIRE(b.f_value[1] == b5.f_value[1]);
                CATCH_REQUIRE(b.f_value[2] == b5.f_value[2]);
                CATCH_REQUIRE(b.f_value[3] == b5.f_value[3]);
                CATCH_REQUIRE(b.f_value[4] == b5.f_value[4]);
                CATCH_REQUIRE(b.f_value[5] == b5.f_value[5]);
                CATCH_REQUIRE(b.f_value[6] == b5.f_value[6]);
                CATCH_REQUIRE(b.f_high_value == b5.f_high_value);

                prinbee::int512_t b6;
                b6 = a;
                CATCH_REQUIRE(a.f_value[0] == b6.f_value[0]);
                CATCH_REQUIRE(a.f_value[1] == b6.f_value[1]);
                CATCH_REQUIRE(a.f_value[2] == b6.f_value[2]);
                CATCH_REQUIRE(a.f_value[3] == b6.f_value[3]);
                CATCH_REQUIRE(a.f_value[4] == b6.f_value[4]);
                CATCH_REQUIRE(a.f_value[5] == b6.f_value[5]);
                CATCH_REQUIRE(a.f_value[6] == b6.f_value[6]);
                CATCH_REQUIRE(a.f_value[7] == static_cast<std::uint64_t>(b6.f_high_value));

                prinbee::uint512_t diff;
                int overflow(prinbee::sub(diff.f_value, a.f_value, b3.f_value, 8));
                if(overflow == 0)
                {
                    // no overflow means a >= b3
                    //
                    CATCH_REQUIRE(a >= b3);

                    overflow = prinbee::sub(diff.f_value, b3.f_value, a.f_value, 8);
                    if(overflow == 1)
                    {
                        // overflow the other way, then it's not equal so a > b3
                        //
                        CATCH_REQUIRE(a > b3);
                    }
                }
                else
                {
                    // overflow means a < b3
                    //
                    CATCH_REQUIRE(a < b3);
                }
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bigint additions")
    {
        for(int count(0); count < 10; ++count)
        {
            std::size_t const size(rand() % 128 + 16);
            std::vector<std::uint64_t> a(size);
            std::vector<std::uint64_t> b(size);
            std::vector<std::uint64_t> c(size);
            std::vector<std::uint64_t> d(size);

            for(int n(0); n < 10; ++n)
            {
                int carry(0);
                for(std::size_t i(0); i < size; ++i)
                {
                    a[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
                    b[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());

                    // "manually" compute the sum
                    c[i] = a[i] + b[i] + carry;
                    carry = c[i] < a[i] || c[i] < b[i] ? 1 : 0;
                }

                // very large number addition
                //
                int const overflow(prinbee::add(d.data(), a.data(), b.data(), size));

                CATCH_REQUIRE(overflow == carry);
                for(std::size_t i(0); i < size; ++i)
                {
                    CATCH_REQUIRE(d[i] == c[i]);
                }

                // 128 bits addition
                //
                d = a;
                prinbee::add128(d.data(), b.data());
                CATCH_REQUIRE(d[0] == c[0]);
                CATCH_REQUIRE(d[1] == c[1]);

                // 256 bits addition
                //
                d = a;
                prinbee::add256(d.data(), b.data());
                CATCH_REQUIRE(d[0] == c[0]);
                CATCH_REQUIRE(d[1] == c[1]);
                CATCH_REQUIRE(d[2] == c[2]);
                CATCH_REQUIRE(d[3] == c[3]);

                // 512 bits addition
                //
                d = a;
                prinbee::add512(d.data(), b.data());
                CATCH_REQUIRE(d[0] == c[0]);
                CATCH_REQUIRE(d[1] == c[1]);
                CATCH_REQUIRE(d[2] == c[2]);
                CATCH_REQUIRE(d[3] == c[3]);
                CATCH_REQUIRE(d[4] == c[4]);
                CATCH_REQUIRE(d[5] == c[5]);
                CATCH_REQUIRE(d[6] == c[6]);
                CATCH_REQUIRE(d[7] == c[7]);

                prinbee::uint512_t ai;
                ai.f_value[0] = a[0];
                ai.f_value[1] = a[1];
                ai.f_value[2] = a[2];
                ai.f_value[3] = a[3];
                ai.f_value[4] = a[4];
                ai.f_value[5] = a[5];
                ai.f_value[6] = a[6];
                ai.f_value[7] = a[7];

                prinbee::uint512_t bi;
                bi.f_value[0] = b[0];
                bi.f_value[1] = b[1];
                bi.f_value[2] = b[2];
                bi.f_value[3] = b[3];
                bi.f_value[4] = b[4];
                bi.f_value[5] = b[5];
                bi.f_value[6] = b[6];
                bi.f_value[7] = b[7];

                prinbee::int512_t as(ai);
                prinbee::int512_t bs(bi);

                // operator + ()
                prinbee::uint512_t di(ai + bi);
                CATCH_REQUIRE(c[0] == di.f_value[0]);
                CATCH_REQUIRE(c[1] == di.f_value[1]);
                CATCH_REQUIRE(c[2] == di.f_value[2]);
                CATCH_REQUIRE(c[3] == di.f_value[3]);
                CATCH_REQUIRE(c[4] == di.f_value[4]);
                CATCH_REQUIRE(c[5] == di.f_value[5]);
                CATCH_REQUIRE(c[6] == di.f_value[6]);
                CATCH_REQUIRE(c[7] == di.f_value[7]);

                // operator += ()
                ai += bi;
                CATCH_REQUIRE(c[0] == ai.f_value[0]);
                CATCH_REQUIRE(c[1] == ai.f_value[1]);
                CATCH_REQUIRE(c[2] == ai.f_value[2]);
                CATCH_REQUIRE(c[3] == ai.f_value[3]);
                CATCH_REQUIRE(c[4] == ai.f_value[4]);
                CATCH_REQUIRE(c[5] == ai.f_value[5]);
                CATCH_REQUIRE(c[6] == ai.f_value[6]);
                CATCH_REQUIRE(c[7] == ai.f_value[7]);

                // operator + ()
                // TODO
                //prinbee::int512_t ds(as + bs);
                //CATCH_REQUIRE(c[0] == ds.f_value[0]);
                //CATCH_REQUIRE(c[1] == ds.f_value[1]);
                //CATCH_REQUIRE(c[2] == ds.f_value[2]);
                //CATCH_REQUIRE(c[3] == ds.f_value[3]);
                //CATCH_REQUIRE(c[4] == ds.f_value[4]);
                //CATCH_REQUIRE(c[5] == ds.f_value[5]);
                //CATCH_REQUIRE(c[6] == ds.f_value[6]);
                //CATCH_REQUIRE(c[7] == static_cast<std::uint64_t>(ds.f_high_value));

                // operator += ()
                as += bs;
                CATCH_REQUIRE(c[0] == as.f_value[0]);
                CATCH_REQUIRE(c[1] == as.f_value[1]);
                CATCH_REQUIRE(c[2] == as.f_value[2]);
                CATCH_REQUIRE(c[3] == as.f_value[3]);
                CATCH_REQUIRE(c[4] == as.f_value[4]);
                CATCH_REQUIRE(c[5] == as.f_value[5]);
                CATCH_REQUIRE(c[6] == as.f_value[6]);
                CATCH_REQUIRE(c[7] == static_cast<std::uint64_t>(as.f_high_value));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bigint substractions")
    {
        for(int count(0); count < 10; ++count)
        {
            std::size_t const size(rand() % 128 + 16);
            std::vector<std::uint64_t> a(size);
            std::vector<std::uint64_t> b(size);
            std::vector<std::uint64_t> c(size);
            std::vector<std::uint64_t> d(size);

            for(int n(0); n < 10; ++n)
            {
                int borrow(0);
                for(std::size_t i(0); i < size; ++i)
                {
                    a[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
                    b[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());

                    // "manually" compute the difference
                    c[i] = a[i] - b[i] - borrow;
                    borrow = a[i] < b[i] ? 1 : 0;
                }

                int const overflow(prinbee::sub(d.data(), a.data(), b.data(), size));

                CATCH_REQUIRE(overflow == borrow);
                for(std::size_t i(0); i < size; ++i)
                {
                    CATCH_REQUIRE(d[i] == c[i]);
                }

                // 128 bits addition
                //
                d = a;
                prinbee::sub128(d.data(), b.data());
                CATCH_REQUIRE(d[0] == c[0]);
                CATCH_REQUIRE(d[1] == c[1]);

                // 256 bits addition
                //
                d = a;
                prinbee::sub256(d.data(), b.data());
                CATCH_REQUIRE(d[0] == c[0]);
                CATCH_REQUIRE(d[1] == c[1]);
                CATCH_REQUIRE(d[2] == c[2]);
                CATCH_REQUIRE(d[3] == c[3]);

                // 512 bits addition
                //
                d = a;
                prinbee::sub512(d.data(), b.data());
                CATCH_REQUIRE(d[0] == c[0]);
                CATCH_REQUIRE(d[1] == c[1]);
                CATCH_REQUIRE(d[2] == c[2]);
                CATCH_REQUIRE(d[3] == c[3]);
                CATCH_REQUIRE(d[4] == c[4]);
                CATCH_REQUIRE(d[5] == c[5]);
                CATCH_REQUIRE(d[6] == c[6]);
                CATCH_REQUIRE(d[7] == c[7]);

                prinbee::uint512_t ai;
                ai.f_value[0] = a[0];
                ai.f_value[1] = a[1];
                ai.f_value[2] = a[2];
                ai.f_value[3] = a[3];
                ai.f_value[4] = a[4];
                ai.f_value[5] = a[5];
                ai.f_value[6] = a[6];
                ai.f_value[7] = a[7];

                prinbee::uint512_t bi;
                bi.f_value[0] = b[0];
                bi.f_value[1] = b[1];
                bi.f_value[2] = b[2];
                bi.f_value[3] = b[3];
                bi.f_value[4] = b[4];
                bi.f_value[5] = b[5];
                bi.f_value[6] = b[6];
                bi.f_value[7] = b[7];

                if(a[0] == b[0]
                && a[1] == b[1]
                && a[2] == b[2]
                && a[3] == b[3]
                && a[4] == b[4]
                && a[5] == b[5]
                && a[6] == b[6]
                && a[7] == b[7])
                {
                    // this is incredibly unlikely since we randomly generate
                    // a and b values
                    //
                    CATCH_REQUIRE(ai == bi);
                    CATCH_REQUIRE_FALSE(ai != bi);
                }
                else
                {
                    CATCH_REQUIRE_FALSE(ai == bi);
                    CATCH_REQUIRE(ai != bi);
                }

                // operator - ()
                prinbee::uint512_t di(ai - bi);
                CATCH_REQUIRE(c[0] == di.f_value[0]);
                CATCH_REQUIRE(c[1] == di.f_value[1]);
                CATCH_REQUIRE(c[2] == di.f_value[2]);
                CATCH_REQUIRE(c[3] == di.f_value[3]);
                CATCH_REQUIRE(c[4] == di.f_value[4]);
                CATCH_REQUIRE(c[5] == di.f_value[5]);
                CATCH_REQUIRE(c[6] == di.f_value[6]);
                CATCH_REQUIRE(c[7] == di.f_value[7]);

                // operator -= ()
                ai -= bi;
                CATCH_REQUIRE(c[0] == ai.f_value[0]);
                CATCH_REQUIRE(c[1] == ai.f_value[1]);
                CATCH_REQUIRE(c[2] == ai.f_value[2]);
                CATCH_REQUIRE(c[3] == ai.f_value[3]);
                CATCH_REQUIRE(c[4] == ai.f_value[4]);
                CATCH_REQUIRE(c[5] == ai.f_value[5]);
                CATCH_REQUIRE(c[6] == ai.f_value[6]);
                CATCH_REQUIRE(c[7] == ai.f_value[7]);

                // operator == () and operator != ()
                CATCH_REQUIRE(ai == ai);
                CATCH_REQUIRE_FALSE(ai != ai);

                if(bi.f_value[1] != 0
                || bi.f_value[2] != 0
                || bi.f_value[3] != 0
                || bi.f_value[4] != 0
                || bi.f_value[5] != 0
                || bi.f_value[6] != 0
                || bi.f_value[7] != 0)
                {
                    CATCH_REQUIRE_FALSE(bi == bi.f_value[0]);
                    CATCH_REQUIRE(bi != bi.f_value[0]);
                }

                bi.f_value[1] = 0;
                bi.f_value[2] = 0;
                bi.f_value[3] = 0;
                bi.f_value[4] = 0;
                bi.f_value[5] = 0;
                bi.f_value[6] = 0;
                bi.f_value[7] = 0;

                CATCH_REQUIRE(bi == bi.f_value[0]);
                CATCH_REQUIRE_FALSE(bi != bi.f_value[0]);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bigint not/neg")
    {
        for(int n(0); n < 10; ++n)
        {
            prinbee::uint512_t a;
            std::vector<std::uint64_t> not_a(8);
            std::vector<std::uint64_t> neg_a(8);
            int carry(1);
            for(std::size_t i(0); i < 8; ++i)
            {
                a.f_value[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());

                not_a[i] = ~a.f_value[i];
                neg_a[i] = not_a[i] + carry;
                carry = neg_a[i] == 0 ? 1 : 0;
            }

            prinbee::uint512_t b;
            b = ~a;

            CATCH_REQUIRE(b.f_value[0] == not_a[0]);
            CATCH_REQUIRE(b.f_value[1] == not_a[1]);
            CATCH_REQUIRE(b.f_value[2] == not_a[2]);
            CATCH_REQUIRE(b.f_value[3] == not_a[3]);
            CATCH_REQUIRE(b.f_value[4] == not_a[4]);
            CATCH_REQUIRE(b.f_value[5] == not_a[5]);
            CATCH_REQUIRE(b.f_value[6] == not_a[6]);
            CATCH_REQUIRE(b.f_value[7] == not_a[7]);

            prinbee::uint512_t c;
            c = -a;

            CATCH_REQUIRE(c.f_value[0] == neg_a[0]);
            CATCH_REQUIRE(c.f_value[1] == neg_a[1]);
            CATCH_REQUIRE(c.f_value[2] == neg_a[2]);
            CATCH_REQUIRE(c.f_value[3] == neg_a[3]);
            CATCH_REQUIRE(c.f_value[4] == neg_a[4]);
            CATCH_REQUIRE(c.f_value[5] == neg_a[5]);
            CATCH_REQUIRE(c.f_value[6] == neg_a[6]);
            CATCH_REQUIRE(c.f_value[7] == neg_a[7]);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bigint multiplication")
    {
        for(int count(0); count < 10; ++count)
        {
            prinbee::uint512_t a;
            prinbee::uint512_t b;
            prinbee::uint512_t c;

            for(int n(0); n < 10; ++n)
            {
                for(std::size_t i(0); i < 8; ++i)
                {
                    a.f_value[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
                    b.f_value[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
                }

                c = a;
                c *= b;

                Num na(a.f_value, a.f_value + 8);
                Num nb(b.f_value, b.f_value + 8);
                Num nd(na * nb);

                int idx(0);
                while(idx < std::min(static_cast<int>(nd.words.size()), 8))
                {
                    CATCH_REQUIRE(nd.words[idx] == c.f_value[idx]);
                    ++idx;
                }
                while(idx < 8) // the rest must be zeroes
                {
                    CATCH_REQUIRE(0 == c.f_value[idx]);
                    ++idx;
                }
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bigint division")
    {
        for(int count(0); count < 10; ++count)
        {
            prinbee::uint512_t a;
            prinbee::uint512_t b;
            prinbee::uint512_t c;
            prinbee::uint512_t d;

            for(int n(0); n < 10; ++n)
            {
                for(std::size_t i(0); i < 8; ++i)
                {
                    a.f_value[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
                    b.f_value[i] = (static_cast<std::uint64_t>(rand()) << 48) ^ (static_cast<std::uint64_t>(rand()) << 16) ^ static_cast<std::uint64_t>(rand());
                }

                prinbee::uint512_t const one(a / a);
                CATCH_REQUIRE(one.f_value[0] == 1);
                CATCH_REQUIRE(one.f_value[1] == 0);
                CATCH_REQUIRE(one.f_value[2] == 0);
                CATCH_REQUIRE(one.f_value[3] == 0);
                CATCH_REQUIRE(one.f_value[4] == 0);
                CATCH_REQUIRE(one.f_value[5] == 0);
                CATCH_REQUIRE(one.f_value[6] == 0);
                CATCH_REQUIRE(one.f_value[7] == 0);

                c = a;
                c /= b;

                d = a / b;
                CATCH_REQUIRE(c == d);

                Num na(a.f_value, a.f_value + 8);
                Num nb(b.f_value, b.f_value + 8);
                Num nd(na / nb);

                int idx(0);
                while(idx < static_cast<int>(nd.words.size()))
                {
                    CATCH_REQUIRE(nd.words[idx] == c.f_value[idx]);
                    ++idx;
                }
                while(idx < 8) // the rest must be zeroes
                {
                    CATCH_REQUIRE(0 == c.f_value[idx]);
                    ++idx;
                }

                c = a;
                c %= b;
                nd = na % nb;
                idx = 0;
                while(idx < static_cast<int>(nd.words.size()))
                {
                    CATCH_REQUIRE(nd.words[idx] == c.f_value[idx]);
                    ++idx;
                }
                while(idx < 8) // the rest must be zeroes
                {
                    CATCH_REQUIRE(0 == c.f_value[idx]);
                    ++idx;
                }
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("rounding", "[round] [valid]")
{
    CATCH_START_SECTION("round down")
    {
        std::uint64_t const multiple(rand() % 512 + 512);
        std::uint64_t const max(multiple * 5 + multiple / 2);
        std::uint64_t current(-multiple);
        for(std::uint64_t value(0); value < max; ++value)
        {
            if((value % multiple) == 0)
            {
                current += multiple;
            }
            CATCH_REQUIRE(current == prinbee::round_down(value, multiple));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("round up")
    {
        std::uint64_t const multiple(rand() % 512 + 512);
        std::uint64_t const max(multiple * 5 + multiple / 2);
        std::uint64_t current(0);
        for(std::uint64_t value(0); value < max; ++value)
        {
            CATCH_REQUIRE(current == prinbee::round_up(value, multiple));
            if((value % multiple) == 0)
            {
                current += multiple;
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("divide round up")
    {
        std::uint64_t const multiple(rand() % 512 + 512);
        std::uint64_t const max(multiple * 5 + multiple / 2);
        std::uint64_t current(0);
        for(std::uint64_t value(0); value < max; ++value)
        {
            CATCH_REQUIRE(current == prinbee::divide_rounded_up(value, multiple));
            if((value % multiple) == 0)
            {
                ++current;
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("bigint_invalid", "[bigint] [invalid]")
{
    CATCH_START_SECTION("bigint input too large")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::int512_t({1, 2, 3, 4, 5, 6, 7, 8, 9})
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: rhs array too large for int512_t constructor (9 > 8)."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::uint512_t({1, 2, 3, 4, 5, 6, 7, 8, 9})
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage("out_of_range: rhs array too large for uint512_t constructor (9 > 8)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bigint negative shift")
    {
        prinbee::uint512_t a({1, 2, 3, 4, 5, 6, 7, 8});
        for(int i(-10); i < 0; ++i)
        {
            CATCH_REQUIRE_THROWS_MATCHES(
                      a.lsl(i)
                    , prinbee::out_of_range
                    , Catch::Matchers::ExceptionMessage(
                          "out_of_range: lsl() cannot be called with a negative value ("
                        + std::to_string(i)
                        + ")."));

            CATCH_REQUIRE_THROWS_MATCHES(
                      a.lsr(i)
                    , prinbee::out_of_range
                    , Catch::Matchers::ExceptionMessage(
                          "out_of_range: lsr() cannot be called with a negative value ("
                        + std::to_string(i)
                        + ")."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bigint divide by zero")
    {
        prinbee::uint512_t a({1, 2, 3, 4, 5, 6, 7, 8});
        prinbee::uint512_t b;

        // 0 / n = 0
        prinbee::uint512_t zero(b / a);
        CATCH_REQUIRE(zero.is_zero());

        // n / 0 is undefined
        CATCH_REQUIRE_THROWS_MATCHES(
                  a / b
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                      "logic_error: uint512_t: division by zero not allowed."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
