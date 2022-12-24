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
#include    <prinbee/data/structure.h>


// advgetopt
//
#include    <advgetopt/options.h>


// snapdev
//
#include    <snapdev/not_reached.h>


// C++
//
#include    <iomanip>


// last include
//
#include    <snapdev/poison.h>



namespace
{



constexpr prinbee::struct_description_t g_description1[] =
{
    prinbee::define_description(
          prinbee::FieldName("magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("count")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("size")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("change")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT8) // -100 to +100
    ),
    prinbee::define_description(
          prinbee::FieldName("next")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE)
    ),
    prinbee::define_description(
          prinbee::FieldName("previous")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE)
    ),
    prinbee::end_descriptions()
};



constexpr prinbee::struct_description_t g_description2[] =
{
    prinbee::define_description(
          prinbee::FieldName("magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("flags")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("name")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P8STRING)
    ),
    prinbee::define_description(
          prinbee::FieldName("size")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT64)
    ),
    prinbee::define_description(
          prinbee::FieldName("model")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT16)
    ),
    prinbee::end_descriptions()
};



struct fixed_size_t
{
    prinbee::struct_type_t  f_type = prinbee::INVALID_STRUCT_TYPE;
    bool                    f_fixed = false;
};

std::vector<fixed_size_t> const g_fixed_sizes{
    { prinbee::struct_type_t::STRUCT_TYPE_END, true },
    { prinbee::struct_type_t::STRUCT_TYPE_VOID, true },
    { prinbee::struct_type_t::STRUCT_TYPE_BITS8, true },
    { prinbee::struct_type_t::STRUCT_TYPE_BITS16, true },
    { prinbee::struct_type_t::STRUCT_TYPE_BITS32, true },
    { prinbee::struct_type_t::STRUCT_TYPE_BITS64, true },
    { prinbee::struct_type_t::STRUCT_TYPE_BITS128, true },
    { prinbee::struct_type_t::STRUCT_TYPE_BITS256, true },
    { prinbee::struct_type_t::STRUCT_TYPE_BITS512, true },
    { prinbee::struct_type_t::STRUCT_TYPE_INT8, true },
    { prinbee::struct_type_t::STRUCT_TYPE_UINT8, true },
    { prinbee::struct_type_t::STRUCT_TYPE_INT16, true },
    { prinbee::struct_type_t::STRUCT_TYPE_UINT16, true },
    { prinbee::struct_type_t::STRUCT_TYPE_INT32, true },
    { prinbee::struct_type_t::STRUCT_TYPE_UINT32, true },
    { prinbee::struct_type_t::STRUCT_TYPE_INT64, true },
    { prinbee::struct_type_t::STRUCT_TYPE_UINT64, true },
    { prinbee::struct_type_t::STRUCT_TYPE_INT128, true },
    { prinbee::struct_type_t::STRUCT_TYPE_UINT128, true },
    { prinbee::struct_type_t::STRUCT_TYPE_INT256, true },
    { prinbee::struct_type_t::STRUCT_TYPE_UINT256, true },
    { prinbee::struct_type_t::STRUCT_TYPE_INT512, true },
    { prinbee::struct_type_t::STRUCT_TYPE_UINT512, true },
    { prinbee::struct_type_t::STRUCT_TYPE_FLOAT32, true },
    { prinbee::struct_type_t::STRUCT_TYPE_FLOAT64, true },
    { prinbee::struct_type_t::STRUCT_TYPE_FLOAT128, true },
    { prinbee::struct_type_t::STRUCT_TYPE_VERSION, true },
    { prinbee::struct_type_t::STRUCT_TYPE_TIME, true },
    { prinbee::struct_type_t::STRUCT_TYPE_MSTIME, true },
    { prinbee::struct_type_t::STRUCT_TYPE_USTIME, true },
    { prinbee::struct_type_t::STRUCT_TYPE_P8STRING, false },
    { prinbee::struct_type_t::STRUCT_TYPE_P16STRING, false },
    { prinbee::struct_type_t::STRUCT_TYPE_P32STRING, false },
    { prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE, false },
    { prinbee::struct_type_t::STRUCT_TYPE_ARRAY8, false },
    { prinbee::struct_type_t::STRUCT_TYPE_ARRAY16, false },
    { prinbee::struct_type_t::STRUCT_TYPE_ARRAY32, false },
    { prinbee::struct_type_t::STRUCT_TYPE_BUFFER8, false },
    { prinbee::struct_type_t::STRUCT_TYPE_BUFFER16, false },
    { prinbee::struct_type_t::STRUCT_TYPE_BUFFER32, false },
    { prinbee::struct_type_t::STRUCT_TYPE_REFERENCE, true },
    { prinbee::struct_type_t::STRUCT_TYPE_OID, true },
    { prinbee::struct_type_t::STRUCT_TYPE_RENAMED, true },
};


struct field_info_t
{
    char const *        f_type_name = nullptr;
    ssize_t             f_field_size = 0;
    ssize_t             f_type_field_size = 0;
};

std::vector<field_info_t> g_field_info{
    { "VOID",       0,  0 },
    { "BITS8",      1,  0 },
    { "BITS16",     2,  0 },
    { "BITS32",     4,  0 },
    { "BITS64",     8,  0 },
    { "BITS128",   16,  0 },
    { "BITS256",   32,  0 },
    { "BITS512",   64,  0 },
    { "INT8",       1,  0 },
    { "UINT8",      1,  0 },
    { "INT16",      2,  0 },
    { "UINT16",     2,  0 },
    { "INT32",      4,  0 },
    { "UINT32",     4,  0 },
    { "INT64",      8,  0 },
    { "UINT64",     8,  0 },
    { "INT128",    16,  0 },
    { "UINT128",   16,  0 },
    { "INT256",    32,  0 },
    { "UINT256",   32,  0 },
    { "INT512",    64,  0 },
    { "UINT512",   64,  0 },
    { "FLOAT32",    4,  0 },
    { "FLOAT64",    8,  0 },
    { "FLOAT128",  16,  0 },
    { "VERSION",    4,  0 },
    { "TIME",       8,  0 },
    { "MSTIME",     8,  0 },
    { "USTIME",     8,  0 },
    { "P8STRING",  -2,  1 },
    { "P16STRING", -2,  2 },
    { "P32STRING", -2,  4 },
    { "STRUCTURE", -2,  0 },
    { "ARRAY8",    -2,  1 },
    { "ARRAY16",   -2,  2 },
    { "ARRAY32",   -2,  4 },
    { "BUFFER8",   -2,  1 },
    { "BUFFER16",  -2,  2 },
    { "BUFFER32",  -2,  4 },
    { "REFERENCE",  8,  0 },
    { "OID",        8,  0 },
    { "RENAMED",   -1,  0 },
};


