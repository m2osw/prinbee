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
#include    <prinbee/utils.h>


// advgetopt
//
//#include    <advgetopt/options.h>


// snapdev
//
//#include    <snapdev/enum_class_math.h>
//#include    <snapdev/not_reached.h>


// C++
//
//#include    <iomanip>


// last include
//
#include    <snapdev/poison.h>



namespace
{




}
// no name namespace


CATCH_TEST_CASE("utils_defaults", "[utils][valid]")
{
    CATCH_START_SECTION("utils_defaults: verify default context path")
    {
        CATCH_REQUIRE(strcmp(prinbee::get_default_prinbee_path(), "/var/lib/prinbee") == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_defaults: verify default user:group names")
    {
        CATCH_REQUIRE(strcmp(prinbee::get_prinbee_user(), "prinbee") == 0);
        CATCH_REQUIRE(strcmp(prinbee::get_prinbee_group(), "prinbee") == 0);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("utils_validate_name", "[utils][valid]")
{
    CATCH_START_SECTION("utils_validate_name: validate name -- first character")
    {
        for(char c(' '); c <= '~'; ++c)
        {
            char buf[2] = { c, '\0' };
            if((c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z')
            || c == '_')
            {
                CATCH_REQUIRE(prinbee::validate_name(buf));
            }
            else
            {
                CATCH_REQUIRE_FALSE(prinbee::validate_name(buf));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_name: validate name -- beyond first character")
    {
        for(char c(' '); c <= '~'; ++c)
        {
            char buf1[3] = { '_', c, '\0' };
            char buf2[4] = { '_', c, c, '\0' };
            if((c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z')
            || (c >= '0' && c <= '9')
            || c == '_')
            {
                CATCH_REQUIRE(prinbee::validate_name(buf1));
                CATCH_REQUIRE(prinbee::validate_name(buf2));
            }
            else
            {
                CATCH_REQUIRE_FALSE(prinbee::validate_name(buf1));
                CATCH_REQUIRE_FALSE(prinbee::validate_name(buf2));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_name: validate name -- empty/null")
    {
        CATCH_REQUIRE_FALSE(prinbee::validate_name(nullptr));
        CATCH_REQUIRE_FALSE(prinbee::validate_name(""));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_name: validate name -- too long")
    {
        std::string const too_long("too_long");
        CATCH_REQUIRE(prinbee::validate_name(too_long.c_str())); // it works with default max_length

        // it fails when max_length < too_long.length()
        //
        for(std::size_t size(1); size < too_long.length(); ++size)
        {
            CATCH_REQUIRE_FALSE(prinbee::validate_name(too_long.c_str(), size));
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("utils_validate_bitfield_name", "[utils][valid]")
{
    CATCH_START_SECTION("utils_validate_bitfield_name: validate bit field name -- first character")
    {
        for(char c(' '); c <= '~'; ++c)
        {
            char buf[4] = { c, '=', c, '\0' };
            if((c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z')
            || c == '_')
            {
                CATCH_REQUIRE(prinbee::validate_bit_field_name(buf));
            }
            else
            {
                CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name(buf));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_bitfield_name: validate bit field name -- beyond first character")
    {
        for(char c(' '); c <= '~'; ++c)
        {
            char buf1[5] = { '_', c, '=', 'a', '\0' };
            char buf2[6] = { '_', c, c, '=', 'a', '\0' };
            if((c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z')
            || (c >= '0' && c <= '9')
            || c == '_')
            {
                CATCH_REQUIRE(prinbee::validate_bit_field_name(buf1));
                CATCH_REQUIRE(prinbee::validate_bit_field_name(buf2));
            }
            else
            {
                CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name(buf1));
                CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name(buf2));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_bitfield_name: validate bit field name -- empty/null")
    {
        CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name(nullptr));
        CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name(""));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_bitfield_name: validate bit field name -- too long")
    {
        std::string const too_long("too_long");
        CATCH_REQUIRE(prinbee::validate_bit_field_name((too_long + "=foo").c_str())); // it works with default max_length
        CATCH_REQUIRE(prinbee::validate_bit_field_name((too_long + "=foo").c_str(), too_long.length())); // and the exact si

        // it fails when max_length < too_long.length()
        //
        for(std::size_t size(1); size < too_long.length(); ++size)
        {
            CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name((too_long + "=foo").c_str(), size));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_bitfield_name: validate flag name -- too long")
    {
        std::string const too_long("too_long");
        CATCH_REQUIRE(prinbee::validate_bit_field_name(("b=" + too_long).c_str())); // it works with default max_length
        CATCH_REQUIRE(prinbee::validate_bit_field_name(("b=" + too_long).c_str(), too_long.length())); // and the exact size

        // it fails when max_length < too_long.length()
        //
        for(std::size_t size(1); size < too_long.length(); ++size)
        {
            CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name(("b=" + too_long).c_str(), size));
            CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name(("b=" + too_long + ":12").c_str(), size));
            CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name(("b=" + too_long + ":12/f").c_str(), size));
            CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name(("b=" + too_long + "/f").c_str(), size));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_bitfield_name: validate bit field name -- with fields")
    {
        CATCH_REQUIRE(prinbee::validate_bit_field_name("bits=foo"));
        CATCH_REQUIRE(prinbee::validate_bit_field_name("bits=foo:1"));
        CATCH_REQUIRE(prinbee::validate_bit_field_name("bits=foo:2"));
        CATCH_REQUIRE(prinbee::validate_bit_field_name("bits=foo:58"));
        CATCH_REQUIRE(prinbee::validate_bit_field_name("bits=foo:58/bar"));
        CATCH_REQUIRE(prinbee::validate_bit_field_name("bits=foo/bar:58"));
        CATCH_REQUIRE(prinbee::validate_bit_field_name("bits=foo:7/bar:9"));

        CATCH_REQUIRE(prinbee::validate_bit_field_name("eight_bits=null/advance:4/efficient:2/sign"));
        CATCH_REQUIRE(prinbee::validate_bit_field_name("bloom_filter_flags=algorithm:4/renewing"));

        CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name("bits=123"));
        CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name("bits=foo:"));
        CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name("bits=foo:/bar"));
        CATCH_REQUIRE_FALSE(prinbee::validate_bit_field_name("bits=foo/bar/"));
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("utils_validate_char_name", "[utils][valid]")
{
    CATCH_START_SECTION("utils_validate_char_name: validate CHAR field name -- first character")
    {
        for(char c(' '); c <= '~'; ++c)
        {
            char buf[4] = { c, '=', '3', '\0' };
            if((c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z')
            || c == '_')
            {
                CATCH_REQUIRE(prinbee::validate_char_field_name(buf));
            }
            else
            {
                CATCH_REQUIRE_FALSE(prinbee::validate_char_field_name(buf));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_char_name: validate CHAR field name -- beyond first character")
    {
        for(char c(' '); c <= '~'; ++c)
        {
            char buf1[5] = { '_', c, '=', '5', '\0' };
            char buf2[6] = { '_', c, c, '=', '5', '\0' };
            if((c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z')
            || (c >= '0' && c <= '9')
            || c == '_')
            {
                CATCH_REQUIRE(prinbee::validate_char_field_name(buf1));
                CATCH_REQUIRE(prinbee::validate_char_field_name(buf2));
            }
            else
            {
                CATCH_REQUIRE_FALSE(prinbee::validate_char_field_name(buf1));
                CATCH_REQUIRE_FALSE(prinbee::validate_char_field_name(buf2));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_char_name: validate CHAR field name -- empty/null")
    {
        CATCH_REQUIRE_FALSE(prinbee::validate_char_field_name(nullptr));
        CATCH_REQUIRE_FALSE(prinbee::validate_char_field_name(""));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_char_name: validate CHAR field name -- too long")
    {
        std::string const too_long("too_long");
        CATCH_REQUIRE(prinbee::validate_char_field_name((too_long + "=123").c_str())); // it works with default max_length
        CATCH_REQUIRE(prinbee::validate_char_field_name((too_long + "=123").c_str(), too_long.length())); // and the exact size

        // it fails when max_length < too_long.length()
        //
        for(std::size_t size(1); size < too_long.length(); ++size)
        {
            CATCH_REQUIRE_FALSE(prinbee::validate_char_field_name((too_long + "=123").c_str(), size));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_validate_char_name: validate CHAR field name -- with fields")
    {
        CATCH_REQUIRE(prinbee::validate_char_field_name("char=0"));
        CATCH_REQUIRE(prinbee::validate_char_field_name("char=1"));
        CATCH_REQUIRE(prinbee::validate_char_field_name("char=2"));
        CATCH_REQUIRE(prinbee::validate_char_field_name("char=12345"));
        CATCH_REQUIRE(prinbee::validate_char_field_name("char=99999999999999999999999999999999")); // number is too large, but we do not check that here

        CATCH_REQUIRE_FALSE(prinbee::validate_char_field_name("char"));
        CATCH_REQUIRE_FALSE(prinbee::validate_char_field_name("char="));
        CATCH_REQUIRE_FALSE(prinbee::validate_char_field_name("char=foo"));
        CATCH_REQUIRE_FALSE(prinbee::validate_char_field_name("char=123foo"));
        CATCH_REQUIRE_FALSE(prinbee::validate_char_field_name("char=123!"));
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("utils_invalid", "[utils][invalid]")
{
    CATCH_START_SECTION("utils_invalid: validate_name() max_length cannot be 0")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::validate_name("bad_max_length", 0)
            , prinbee::logic_error
            , Catch::Matchers::ExceptionMessage("logic_error: max_length parameter cannot be zero in validate_name()."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_invalid: validate_bit_field_name() max_length cannot be 0")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::validate_bit_field_name("bad_max_length=flag:3", 0)
            , prinbee::logic_error
            , Catch::Matchers::ExceptionMessage("logic_error: max_length parameter cannot be zero in validate_bit_field_name()."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("utils_invalid: validate_char_field_name() max_length cannot be 0")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::validate_char_field_name("bad_max_length=32", 0)
            , prinbee::logic_error
            , Catch::Matchers::ExceptionMessage("logic_error: max_length parameter cannot be zero in validate_char_field_name()."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