bool is_valid_type(prinbee::struct_type_t type)
{
    switch(type)
    {
    case prinbee::struct_type_t::STRUCT_TYPE_END:
    case prinbee::struct_type_t::STRUCT_TYPE_VOID:
    case prinbee::struct_type_t::STRUCT_TYPE_BITS8:
    case prinbee::struct_type_t::STRUCT_TYPE_BITS16:
    case prinbee::struct_type_t::STRUCT_TYPE_BITS32:
    case prinbee::struct_type_t::STRUCT_TYPE_BITS64:
    case prinbee::struct_type_t::STRUCT_TYPE_BITS128:
    case prinbee::struct_type_t::STRUCT_TYPE_BITS256:
    case prinbee::struct_type_t::STRUCT_TYPE_BITS512:
    case prinbee::struct_type_t::STRUCT_TYPE_INT8:
    case prinbee::struct_type_t::STRUCT_TYPE_UINT8:
    case prinbee::struct_type_t::STRUCT_TYPE_INT16:
    case prinbee::struct_type_t::STRUCT_TYPE_UINT16:
    case prinbee::struct_type_t::STRUCT_TYPE_INT32:
    case prinbee::struct_type_t::STRUCT_TYPE_UINT32:
    case prinbee::struct_type_t::STRUCT_TYPE_INT64:
    case prinbee::struct_type_t::STRUCT_TYPE_UINT64:
    case prinbee::struct_type_t::STRUCT_TYPE_INT128:
    case prinbee::struct_type_t::STRUCT_TYPE_UINT128:
    case prinbee::struct_type_t::STRUCT_TYPE_INT256:
    case prinbee::struct_type_t::STRUCT_TYPE_UINT256:
    case prinbee::struct_type_t::STRUCT_TYPE_INT512:
    case prinbee::struct_type_t::STRUCT_TYPE_UINT512:
    case prinbee::struct_type_t::STRUCT_TYPE_FLOAT32:
    case prinbee::struct_type_t::STRUCT_TYPE_FLOAT64:
    case prinbee::struct_type_t::STRUCT_TYPE_FLOAT128:
    case prinbee::struct_type_t::STRUCT_TYPE_VERSION:
    case prinbee::struct_type_t::STRUCT_TYPE_TIME:
    case prinbee::struct_type_t::STRUCT_TYPE_MSTIME:
    case prinbee::struct_type_t::STRUCT_TYPE_USTIME:
    case prinbee::struct_type_t::STRUCT_TYPE_P8STRING:
    case prinbee::struct_type_t::STRUCT_TYPE_P16STRING:
    case prinbee::struct_type_t::STRUCT_TYPE_P32STRING:
    case prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE:
    case prinbee::struct_type_t::STRUCT_TYPE_ARRAY8:
    case prinbee::struct_type_t::STRUCT_TYPE_ARRAY16:
    case prinbee::struct_type_t::STRUCT_TYPE_ARRAY32:
    case prinbee::struct_type_t::STRUCT_TYPE_BUFFER8:
    case prinbee::struct_type_t::STRUCT_TYPE_BUFFER16:
    case prinbee::struct_type_t::STRUCT_TYPE_BUFFER32:
    case prinbee::struct_type_t::STRUCT_TYPE_REFERENCE:
    case prinbee::struct_type_t::STRUCT_TYPE_OID:
    case prinbee::struct_type_t::STRUCT_TYPE_RENAMED:
        return true;

    default:
        return false;

    }
    snapdev::NOT_REACHED();
}

}
// no name namespace


CATCH_TEST_CASE("structure_type_name", "[structure] [type] [valid]")
{
    CATCH_START_SECTION("structure_type_name: name from type")
    {
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_END) == "END");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_VOID) == "VOID");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS8) == "BITS8");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS16) == "BITS16");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS32) == "BITS32");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS64) == "BITS64");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS128) == "BITS128");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS256) == "BITS256");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_BITS512) == "BITS512");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_INT8) == "INT8");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT8) == "UINT8");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_INT16) == "INT16");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT16) == "UINT16");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_INT32) == "INT32");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT32) == "UINT32");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_INT64) == "INT64");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT64) == "UINT64");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_INT128) == "INT128");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT128) == "UINT128");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_INT256) == "INT256");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT256) == "UINT256");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_INT512) == "INT512");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT512) == "UINT512");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_FLOAT32) == "FLOAT32");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_FLOAT64) == "FLOAT64");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_FLOAT128) == "FLOAT128");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_VERSION) == "VERSION");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_TIME) == "TIME");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_MSTIME) == "MSTIME");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_USTIME) == "USTIME");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_P8STRING) == "P8STRING");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_P16STRING) == "P16STRING");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_P32STRING) == "P32STRING");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE) == "STRUCTURE");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_ARRAY8) == "ARRAY8");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_ARRAY16) == "ARRAY16");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_ARRAY32) == "ARRAY32");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER8) == "BUFFER8");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER16) == "BUFFER16");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_BUFFER32) == "BUFFER32");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE) == "REFERENCE");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_OID) == "OID");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_RENAMED) == "RENAMED");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_type_name: type from name")
    {
        CATCH_REQUIRE(prinbee::name_to_struct_type("END") == prinbee::struct_type_t::STRUCT_TYPE_END);
        CATCH_REQUIRE(prinbee::name_to_struct_type("VOID") == prinbee::struct_type_t::STRUCT_TYPE_VOID);
        CATCH_REQUIRE(prinbee::name_to_struct_type("BITS8") == prinbee::struct_type_t::STRUCT_TYPE_BITS8);
        CATCH_REQUIRE(prinbee::name_to_struct_type("BITS16") == prinbee::struct_type_t::STRUCT_TYPE_BITS16);
        CATCH_REQUIRE(prinbee::name_to_struct_type("BITS32") == prinbee::struct_type_t::STRUCT_TYPE_BITS32);
        CATCH_REQUIRE(prinbee::name_to_struct_type("BITS64") == prinbee::struct_type_t::STRUCT_TYPE_BITS64);
        CATCH_REQUIRE(prinbee::name_to_struct_type("BITS128") == prinbee::struct_type_t::STRUCT_TYPE_BITS128);
        CATCH_REQUIRE(prinbee::name_to_struct_type("BITS256") == prinbee::struct_type_t::STRUCT_TYPE_BITS256);
        CATCH_REQUIRE(prinbee::name_to_struct_type("BITS512") == prinbee::struct_type_t::STRUCT_TYPE_BITS512);
        CATCH_REQUIRE(prinbee::name_to_struct_type("INT8") == prinbee::struct_type_t::STRUCT_TYPE_INT8);
        CATCH_REQUIRE(prinbee::name_to_struct_type("UINT8") == prinbee::struct_type_t::STRUCT_TYPE_UINT8);
        CATCH_REQUIRE(prinbee::name_to_struct_type("INT16") == prinbee::struct_type_t::STRUCT_TYPE_INT16);
        CATCH_REQUIRE(prinbee::name_to_struct_type("UINT16") == prinbee::struct_type_t::STRUCT_TYPE_UINT16);
        CATCH_REQUIRE(prinbee::name_to_struct_type("INT32") == prinbee::struct_type_t::STRUCT_TYPE_INT32);
        CATCH_REQUIRE(prinbee::name_to_struct_type("UINT32") == prinbee::struct_type_t::STRUCT_TYPE_UINT32);
        CATCH_REQUIRE(prinbee::name_to_struct_type("INT64") == prinbee::struct_type_t::STRUCT_TYPE_INT64);
        CATCH_REQUIRE(prinbee::name_to_struct_type("UINT64") == prinbee::struct_type_t::STRUCT_TYPE_UINT64);
        CATCH_REQUIRE(prinbee::name_to_struct_type("INT128") == prinbee::struct_type_t::STRUCT_TYPE_INT128);
        CATCH_REQUIRE(prinbee::name_to_struct_type("UINT128") == prinbee::struct_type_t::STRUCT_TYPE_UINT128);
        CATCH_REQUIRE(prinbee::name_to_struct_type("INT256") == prinbee::struct_type_t::STRUCT_TYPE_INT256);
        CATCH_REQUIRE(prinbee::name_to_struct_type("UINT256") == prinbee::struct_type_t::STRUCT_TYPE_UINT256);
        CATCH_REQUIRE(prinbee::name_to_struct_type("INT512") == prinbee::struct_type_t::STRUCT_TYPE_INT512);
        CATCH_REQUIRE(prinbee::name_to_struct_type("UINT512") == prinbee::struct_type_t::STRUCT_TYPE_UINT512);
        CATCH_REQUIRE(prinbee::name_to_struct_type("FLOAT32") == prinbee::struct_type_t::STRUCT_TYPE_FLOAT32);
        CATCH_REQUIRE(prinbee::name_to_struct_type("FLOAT64") == prinbee::struct_type_t::STRUCT_TYPE_FLOAT64);
        CATCH_REQUIRE(prinbee::name_to_struct_type("FLOAT128") == prinbee::struct_type_t::STRUCT_TYPE_FLOAT128);
        CATCH_REQUIRE(prinbee::name_to_struct_type("VERSION") == prinbee::struct_type_t::STRUCT_TYPE_VERSION);
        CATCH_REQUIRE(prinbee::name_to_struct_type("TIME") == prinbee::struct_type_t::STRUCT_TYPE_TIME);
        CATCH_REQUIRE(prinbee::name_to_struct_type("MSTIME") == prinbee::struct_type_t::STRUCT_TYPE_MSTIME);
        CATCH_REQUIRE(prinbee::name_to_struct_type("USTIME") == prinbee::struct_type_t::STRUCT_TYPE_USTIME);
        CATCH_REQUIRE(prinbee::name_to_struct_type("P8STRING") == prinbee::struct_type_t::STRUCT_TYPE_P8STRING);
        CATCH_REQUIRE(prinbee::name_to_struct_type("P16STRING") == prinbee::struct_type_t::STRUCT_TYPE_P16STRING);
        CATCH_REQUIRE(prinbee::name_to_struct_type("P32STRING") == prinbee::struct_type_t::STRUCT_TYPE_P32STRING);
        CATCH_REQUIRE(prinbee::name_to_struct_type("STRUCTURE") == prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE);
        CATCH_REQUIRE(prinbee::name_to_struct_type("ARRAY8") == prinbee::struct_type_t::STRUCT_TYPE_ARRAY8);
        CATCH_REQUIRE(prinbee::name_to_struct_type("ARRAY16") == prinbee::struct_type_t::STRUCT_TYPE_ARRAY16);
        CATCH_REQUIRE(prinbee::name_to_struct_type("ARRAY32") == prinbee::struct_type_t::STRUCT_TYPE_ARRAY32);
        CATCH_REQUIRE(prinbee::name_to_struct_type("BUFFER8") == prinbee::struct_type_t::STRUCT_TYPE_BUFFER8);
        CATCH_REQUIRE(prinbee::name_to_struct_type("BUFFER16") == prinbee::struct_type_t::STRUCT_TYPE_BUFFER16);
        CATCH_REQUIRE(prinbee::name_to_struct_type("BUFFER32") == prinbee::struct_type_t::STRUCT_TYPE_BUFFER32);
        CATCH_REQUIRE(prinbee::name_to_struct_type("REFERENCE") == prinbee::struct_type_t::STRUCT_TYPE_REFERENCE);
        CATCH_REQUIRE(prinbee::name_to_struct_type("OID") == prinbee::struct_type_t::STRUCT_TYPE_OID);
        CATCH_REQUIRE(prinbee::name_to_struct_type("RENAMED") == prinbee::struct_type_t::STRUCT_TYPE_RENAMED);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_type_name_invalid", "[structure] [type] [invalid]")
{
    CATCH_START_SECTION("structure_type_name: unknown")
    {
        for(int i(0); i < 100; ++i)
        {
            prinbee::struct_type_t bad_type(prinbee::struct_type_t::STRUCT_TYPE_END);
            do
            {
                bad_type = static_cast<prinbee::struct_type_t>(SNAP_CATCH2_NAMESPACE::rand32());
            }
            while(is_valid_type(bad_type));
            std::string const expected(
                      "*unknown struct type ("
                    + std::to_string(static_cast<int>(bad_type))
                    + ")*");
            CATCH_REQUIRE(expected == prinbee::to_string(bad_type));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_type_name: invalid")
    {
        for(int i(0); i < 100; ++i)
        {
            //prinbee::struct_type_t bad_type(prinbee::struct_type_t::STRUCT_TYPE_END);
            std::string const bad_type_name(SNAP_CATCH2_NAMESPACE::rand_string(rand() % 10 + 1));
            prinbee::struct_type_t type(prinbee::name_to_struct_type(bad_type_name));
            if(type != prinbee::INVALID_STRUCT_TYPE)
            {
                // the rand_string() could return a valid name (very unlikely, though)
                //
                CATCH_REQUIRE(prinbee::to_string(type) == bad_type_name);  // not that bad after all...
            }
            else
            {
                // avoid catch2 error "no assertion in section ..."
                //
                CATCH_REQUIRE(prinbee::INVALID_STRUCT_TYPE == type);
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_type_metadata", "[structure] [type] [valid]")
{
    CATCH_START_SECTION("structure_type_metadata: fixed size")
    {
        for(auto const & f : g_fixed_sizes)
        {
            CATCH_REQUIRE(f.f_fixed == prinbee::type_with_fixed_size(f.f_type));
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_version_basics", "[structure] [version]")
{
    CATCH_START_SECTION("version: default")
    {
        prinbee::version_t version;
        CATCH_REQUIRE(version.get_major() == 0);
        CATCH_REQUIRE(version.get_minor() == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("version: conversions")
    {
        for(int n(0); n < 100; ++n)
        {
            int const major_version(rand() & 0xFFFF);
            int const minor_version(rand() & 0xFFFF);

            std::uint32_t const binary((major_version << 16) + minor_version);

            prinbee::version_t const v1(major_version, minor_version);
            CATCH_REQUIRE(v1.get_major() == major_version);
            CATCH_REQUIRE(v1.get_minor() == minor_version);
            CATCH_REQUIRE(v1.to_binary() == binary);

            prinbee::version_t v2;
            CATCH_REQUIRE(v2.get_major() == 0);
            CATCH_REQUIRE(v2.get_minor() == 0);
            CATCH_REQUIRE(v2.is_null());
            CATCH_REQUIRE(v2 != v1);
            v2.from_binary(binary);
            CATCH_REQUIRE(v2.get_major() == major_version);
            CATCH_REQUIRE(v2.get_minor() == minor_version);
            CATCH_REQUIRE(v2.to_binary() == binary);
            CATCH_REQUIRE(v2 == v1);

            v2.next_revision();

            if(minor_version == 0xFFFF)
            {
                CATCH_REQUIRE(v2.get_major() == major_version + 1);
                CATCH_REQUIRE(v2.get_minor() == 0);
            }
            else
            {
                CATCH_REQUIRE(v2.get_major() == major_version);
                CATCH_REQUIRE(v2.get_minor() == minor_version + 1);
            }

            v2 = v1;
            int const new_major_version(rand() & 0xFFFF);
            v2.set_major(new_major_version);
            CATCH_REQUIRE(v2.get_major() == new_major_version);
            CATCH_REQUIRE(v2.get_minor() == minor_version);
            CATCH_REQUIRE(v2 != v1);

            int const new_minor_version(rand() & 0xFFFF);
            v2.set_minor(new_minor_version);
            CATCH_REQUIRE(v2.get_major() == new_major_version);
            CATCH_REQUIRE(v2.get_minor() == new_minor_version);
            CATCH_REQUIRE(v2 != v1);

            v2 = v1;
            CATCH_REQUIRE(v2.get_major() == major_version);
            CATCH_REQUIRE(v2.get_minor() == minor_version);
            CATCH_REQUIRE(v2.to_binary() == binary);
            CATCH_REQUIRE(v2 == v1);

            prinbee::version_t v3(v1);
            CATCH_REQUIRE_FALSE(v3.is_null());
            CATCH_REQUIRE(v3.get_major() == major_version);
            CATCH_REQUIRE(v3.get_minor() == minor_version);
            CATCH_REQUIRE(v3.to_binary() == binary);
            CATCH_REQUIRE(v3 == v1);
            CATCH_REQUIRE_FALSE(v3 > v1);
            CATCH_REQUIRE(v3 >= v1);
            CATCH_REQUIRE_FALSE(v3 < v1);
            CATCH_REQUIRE(v3 <= v1);

            std::string v3_str(v3.to_string());
            std::string version_str;
            version_str += std::to_string(major_version);
            version_str += '.';
            version_str += std::to_string(minor_version);
            CATCH_REQUIRE(v3_str == version_str);

            v3.next_branch();
            CATCH_REQUIRE(v3.get_major() == major_version + 1);
            CATCH_REQUIRE(v3.get_minor() == 0);
            CATCH_REQUIRE(v3.to_binary() == static_cast<uint32_t>((major_version + 1) << 16));

            prinbee::version_t v4(binary);
            CATCH_REQUIRE_FALSE(v4.is_null());
            CATCH_REQUIRE(v4.get_major() == major_version);
            CATCH_REQUIRE(v4.get_minor() == minor_version);
            CATCH_REQUIRE(v4.to_binary() == binary);
            CATCH_REQUIRE(v1 == v4);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_version_compare", "[structure] [version]")
{
    CATCH_START_SECTION("version: compare")
    {
        for(int n(0); n < 100; ++n)
        {
            int major_version(rand() & 0xFFFF);
            int minor_version(rand() & 0xFFFF);
            int major_version2(rand() & 0xFFFF);
            while(major_version == major_version2)
            {
                major_version2 = rand() & 0xFFFF;
            }

            prinbee::version_t v1(major_version, minor_version);
            prinbee::version_t v2(major_version2, minor_version);
            if(major_version < major_version2)
            {
                CATCH_REQUIRE_FALSE(v1 == v2);
                CATCH_REQUIRE(v1 != v2);
                CATCH_REQUIRE(v1 < v2);
                CATCH_REQUIRE(v1 <= v2);
                CATCH_REQUIRE(v2 > v1);
                CATCH_REQUIRE(v2 >= v1);
            }
            else
            {
                CATCH_REQUIRE_FALSE(v1 == v2);
                CATCH_REQUIRE(v1 != v2);
                CATCH_REQUIRE(v1 > v2);
                CATCH_REQUIRE(v1 >= v2);
                CATCH_REQUIRE(v2 < v1);
                CATCH_REQUIRE(v2 <= v1);
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_version_overflow", "[structure] [version] [invalid]")
{
    CATCH_START_SECTION("version overflow")
    {
        for(int n(0); n < 100; ++n)
        {
            int major_version(0);
            int minor_version(0);
            do
            {
                major_version = SNAP_CATCH2_NAMESPACE::rand32();
                minor_version = SNAP_CATCH2_NAMESPACE::rand32();
            }
            while(major_version < 65536
               && minor_version < 65536);

            CATCH_REQUIRE_THROWS_MATCHES(
                      prinbee::version_t(major_version, minor_version)
                    , prinbee::invalid_parameter
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: major/minor version must be between 0 and 65535 inclusive, "
                            + std::to_string(major_version)
                            + "."
                            + std::to_string(minor_version)
                            + " is incorrect."));
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_min_max_version", "[structure] [version] [valid]")
{
    CATCH_START_SECTION("min_max_version: default")
    {
        prinbee::version_t version = prinbee::version_t();
        prinbee::min_max_version_t zero;
        CATCH_REQUIRE(version == zero.min());
        CATCH_REQUIRE(version == zero.max());
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_flag_definitions", "[structure] [flags] [valid]")
{
    CATCH_START_SECTION("structure_flag_definitions: all positions")
    {
        for(std::size_t pos(0); pos < 64; ++pos)
        {
            std::string const field_name(SNAP_CATCH2_NAMESPACE::rand_string(rand() % 100 + 1));
            std::string const flag_name(SNAP_CATCH2_NAMESPACE::rand_string(rand() % 100 + 1));

            std::string full_name(field_name);
            full_name += '.';
            full_name += flag_name;

            prinbee::flag_definition const def(field_name, flag_name, pos);
            CATCH_REQUIRE(full_name == def.full_name());
            CATCH_REQUIRE(field_name == def.field_name());
            CATCH_REQUIRE(flag_name == def.flag_name());
            CATCH_REQUIRE(pos == def.pos());
            CATCH_REQUIRE(1ULL == def.size());
            CATCH_REQUIRE((1ULL << pos) == def.mask());
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_flag_definitions_incorrect_construction", "[structure] [flags] [invalid]")
{
    CATCH_START_SECTION("structure_flag_definitions: missing name(s)")
    {
        // missing flag name
        std::string name(SNAP_CATCH2_NAMESPACE::rand_string(rand() % 100 + 1));
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::flag_definition(name, std::string(), rand(), rand())
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: the flag definition must have a non-empty field name and flag name."));

        // missing field name
        name = SNAP_CATCH2_NAMESPACE::rand_string(rand() % 100 + 1);
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::flag_definition(std::string(), name, rand())
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: the flag definition must have a non-empty field name and flag name."));

        // missing both names
        name = SNAP_CATCH2_NAMESPACE::rand_string(rand() % 100 + 1);
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::flag_definition(std::string(), std::string(), rand() % 64, rand())
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: the flag definition must have a non-empty field name and flag name."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_flag_definitions: unsupported sizes / positions")
    {
        // zero fails
        //
        std::string field_name(SNAP_CATCH2_NAMESPACE::rand_string(rand() % 100 + 1));
        std::string flag_name(SNAP_CATCH2_NAMESPACE::rand_string(rand() % 100 + 1));
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::flag_definition(field_name, flag_name, rand(), 0ULL)
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: bit field named \""
                        + field_name
                        + "."
                        + flag_name
                        + "\" can't have a size of 0."));

        for(std::size_t size(64); size < 100; ++size)
        {
            CATCH_REQUIRE_THROWS_MATCHES(
                      prinbee::flag_definition(field_name, flag_name, rand(), size)
                    , prinbee::invalid_parameter
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: bit field named \""
                            + field_name
                            + "."
                            + flag_name
                            + "\" is too large ("
                            + std::to_string(size)
                            + " >= 64)."));
        }

        for(std::size_t size(1ULL); size < 64ULL; ++size)
        {
            for(std::size_t pos(65ULL - size); pos < 100ULL; ++pos)
            {
                CATCH_REQUIRE_THROWS_MATCHES(
                          prinbee::flag_definition(field_name, flag_name, pos, size)
                        , prinbee::invalid_parameter
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: the mask of the bit field named \""
                                + field_name
                                + "."
                                + flag_name
                                + "\" does not fit in a uint64_t."));
            }
        }

        // position 65+ is not valid either
        //
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::flag_definition(field_name, flag_name, 65ULL)
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: the mask of the bit field named \""
                        + field_name
                        + "."
                        + flag_name
                        + "\" does not fit in a uint64_t."));
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_field", "[structure] [valid]")
{
    CATCH_START_SECTION("structure_field: check description of all different database types")
    {
        for(auto const & info : g_field_info)
        {
            prinbee::field_t::field_flags_t flags((rand() & 1) != 0 ? 0 : prinbee::STRUCT_DESCRIPTION_FLAG_OPTIONAL);
            prinbee::struct_type_t const type(prinbee::name_to_struct_type(info.f_type_name));
            prinbee::struct_description_t description =
            {
                prinbee::define_description(
                      prinbee::FieldName(info.f_type_name)
                    , prinbee::FieldType(type)
                    , prinbee::FieldFlags(flags)
                ),
            };

            prinbee::field_t::const_pointer_t f(std::make_shared<prinbee::field_t>(&description));

            CATCH_REQUIRE(&description == f->description());
            CATCH_REQUIRE(nullptr == f->next());
            CATCH_REQUIRE(nullptr == f->previous());
            CATCH_REQUIRE(f == f->first());
            CATCH_REQUIRE(f == f->last());
            CATCH_REQUIRE(type == f->type());
            CATCH_REQUIRE(info.f_field_size == f->field_size());
            CATCH_REQUIRE(info.f_type_field_size == f->type_field_size());
            CATCH_REQUIRE(info.f_type_name == f->field_name());
            CATCH_REQUIRE(0 == f->size());

            std::uint32_t const size(SNAP_CATCH2_NAMESPACE::rand32());
            std::const_pointer_cast<prinbee::field_t>(f)->set_size(size);
            CATCH_REQUIRE(size == f->size());

            // the flag are set by the structure parser, so here it's never set
            // whether it is defined in the description above
            //
            CATCH_REQUIRE_FALSE(f->has_flags(prinbee::field_t::FIELD_FLAG_VARIABLE_SIZE));
            CATCH_REQUIRE(0 == f->flags());

            std::const_pointer_cast<prinbee::field_t>(f)->set_flags(prinbee::field_t::FIELD_FLAG_VARIABLE_SIZE);
            CATCH_REQUIRE(f->has_flags(prinbee::field_t::FIELD_FLAG_VARIABLE_SIZE));
            CATCH_REQUIRE(prinbee::field_t::FIELD_FLAG_VARIABLE_SIZE == f->flags());

            std::const_pointer_cast<prinbee::field_t>(f)->clear_flags(prinbee::field_t::FIELD_FLAG_VARIABLE_SIZE);
            CATCH_REQUIRE_FALSE(f->has_flags(prinbee::field_t::FIELD_FLAG_VARIABLE_SIZE));
            CATCH_REQUIRE(0 == f->flags());

            std::const_pointer_cast<prinbee::field_t>(f)->add_flags(prinbee::field_t::FIELD_FLAG_VARIABLE_SIZE);
            CATCH_REQUIRE(f->has_flags(prinbee::field_t::FIELD_FLAG_VARIABLE_SIZE));
            CATCH_REQUIRE(prinbee::field_t::FIELD_FLAG_VARIABLE_SIZE == f->flags());

            CATCH_REQUIRE(0 == f->offset());

            std::uint64_t const offset(SNAP_CATCH2_NAMESPACE::rand64());
            std::const_pointer_cast<prinbee::field_t>(f)->set_offset(offset);
            CATCH_REQUIRE(offset == f->offset());

            std::int64_t const adjust(SNAP_CATCH2_NAMESPACE::rand64());
            std::const_pointer_cast<prinbee::field_t>(f)->adjust_offset(adjust);
            CATCH_REQUIRE(offset + adjust == f->offset());

            CATCH_REQUIRE(f->sub_structures().empty());
            CATCH_REQUIRE(std::const_pointer_cast<prinbee::field_t>(f)->sub_structures().empty());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_field: check flag definitions")
    {
        prinbee::struct_description_t description =
        {
            prinbee::define_description(
                  prinbee::FieldName("flags")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_BITS64)
            ),
        };

        prinbee::field_t::pointer_t f(std::make_shared<prinbee::field_t>(&description));

        CATCH_REQUIRE(&description == f->description());
        CATCH_REQUIRE(nullptr == f->next());
        CATCH_REQUIRE(nullptr == f->previous());
        CATCH_REQUIRE(f == f->first());
        CATCH_REQUIRE(f == f->last());
        CATCH_REQUIRE(prinbee::struct_type_t::STRUCT_TYPE_BITS64 == f->type());
        CATCH_REQUIRE(8 == f->field_size());
        CATCH_REQUIRE(0 == f->type_field_size());
        CATCH_REQUIRE("flags" == f->field_name());
        CATCH_REQUIRE(0 == f->size());
        CATCH_REQUIRE_FALSE(f->has_flags(prinbee::field_t::FIELD_FLAG_VARIABLE_SIZE));
        CATCH_REQUIRE(0 == f->flags());
        CATCH_REQUIRE(0 == f->offset());

//    flag_definition::pointer_t              find_flag_definition(std::string const & name) const;
//    void                                    add_flag_definition(flag_definition::pointer_t bits);

        for(int i(1); i <= 10; ++i)
        {
            std::string const name("f" + std::to_string(i));
            prinbee::flag_definition::pointer_t flag(std::make_shared<prinbee::flag_definition>("flags", name, i * 3, 3));
            CATCH_REQUIRE(flag->full_name() == "flags." + name);
            f->add_flag_definition(flag);
            CATCH_REQUIRE(flag == f->find_flag_definition(name));
        }

        // make sure they stay around
        //
        for(int i(1); i <= 10; ++i)
        {
            std::string const name("f" + std::to_string(i));
            CATCH_REQUIRE(f->find_flag_definition(name)->full_name() == "flags." + name);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_field: next/previous (1)")
    {
        prinbee::struct_description_t description[3] =
        {
            prinbee::define_description(
                  prinbee::FieldName("head")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT32)
            ),
            prinbee::define_description(
                  prinbee::FieldName("cont")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT64)
            ),
            prinbee::define_description(
                  prinbee::FieldName("tail")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_BITS16)
            ),
        };

        prinbee::field_t::pointer_t first(std::make_shared<prinbee::field_t>(description + 0));
        prinbee::field_t::pointer_t middle(std::make_shared<prinbee::field_t>(description + 1));
        prinbee::field_t::pointer_t last(std::make_shared<prinbee::field_t>(description + 2));

        // fields are used internally so the linking requires two calls
        //
        first->set_next(middle);
        middle->set_previous(first);

        middle->set_next(last);
        last->set_previous(middle);

        CATCH_REQUIRE(nullptr == first->previous());
        CATCH_REQUIRE(middle == first->next());
        CATCH_REQUIRE(first == middle->previous());
        CATCH_REQUIRE(last == middle->next());
        CATCH_REQUIRE(middle == last->previous());
        CATCH_REQUIRE(nullptr == last->next());

        CATCH_REQUIRE(first == first->first());
        CATCH_REQUIRE(first == middle->first());
        CATCH_REQUIRE(first == last->first());
        CATCH_REQUIRE(last == first->last());
        CATCH_REQUIRE(last == middle->last());
        CATCH_REQUIRE(last == last->last());

        CATCH_REQUIRE(first->type() == prinbee::struct_type_t::STRUCT_TYPE_INT32);
        CATCH_REQUIRE(middle->type() == prinbee::struct_type_t::STRUCT_TYPE_UINT64);
        CATCH_REQUIRE(last->type() == prinbee::struct_type_t::STRUCT_TYPE_BITS16);

        // when last pointer gets reset, it changes a few things
        //
        last.reset();
        CATCH_REQUIRE(last == nullptr);

        CATCH_REQUIRE(nullptr == first->previous());
        CATCH_REQUIRE(middle == first->next());
        CATCH_REQUIRE(first == first->first());
        CATCH_REQUIRE(middle == first->last());

        CATCH_REQUIRE(first == middle->previous());
        CATCH_REQUIRE(nullptr == middle->next());
        CATCH_REQUIRE(first == middle->first());
        CATCH_REQUIRE(middle == middle->last());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_field: next/previous (2)")
    {
        prinbee::struct_description_t description[5] =
        {
            prinbee::define_description(
                  prinbee::FieldName("head")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT32)
            ),
            prinbee::define_description(
                  prinbee::FieldName("early")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT64)
            ),
            prinbee::define_description(
                  prinbee::FieldName("middle")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT256)
            ),
            prinbee::define_description(
                  prinbee::FieldName("late")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_REFERENCE)
            ),
            prinbee::define_description(
                  prinbee::FieldName("tail")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_BITS16)
            ),
        };

        prinbee::field_t::pointer_t first(std::make_shared<prinbee::field_t>(description + 0));
        prinbee::field_t::pointer_t early(std::make_shared<prinbee::field_t>(description + 1));
        prinbee::field_t::pointer_t middle(std::make_shared<prinbee::field_t>(description + 2));
        prinbee::field_t::pointer_t late(std::make_shared<prinbee::field_t>(description + 3));
        prinbee::field_t::pointer_t last(std::make_shared<prinbee::field_t>(description + 4));

        // fields are used internally so the linking requires two calls
        //
        first->set_next(early);
        early->set_previous(first);

        early->set_next(middle);
        middle->set_previous(early);

        middle->set_next(late);
        late->set_previous(middle);

        late->set_next(last);
        last->set_previous(late);

        CATCH_REQUIRE(nullptr == first->previous());
        CATCH_REQUIRE(early == first->next());
        CATCH_REQUIRE(first == early->previous());
        CATCH_REQUIRE(middle == early->next());
        CATCH_REQUIRE(early == middle->previous());
        CATCH_REQUIRE(late == middle->next());
        CATCH_REQUIRE(middle == late->previous());
        CATCH_REQUIRE(last == late->next());
        CATCH_REQUIRE(late == last->previous());
        CATCH_REQUIRE(nullptr == last->next());

        CATCH_REQUIRE(first == first->first());
        CATCH_REQUIRE(first == early->first());
        CATCH_REQUIRE(first == middle->first());
        CATCH_REQUIRE(first == late->first());
        CATCH_REQUIRE(first == last->first());
        CATCH_REQUIRE(last == first->last());
        CATCH_REQUIRE(last == early->last());
        CATCH_REQUIRE(last == middle->last());
        CATCH_REQUIRE(last == late->last());
        CATCH_REQUIRE(last == last->last());

        CATCH_REQUIRE(first->type() == prinbee::struct_type_t::STRUCT_TYPE_INT32);
        CATCH_REQUIRE(early->type() == prinbee::struct_type_t::STRUCT_TYPE_UINT64);
        CATCH_REQUIRE(middle->type() == prinbee::struct_type_t::STRUCT_TYPE_INT256);
        CATCH_REQUIRE(late->type() == prinbee::struct_type_t::STRUCT_TYPE_REFERENCE);
        CATCH_REQUIRE(last->type() == prinbee::struct_type_t::STRUCT_TYPE_BITS16);

        // when middle pointer gets reset, it changes a few things
        //
        middle.reset();
        CATCH_REQUIRE(middle == nullptr);

        CATCH_REQUIRE(nullptr == first->previous());
        CATCH_REQUIRE(early == first->next());
        CATCH_REQUIRE(first == first->first());
        CATCH_REQUIRE(last == first->last());

        CATCH_REQUIRE(first == early->previous());
        CATCH_REQUIRE(late == early->next());
        CATCH_REQUIRE(first == early->first());
        CATCH_REQUIRE(last == early->last());

        CATCH_REQUIRE(early == late->previous());
        CATCH_REQUIRE(last == late->next());
        CATCH_REQUIRE(first == late->first());
        CATCH_REQUIRE(last == late->last());

        CATCH_REQUIRE(late == last->previous());
        CATCH_REQUIRE(nullptr == last->next());
        CATCH_REQUIRE(first == last->first());
        CATCH_REQUIRE(last == last->last());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_field: new name with a RENAMED but no field name")
    {
        prinbee::struct_description_t rename =
        {
            prinbee::define_description(
                  prinbee::FieldName("true_name")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT512)
            ),
        };

        prinbee::struct_description_t description =
        {
            prinbee::define_description(
                  prinbee::FieldName("name_missing")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_RENAMED)
                , prinbee::FieldSubDescription(&rename)
            ),
        };

        prinbee::field_t::pointer_t f(std::make_shared<prinbee::field_t>(&description));

        CATCH_REQUIRE("true_name" == f->new_name());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_field: sub-structures")
    {
        prinbee::struct_description_t description =
        {
            prinbee::define_description(
                  prinbee::FieldName("structure")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE)
            ),
        };

        prinbee::field_t::pointer_t f(std::make_shared<prinbee::field_t>(&description));

        prinbee::structure::pointer_t s(std::make_shared<prinbee::structure>(g_description1));
        f->sub_structures().push_back(s);
        CATCH_REQUIRE((*f)[0] == s);

        prinbee::structure::vector_t v;
        prinbee::structure::pointer_t s1(std::make_shared<prinbee::structure>(g_description1));
        v.push_back(s1);
        prinbee::structure::pointer_t s2(std::make_shared<prinbee::structure>(g_description1));
        v.push_back(s2);
        prinbee::structure::pointer_t s3(std::make_shared<prinbee::structure>(g_description1));
        v.push_back(s3);
        f->set_sub_structures(v);
        CATCH_REQUIRE((*f)[0] == s1);
        CATCH_REQUIRE((*f)[1] == s2);
        CATCH_REQUIRE((*f)[2] == s3);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_invalid_field", "[structure] [valid]")
{
    CATCH_START_SECTION("structure_invalid_field: check description of all different database types")
    {
        // generate an invalid type and then try a field description
        // with such, we should get errors in various places
        //
        prinbee::struct_type_t bad_type(static_cast<prinbee::struct_type_t>(rand()));
        while(is_valid_type(bad_type))
        {
            bad_type = static_cast<prinbee::struct_type_t>(rand());
        }
        prinbee::struct_description_t description =
        {
            prinbee::define_description(
                  prinbee::FieldName("INVALID")
                , prinbee::FieldType(bad_type)
            ),
        };

        prinbee::field_t::const_pointer_t f(std::make_shared<prinbee::field_t>(&description));

        CATCH_REQUIRE_THROWS_MATCHES(
                  f->field_size()
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                          "out_of_range: type out of range for converting it to a field size ("
                          "*unknown struct type ("
                        + std::to_string(static_cast<int>(bad_type))
                        + ")*"
                          ", max: 43).")); // this number is not defined otherwise...

        CATCH_REQUIRE_THROWS_MATCHES(
                  f->type_field_size()
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                          "out_of_range: type out of range for converting it to a type field size ("
                          "*unknown struct type ("
                        + std::to_string(static_cast<int>(bad_type))
                        + ")*"
                          ", max: 43).")); // this number is not defined otherwise...
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid_field: new name without a RENAMED")
    {
        prinbee::struct_description_t description =
        {
            prinbee::define_description(
                  prinbee::FieldName("not_renamed")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_ARRAY8) // <- wrong type
            ),
        };

        prinbee::field_t::pointer_t f(std::make_shared<prinbee::field_t>(&description));

        CATCH_REQUIRE_THROWS_MATCHES(
                  f->new_name()
                , prinbee::type_mismatch
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: field \"not_renamed\" is not a RENAMED field, it has no new name."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid_field: new name without a sub-description")
    {
        prinbee::struct_description_t description =
        {
            prinbee::define_description(
                  prinbee::FieldName("no_link")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_RENAMED)
            ),
        };

        prinbee::field_t::pointer_t f(std::make_shared<prinbee::field_t>(&description));

        CATCH_REQUIRE_THROWS_MATCHES(
                  f->new_name()
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                          "logic_error: field \"no_link\" is marked as having a new name (RENAMED) but it has no f_sub_description to define the new name."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid_field: new name with a RENAMED but no field name")
    {
        prinbee::struct_description_t rename =
        {
            prinbee::define_description(
                  prinbee::FieldName(nullptr)
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT512)
            ),
        };

        prinbee::struct_description_t description =
        {
            prinbee::define_description(
                  prinbee::FieldName("name_missing")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_RENAMED)
                , prinbee::FieldSubDescription(&rename)
            ),
        };

        prinbee::field_t::pointer_t f(std::make_shared<prinbee::field_t>(&description));

        CATCH_REQUIRE_THROWS_MATCHES(
                  f->new_name()
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                          "logic_error: field \"name_missing\" is marked as having a new name (RENAMED) but it has no entries in its f_sub_description defining the new name."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid_field: unknown flag")
    {
        prinbee::struct_description_t description =
        {
            prinbee::define_description(
                  prinbee::FieldName("flags")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_BITS16)
            ),
        };

        prinbee::field_t::pointer_t f(std::make_shared<prinbee::field_t>(&description));

        CATCH_REQUIRE_THROWS_MATCHES(
                  f->find_flag_definition("unknown")
                , prinbee::field_not_found
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: flag named \"unknown\" not found."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid_field: sub-structure indexing out of range")
    {
        prinbee::struct_description_t description =
        {
            prinbee::define_description(
                  prinbee::FieldName("structure")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE)
            ),
        };

        prinbee::field_t::pointer_t f(std::make_shared<prinbee::field_t>(&description));

        CATCH_REQUIRE_THROWS_MATCHES(
                  (*f)[0]
                , prinbee::out_of_bounds
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: index (0) is out of bounds since there are no sub-structures."));

        prinbee::structure::pointer_t s;
        f->sub_structures().push_back(s);

        CATCH_REQUIRE_THROWS_MATCHES(
                  (*f)[1]
                , prinbee::out_of_bounds
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: index (1) is out of bounds (0..0)"));
    }
    CATCH_END_SECTION()

#ifdef _DEBUG
    // these throw ... only happen when debug is turned on
    //
    // i.e. with `mk -t -r` command line option (Release mode),
    // it does not include debug core
    //
    CATCH_START_SECTION("structure_invalid_field: validity verifications in contructor")
    {
        prinbee::struct_description_t name_missing =
        {
            prinbee::define_description(
                  prinbee::FieldName(nullptr)
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT512)
            ),
        };

        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<prinbee::field_t>(&name_missing)
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                          "logic_error: a field must have a name, `nullptr` is not valid."));

        prinbee::struct_description_t name_empty =
        {
            prinbee::define_description(
                  prinbee::FieldName("")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT512)
            ),
        };

        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<prinbee::field_t>(&name_empty)
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                          "logic_error: a field must have a name, an empty string (\"\") is not valid."));

        for(int i(0); i < 100; ++i)
        {
            char name[10];
            name[sizeof(name) - 1] = '\0';
            for(std::size_t j(0); j < sizeof(name) - 1; ++j)
            {
                switch(rand() % 4)
                {
                case 0:
                    name[j] = rand() % 26 + 'A';
                    break;

                case 1:
                    name[j] = rand() % 26 + 'a';
                    break;

                case 2:
                    if(j == 0)
                    {
                        // avoid digits as the first character
                        //
                        name[j] = rand() % 26 + 'a';
                    }
                    else
                    {
                        name[j] = rand() % 10 + '0';
                    }
                    break;

                case 3:
                    name[j] = '_';
                    break;

                }
            }
            int k(-1);
            if(i == 0)
            {
                name[0] = rand() % 10 + '0';
            }
            else
            {
                k = rand() % (sizeof(name) - 1);
                for(;;)
                {
                    name[k] = rand() % 255;
                    if((name[k] < 'A' || name[k] > 'Z')
                    && (name[k] < 'a' || name[k] > 'z')
                    && (name[k] < '0' || name[k] > '9')
                    && name[k] != '_'
                    && name[k] != '\0')
                    {
                        break;
                    }
                }
            }

            prinbee::struct_description_t name_invalid =
            {
                prinbee::define_description(
                      prinbee::FieldName(name)
                    , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT512)
                ),
            };

            CATCH_REQUIRE_THROWS_MATCHES(
                      std::make_shared<prinbee::field_t>(&name_invalid)
                    , prinbee::logic_error
                    , Catch::Matchers::ExceptionMessage(
                              std::string("logic_error: field name \"")
                            + name
                            + "\" is not valid (unsupported characters)."));
        }
    }
    CATCH_END_SECTION()
#endif
}




CATCH_TEST_CASE("structure", "[structure] [valid]")
{
    CATCH_START_SECTION("structure: simple structure (fixed size)")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description1));

        description->init_buffer();

        description->set_uinteger("magic", static_cast<uint32_t>(prinbee::dbtype_t::BLOCK_TYPE_BLOB));

        std::uint32_t const count(123);
        description->set_uinteger("count", count);

        std::uint32_t const size(900000);
        description->set_uinteger("size", size);

        std::int32_t const change(-55);
        description->set_integer("change", change);

        prinbee::reference_t const next(0xff00ff00ff00);
        description->set_uinteger("next", next);

        prinbee::reference_t const previous(0xff11ff11ff11);
        description->set_uinteger("previous", previous);

        CATCH_REQUIRE(description->get_uinteger("magic") == static_cast<uint32_t>(prinbee::dbtype_t::BLOCK_TYPE_BLOB));
        CATCH_REQUIRE(description->get_uinteger("count") == count);
        CATCH_REQUIRE(description->get_uinteger("size") == size);
        CATCH_REQUIRE(description->get_integer("change") == change);
        CATCH_REQUIRE(description->get_uinteger("next") == next);
        CATCH_REQUIRE(description->get_uinteger("previous") == previous);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure with a string")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description2));

        description->init_buffer();

        description->set_uinteger("magic", static_cast<uint32_t>(prinbee::dbtype_t::BLOCK_TYPE_DATA));

        std::uint32_t flags(0x100105);
        description->set_uinteger("flags", flags);

        std::string const name("this is the name we want to include here");
        description->set_string("name", name);

        uint64_t size(1LL << 53);
        description->set_uinteger("size", size);

        uint16_t model(33);
        description->set_uinteger("model", model);

        CATCH_REQUIRE(description->get_uinteger("magic") == static_cast<uint32_t>(prinbee::dbtype_t::BLOCK_TYPE_DATA));
        CATCH_REQUIRE(description->get_uinteger("flags") == flags);
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_uinteger("size") == size);
        CATCH_REQUIRE(description->get_uinteger("model") == model);
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
