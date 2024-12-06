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
#include    <prinbee/data/structure.h>


// advgetopt
//
#include    <advgetopt/options.h>


// snapdev
//
#include    <snapdev/enum_class_math.h>
#include    <snapdev/not_reached.h>


// C++
//
#include    <iomanip>


// last include
//
#include    <snapdev/poison.h>



namespace
{



constexpr prinbee::struct_description_t g_description1_buffer_size_renamed[] =
{
    prinbee::define_description(
          prinbee::FieldName("size")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
};


// static size is:
//   0 + 4 + 4 + 4 + 1 + 8 + 8 = 29
//
constexpr prinbee::struct_description_t g_description1[] =
{
    prinbee::define_description(
          prinbee::FieldName("_magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_BLOB))
    ),
    prinbee::define_description(
          prinbee::FieldName("_structure_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(0, 1)
    ),
    prinbee::define_description(
          prinbee::FieldName("count")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("buffer_size")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_RENAMED)
        , prinbee::FieldSubDescription(g_description1_buffer_size_renamed)
    ),
    prinbee::define_description(
          prinbee::FieldName("size")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("change")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT8) // -100 to +100
        , prinbee::FieldVersion(3, 2, 11, 24)
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



// static size is 0 (at least because of the name which is a P8STRING)
//
constexpr prinbee::struct_description_t g_description2[] =
{
    prinbee::define_description(
          prinbee::FieldName("_magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::FILE_TYPE_BLOOM_FILTER))
    ),
    prinbee::define_description(
          prinbee::FieldName("_structure_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(15, 10231)
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



constexpr prinbee::struct_description_t g_description3_sub1[] =
{
    prinbee::define_description(
          prinbee::FieldName("major")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("minor")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("release")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("build")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::end_descriptions()
};


// static size is:
//     0 + 4 + 4 + 64 + 16 + 1 = 89
//                      ^--- sub-structure
//
constexpr prinbee::struct_description_t g_description3[] =
{
    prinbee::define_description(
          prinbee::FieldName("_magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_FREE_SPACE))
    ),
    prinbee::define_description(
          prinbee::FieldName("_structure_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(25, 312)
    ),
    prinbee::define_description(
          prinbee::FieldName("sub_field")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("data")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT512)
    ),
    prinbee::define_description(
          prinbee::FieldName("software_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE)
        , prinbee::FieldSubDescription(g_description3_sub1)
    ),
    prinbee::define_description(
          prinbee::FieldName("eight_bits=null/advance:4/efficient:2/sign")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_BITS8)
    ),
    prinbee::end_descriptions()
};



constexpr prinbee::struct_description_t g_description4a[] =
{
    prinbee::define_description(
          prinbee::FieldName("_magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::FILE_TYPE_PRIMARY_INDEX))
    ),
    prinbee::define_description(
          prinbee::FieldName("missing_structure_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_NSTIME)
    ),
    prinbee::end_descriptions()
};



constexpr prinbee::struct_description_t g_description4b[] =
{
    prinbee::define_description(
          prinbee::FieldName("_structure_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(45, 12'231)
    ),
    prinbee::define_description(
          prinbee::FieldName("missing_magic=16")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_CHAR)
    ),
    prinbee::end_descriptions()
};



constexpr prinbee::struct_description_t g_description5_sub1[] =
{
    prinbee::define_description(
          prinbee::FieldName("size")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT8)
    ),
    prinbee::define_description(
          prinbee::FieldName("parts")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_BUFFER8)
    ),
    prinbee::end_descriptions()
};


// not static (but static up to the sub-structure so that is the one
// that breaks the static-ness)
//
// dynamic size on initialization:
//  _magic                   4
//  _structure_version       4
//  sub_field                8
//  data                    16
//  early_version.size       1
//  early_version.parts      1 + 0
//  sixteen_bits             2
//  tag                     15      (the current value does not chnage this size)
//  name                     1
//                   ---------
//  total                   52
//
constexpr prinbee::struct_description_t g_description5[] =
{
    prinbee::define_description(
          prinbee::FieldName("_magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS))
    ),
    prinbee::define_description(
          prinbee::FieldName("_structure_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(405, 119)
    ),
    prinbee::define_description(
          prinbee::FieldName("sub_field")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT64)
    ),
    prinbee::define_description(
          prinbee::FieldName("data")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT128)
    ),
    // WARNING: the order here is ignored when computing the static size,
    //          instead it goes through the sorted field names, so we
    //          make sure that this one (early_version) is before any other 
    //          variable field
    //
    prinbee::define_description(
          prinbee::FieldName("early_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE)
        , prinbee::FieldSubDescription(g_description5_sub1)
    ),
    prinbee::define_description(
          prinbee::FieldName("sixteen_bits=bulk:4/more:4/raise/signal:7")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_BITS16)
    ),
    prinbee::define_description(
          prinbee::FieldName("tag=15")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_CHAR)
        , prinbee::FieldDefaultValue("image")
    ),
    prinbee::define_description(
          prinbee::FieldName("name")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P8STRING)
    ),
    prinbee::end_descriptions()
};


constexpr prinbee::struct_description_t g_description6_rename[] =
{
    prinbee::define_description(
          prinbee::FieldName("essay")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_VOID)
    ),
};


// the structure is dynamic since it includes P-strings
//
// the default size, because it has default strings is a little more
// complicated than the simple size of each field
//    _magic                 4
//    _structure_version     4
//    name                   1 +  5
//    description            2 + 62
//    essay                  4 + 98
//    tag                   15
//    renamed                0
//                        ----------
//    total                197
//
constexpr prinbee::struct_description_t g_description6[] =
{
    prinbee::define_description(
          prinbee::FieldName("_magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS))
    ),
    prinbee::define_description(
          prinbee::FieldName("_structure_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(15'345, 2'341)
    ),
    prinbee::define_description(
          prinbee::FieldName("name")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P8STRING)
        , prinbee::FieldDefaultValue("Henri")
    ),
    prinbee::define_description(
          prinbee::FieldName("description")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P16STRING)
        , prinbee::FieldDefaultValue("King who fell from a horse and had a rotting foot as a result.")
    ),
    prinbee::define_description(
          prinbee::FieldName("essay")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P32STRING)
        , prinbee::FieldDefaultValue("King who killed his wife to marry another. Later wives were lucky that the divorce was \"invented\".")
    ),
    prinbee::define_description(
          prinbee::FieldName("tag=15")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_CHAR)
        , prinbee::FieldDefaultValue("king")
    ),
    prinbee::define_description(
          prinbee::FieldName("dissertation")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_RENAMED)
        , prinbee::FieldSubDescription(g_description6_rename)
    ),
    prinbee::end_descriptions()
};





constexpr prinbee::struct_description_t g_description7_column[] =
{
    prinbee::define_description(
          prinbee::FieldName("colname")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P8STRING)
        , prinbee::FieldDefaultValue("_undefined")
    ),
    prinbee::define_description(
          prinbee::FieldName("max_size")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT16)
        , prinbee::FieldDefaultValue("256")
    ),
    prinbee::define_description(
          prinbee::FieldName("type")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT16)
        , prinbee::FieldDefaultValue("14")
    ),
    prinbee::end_descriptions()
};


// the structure is dynamic (P-strings + arrays)
//
//    _magic                 4
//    _structure_version     4
//    name                  32
//    columns                1
//    comment                4 + 43
//                        ----------
//    total                 45 + 88
//
constexpr prinbee::struct_description_t g_description7[] =
{
    prinbee::define_description(
          prinbee::FieldName("_magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS))
    ),
    prinbee::define_description(
          prinbee::FieldName("_structure_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(1, 2)
    ),
    prinbee::define_description(
          prinbee::FieldName("name=32")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_CHAR)
        , prinbee::FieldDefaultValue("users")
    ),
    prinbee::define_description(
          prinbee::FieldName("columns")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_ARRAY8)
        , prinbee::FieldSubDescription(g_description7_column)
    ),
    prinbee::define_description(
          prinbee::FieldName("comment")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P32STRING)
        , prinbee::FieldDefaultValue("This represents a form of table definition.")
    ),
    prinbee::end_descriptions()
};



constexpr prinbee::struct_description_t g_description8_column[] =
{
    prinbee::define_description(
          prinbee::FieldName("colname")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P8STRING)
        , prinbee::FieldDefaultValue("_undefined")
    ),
    prinbee::define_description(
          prinbee::FieldName("max_size")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT16)
        , prinbee::FieldDefaultValue("256")
    ),
    prinbee::define_description(
          prinbee::FieldName("type")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT16)
        , prinbee::FieldDefaultValue("14")
    ),
    prinbee::end_descriptions()
};


// the structure is dynamic (P-strings + arrays)
//
//    _magic                 4
//    _structure_version     4
//    name                   4 + 5
//    columns                2
//    comment                4 + 33
//                        ----------
//    total                 18 + 56
//
constexpr prinbee::struct_description_t g_description8[] =
{
    prinbee::define_description(
          prinbee::FieldName("_magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS))
    ),
    prinbee::define_description(
          prinbee::FieldName("_structure_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(1, 2)
    ),
    prinbee::define_description(
          prinbee::FieldName("name")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P32STRING)
        , prinbee::FieldDefaultValue("users")
    ),
    prinbee::define_description(
          prinbee::FieldName("columns")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_ARRAY16)
        , prinbee::FieldSubDescription(g_description7_column)
    ),
    prinbee::define_description(
          prinbee::FieldName("comment")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P32STRING)
        , prinbee::FieldDefaultValue("Another form of table definition.")
    ),
    prinbee::end_descriptions()
};





constexpr prinbee::struct_description_t g_description9_column[] =
{
    prinbee::define_description(
          prinbee::FieldName("colname")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P16STRING)
        , prinbee::FieldDefaultValue("_undefined")
    ),
    prinbee::define_description(
          prinbee::FieldName("max_size")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT16)
        , prinbee::FieldDefaultValue("256")
    ),
    prinbee::define_description(
          prinbee::FieldName("type")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT16)
        , prinbee::FieldDefaultValue("14")
    ),
    prinbee::end_descriptions()
};


// the structure is dynamic (P-strings + arrays)
//
//    _magic                 4
//    _structure_version     4
//    name                   1 + 4
//    columns                4
//    comment                2 + 56
//                        ----------
//    total                 15 + 75
//
constexpr prinbee::struct_description_t g_description9[] =
{
    prinbee::define_description(
          prinbee::FieldName("_magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS))
    ),
    prinbee::define_description(
          prinbee::FieldName("_structure_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(1, 2)
    ),
    prinbee::define_description(
          prinbee::FieldName("name")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P8STRING)
        , prinbee::FieldDefaultValue("page")
    ),
    prinbee::define_description(
          prinbee::FieldName("columns")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_ARRAY32)
        , prinbee::FieldSubDescription(g_description7_column)
    ),
    prinbee::define_description(
          prinbee::FieldName("comment")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P16STRING)
        , prinbee::FieldDefaultValue("Table with a number of columns that can go to 4 billion.")
    ),
    prinbee::end_descriptions()
};



constexpr prinbee::struct_description_t g_description10_buffer_size_renamed[] =
{
    prinbee::define_description(
          prinbee::FieldName("unknown")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
};


// rename has "unknown" field
//
constexpr prinbee::struct_description_t g_description10[] =
{
    prinbee::define_description(
          prinbee::FieldName("_magic")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_BLOB))
    ),
    prinbee::define_description(
          prinbee::FieldName("_structure_version")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(0, 1)
    ),
    prinbee::define_description(
          prinbee::FieldName("buffer_size")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_RENAMED)
        , prinbee::FieldSubDescription(g_description10_buffer_size_renamed)
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
    { prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION, true },
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
    { prinbee::struct_type_t::STRUCT_TYPE_NSTIME, true },
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


CATCH_TEST_CASE("structure_type_name", "[structure][type][valid]")
{
    CATCH_START_SECTION("structure_type_name: name from type")
    {
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_END) == "END");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_VOID) == "VOID");
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION) == "STRUCTURE_VERSION");
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
        CATCH_REQUIRE(prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_NSTIME) == "NSTIME");
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
        CATCH_REQUIRE(prinbee::name_to_struct_type("STRUCTURE_VERSION") == prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION);
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
        CATCH_REQUIRE(prinbee::name_to_struct_type("NSTIME") == prinbee::struct_type_t::STRUCT_TYPE_NSTIME);
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


CATCH_TEST_CASE("structure_type_name_invalid", "[structure][type][invalid]")
{
    CATCH_START_SECTION("structure_type_name_invalid: unknown")
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

    CATCH_START_SECTION("structure_type_name_invalid: invalid")
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


CATCH_TEST_CASE("structure_type_metadata", "[structure][type][valid]")
{
    CATCH_START_SECTION("structure_type_metadata: fixed size")
    {
        for(auto const & f : g_fixed_sizes)
        {
            if(f.f_fixed != prinbee::type_with_fixed_size(f.f_type))
            {
                SNAP_LOG_FATAL
                    << "type "
                    << prinbee::to_string(f.f_type)
                    << (f.f_fixed ? " was" : " is not")
                    << " expected to be fixed."
                    << SNAP_LOG_SEND;
            }
            CATCH_REQUIRE(f.f_fixed == prinbee::type_with_fixed_size(f.f_type));
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_version_basics", "[structure][version]")
{
    CATCH_START_SECTION("structure_version_basics: default version")
    {
        prinbee::version_t version;
        CATCH_REQUIRE(version.get_major() == 0);
        CATCH_REQUIRE(version.get_minor() == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_version_basics: version conversions")
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


CATCH_TEST_CASE("structure_version_compare", "[structure][version]")
{
    CATCH_START_SECTION("structure_version_compare: compare")
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


CATCH_TEST_CASE("structure_version_overflow", "[structure][version][invalid]")
{
    CATCH_START_SECTION("structure_version_overflow: version overflow")
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


CATCH_TEST_CASE("structure_min_max_version", "[structure][version][valid]")
{
    CATCH_START_SECTION("structure_min_max_version: default")
    {
        prinbee::version_t version = prinbee::version_t();
        prinbee::min_max_version_t full_range;
        CATCH_REQUIRE(version == full_range.min());
        CATCH_REQUIRE(prinbee::max_version() == full_range.max());
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_flag_definitions", "[structure][flags][valid]")
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


CATCH_TEST_CASE("structure_flag_definitions_incorrect_construction", "[structure][flags][invalid]")
{
    CATCH_START_SECTION("structure_flag_definitions_incorrect_construction: missing name(s)")
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

    CATCH_START_SECTION("structure_flag_definitions_incorrect_construction: unsupported sizes / positions")
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


CATCH_TEST_CASE("structure_field", "[structure][valid]")
{
    CATCH_START_SECTION("structure_field: check description of all different database types")
    {
        for(auto const & info : g_field_info)
        {
            prinbee::field_t::field_flags_t flags((rand() & 1) != 0 ? 0 : prinbee::STRUCT_DESCRIPTION_FLAG_OPTIONAL);
            prinbee::struct_type_t const type(prinbee::name_to_struct_type(info.f_type_name));
            std::shared_ptr<prinbee::struct_description_t> description;
            std::string name(info.f_type_name);
            switch(type)
            {
            case prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE:
            case prinbee::struct_type_t::STRUCT_TYPE_ARRAY8:
            case prinbee::struct_type_t::STRUCT_TYPE_ARRAY16:
            case prinbee::struct_type_t::STRUCT_TYPE_ARRAY32:
            case prinbee::struct_type_t::STRUCT_TYPE_RENAMED:
                description.reset(new prinbee::struct_description_t{
                        prinbee::define_description(
                              prinbee::FieldName(name.c_str())
                            , prinbee::FieldType(type)
                            , prinbee::FieldFlags(flags)
                            , prinbee::FieldSubDescription(g_description3)
                        ),
                    });
                break;

            case prinbee::struct_type_t::STRUCT_TYPE_BITS8:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS16:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS32:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS64:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS128:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS256:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS512:
                name += "=flag";
                description.reset(new prinbee::struct_description_t{
                        prinbee::define_description(
                              prinbee::FieldName(name.c_str())
                            , prinbee::FieldType(type)
                            , prinbee::FieldFlags(flags)
                        ),
                    });
                break;

            default:
                description.reset(new prinbee::struct_description_t{
                        prinbee::define_description(
                              prinbee::FieldName(name.c_str())
                            , prinbee::FieldType(type)
                            , prinbee::FieldFlags(flags)
                        ),
                    });
                break;

            }

            prinbee::field_t::const_pointer_t f(std::make_shared<prinbee::field_t>(description.get()));

            CATCH_REQUIRE(description.get() == f->description());
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
                  prinbee::FieldName("flags=big:60/small:3/tiny")
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wrestrict"
            std::string const name("f" + std::to_string(i));
#pragma GCC diagnostic pop
            prinbee::flag_definition::pointer_t flag(std::make_shared<prinbee::flag_definition>("flags", name, i * 3, 3));
            CATCH_REQUIRE(flag->full_name() == "flags." + name);
            f->add_flag_definition(flag);
            CATCH_REQUIRE(flag == f->find_flag_definition(name));
        }

        // make sure they stay around
        //
        for(int i(1); i <= 10; ++i)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wrestrict"
            std::string const name("f" + std::to_string(i));
#pragma GCC diagnostic pop
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
                  prinbee::FieldName("tail=fish:10/fin:2/gill/bones:3")
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
                  prinbee::FieldName("tail=mask:13/size:3")
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
        prinbee::struct_description_t description[] =
        {
            prinbee::define_description(
                  prinbee::FieldName("structure")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE)
                , prinbee::FieldSubDescription(g_description1)
            ),
        };

        prinbee::field_t::pointer_t f(std::make_shared<prinbee::field_t>(description));

        prinbee::structure::pointer_t s(std::make_shared<prinbee::structure>(g_description1));
        CATCH_REQUIRE(s->get_static_size() == 33);
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


CATCH_TEST_CASE("structure_invalid_field", "[structure][valid]")
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

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::define_description(
                        prinbee::FieldName("INVALID")
                      , prinbee::FieldType(bad_type)
                  )
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage("prinbee_exception: the specified structure field type was not recognized."));

        prinbee::struct_description_t description =
        {
            .f_field_name =          "INVALID",
            .f_type =                bad_type,
            .f_flags =               0,
            .f_default_value =       nullptr,
            .f_min_version =         prinbee::version_t(),
            .f_max_version =         prinbee::version_t(),
            .f_sub_description =     nullptr,
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
                          ", max: 47).")); // this number is not defined otherwise...

        CATCH_REQUIRE_THROWS_MATCHES(
                  f->type_field_size()
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                          "out_of_range: type out of range for converting it to a type field size ("
                          "*unknown struct type ("
                        + std::to_string(static_cast<int>(bad_type))
                        + ")*"
                          ", max: 47).")); // this number is not defined otherwise...
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid_field: new name without a RENAMED")
    {
        prinbee::struct_description_t description =
        {
            prinbee::define_description(
                  prinbee::FieldName("not_renamed")
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_BUFFER8) // <- wrong type
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
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::define_description(
                        prinbee::FieldName("no_link")
                      , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_RENAMED)
                  )
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: this structure field must have a sub-description field."));

        // in case someone creates a description manually, make sure we also
        // catch that error
        //
        prinbee::struct_description_t description =
        {
            .f_field_name =          "no_link",
            .f_type =                prinbee::struct_type_t::STRUCT_TYPE_RENAMED,
            .f_flags =               0,
            .f_default_value =       nullptr,
            .f_min_version =         prinbee::version_t(),
            .f_max_version =         prinbee::version_t(),
            .f_sub_description =     nullptr,
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
                , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_END) // the type is not checked with the RENAMED although it probably should be!
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
                  prinbee::FieldName("flags=some_flag/another/foo/bar")
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
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::define_description(
                        prinbee::FieldName("structure")
                      , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE)
                  )
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: this structure field must have a sub-description field."));

        prinbee::struct_description_t description =
        {
            .f_field_name =          "structure",
            .f_type =                prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE,
            .f_flags =               0,
            .f_default_value =       nullptr,
            .f_min_version =         prinbee::version_t(),
            .f_max_version =         prinbee::version_t(),
            .f_sub_description =     nullptr,
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

    CATCH_START_SECTION("structure_invalid_field: field does not support a sub-structure")
    {
        for(prinbee::struct_type_t type(prinbee::struct_type_t::STRUCT_TYPE_END);
            type <= prinbee::struct_type_t::STRUCT_TYPE_RENAMED;
            ++type)
        {
            switch(type)
            {
            case prinbee::struct_type_t::STRUCT_TYPE_MAGIC:
                CATCH_REQUIRE_THROWS_MATCHES(
                          prinbee::define_description(
                                prinbee::FieldName("_magic")
                              , prinbee::FieldType(type)
                              , prinbee::FieldSubDescription(g_description1)
                          )
                        , prinbee::invalid_parameter
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: this structure field cannot have a sub-description field."));
                break;

            case prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION:
                CATCH_REQUIRE_THROWS_MATCHES(
                          prinbee::define_description(
                                prinbee::FieldName("_structure_version")
                              , prinbee::FieldType(type)
                              , prinbee::FieldSubDescription(g_description1)
                          )
                        , prinbee::invalid_parameter
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: this structure field cannot have a sub-description field."));
                break;

            case prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE:
            case prinbee::struct_type_t::STRUCT_TYPE_ARRAY8:
            case prinbee::struct_type_t::STRUCT_TYPE_ARRAY16:
            case prinbee::struct_type_t::STRUCT_TYPE_ARRAY32:
            case prinbee::struct_type_t::STRUCT_TYPE_RENAMED:
                break;

            case prinbee::struct_type_t::STRUCT_TYPE_BITS8:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS16:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS32:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS64:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS128:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS256:
            case prinbee::struct_type_t::STRUCT_TYPE_BITS512:
                CATCH_REQUIRE_THROWS_MATCHES(
                          prinbee::define_description(
                                prinbee::FieldName("field=bits:3")
                              , prinbee::FieldType(type)
                              , prinbee::FieldSubDescription(g_description1)
                          )
                        , prinbee::invalid_parameter
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: this structure field cannot have a sub-description field."));
                break;

            case prinbee::struct_type_t::STRUCT_TYPE_CHAR:
                CATCH_REQUIRE_THROWS_MATCHES(
                          prinbee::define_description(
                                prinbee::FieldName("field=1001")
                              , prinbee::FieldType(type)
                              , prinbee::FieldSubDescription(g_description1)
                          )
                        , prinbee::invalid_parameter
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: this structure field cannot have a sub-description field."));
                break;

            default:
                CATCH_REQUIRE_THROWS_MATCHES(
                          prinbee::define_description(
                                prinbee::FieldName(type == prinbee::struct_type_t::STRUCT_TYPE_END ? nullptr : "field")
                              , prinbee::FieldType(type)
                              , prinbee::FieldSubDescription(g_description1)
                          )
                        , prinbee::invalid_parameter
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: this structure field cannot have a sub-description field."));
                break;

            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid_field: name field")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::define_description(
                        prinbee::FieldName("unwanted_name")
                      , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_END)
                  )
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: the END structure field cannot have a field name."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid_field: name field")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::define_description(
                    prinbee::FieldName("inverted_version")
                  , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT32)
                  , prinbee::FieldVersion(15, 255, 15, 254)
              )
            , prinbee::invalid_parameter
            , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: this structure field must have a minimum version which is smaller or equal to the maximum version."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid_field: structure version missing or invalid")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::define_description(
                    prinbee::FieldName("_structure_version")
                  , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
              )
            , prinbee::invalid_parameter
            , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: this structure field must have a version."));

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::define_description(
                    prinbee::FieldName("_structure_version")
                  , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
                  , prinbee::FieldVersion(0, 0, 65535, 65535) // explicit (same as unspecified); invalid min.
              )
            , prinbee::invalid_parameter
            , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: this structure field must have a version."));

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::define_description(
                    prinbee::FieldName("_structure_version")
                  , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
                  , prinbee::FieldVersion(3, 7, 123, 456) // invalid max.
              )
            , prinbee::invalid_parameter
            , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: this structure field must have a version."));

        CATCH_REQUIRE_THROWS_MATCHES(
              prinbee::define_description(
                    prinbee::FieldName("_structure_version")
                  , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
                  , prinbee::FieldVersion(0, 0, 123, 456) // invalid min. & max.
              )
            , prinbee::invalid_parameter
            , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: this structure field must have a version."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid_field: field validity verifications in define_description() [compile time if defined constexpr]")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::define_description(
                        prinbee::FieldName(nullptr)
                      , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT512)
                  )
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: this structure field must have a field name."));

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::define_description(
                        prinbee::FieldName("")
                      , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT512)
                  )
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: this structure field name is not considered valid."));

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

            CATCH_REQUIRE_THROWS_MATCHES(
                      prinbee::define_description(
                            prinbee::FieldName(name)
                          , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_INT512)
                      )
                    , prinbee::invalid_parameter
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: this structure field name is not considered valid."));
        }
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
        {
            prinbee::struct_description_t name_missing =
            {
                .f_field_name =          nullptr,
                .f_type =                prinbee::struct_type_t::STRUCT_TYPE_INT512,
                .f_flags =               0,
                .f_default_value =       nullptr,
                .f_min_version =         prinbee::version_t(),
                .f_max_version =         prinbee::version_t(),
                .f_sub_description =     nullptr,
            };

            CATCH_REQUIRE_THROWS_MATCHES(
                      std::make_shared<prinbee::field_t>(&name_missing)
                    , prinbee::logic_error
                    , Catch::Matchers::ExceptionMessage(
                              "logic_error: a field must have a name, null is not valid."));
        }

        {
            prinbee::struct_description_t empty_name =
            {
                .f_field_name =          "",
                .f_type =                prinbee::struct_type_t::STRUCT_TYPE_INT512,
                .f_flags =               0,
                .f_default_value =       nullptr,
                .f_min_version =         prinbee::version_t(),
                .f_max_version =         prinbee::version_t(),
                .f_sub_description =     nullptr,
            };

            CATCH_REQUIRE_THROWS_MATCHES(
                      std::make_shared<prinbee::field_t>(&empty_name)
                    , prinbee::logic_error
                    , Catch::Matchers::ExceptionMessage(
                              "logic_error: a field must have a name, an empty string (\"\") is not valid."));
        }

        {
            prinbee::struct_description_t missing_flags =
            {
                .f_field_name =          "foo",
                .f_type =                prinbee::struct_type_t::STRUCT_TYPE_BITS8,
                .f_flags =               0,
                .f_default_value =       nullptr,
                .f_min_version =         prinbee::version_t(),
                .f_max_version =         prinbee::version_t(),
                .f_sub_description =     nullptr,
            };

            CATCH_REQUIRE_THROWS_MATCHES(
                      std::make_shared<prinbee::field_t>(&missing_flags)
                    , prinbee::logic_error
                    , Catch::Matchers::ExceptionMessage(
                              "logic_error: bit field name & definition \"foo\" are not valid."));
        }

        {
            prinbee::struct_description_t bad_name =
            {
                .f_field_name =          "3_bad_names",
                .f_type =                prinbee::struct_type_t::STRUCT_TYPE_INT32,
                .f_flags =               0,
                .f_default_value =       nullptr,
                .f_min_version =         prinbee::version_t(),
                .f_max_version =         prinbee::version_t(),
                .f_sub_description =     nullptr,
            };

            CATCH_REQUIRE_THROWS_MATCHES(
                      std::make_shared<prinbee::field_t>(&bad_name)
                    , prinbee::logic_error
                    , Catch::Matchers::ExceptionMessage(
                              "logic_error: field name \"3_bad_names\" is not valid (unsupported characters)."));
        }
    }
    CATCH_END_SECTION()
#endif
}


CATCH_TEST_CASE("structure", "[structure][valid]")
{
    CATCH_START_SECTION("structure: simple structure (fixed size)")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description1));

        description->init_buffer();

        std::uint32_t const count(123);
        description->set_uinteger("count", count);

        std::uint32_t const size(900'000);
        description->set_uinteger("size", size);

        std::int32_t const change(-55);
        description->set_integer("change", change);

        prinbee::reference_t const next(0xff00ff00ff00);
        description->set_uinteger("next", next);

        prinbee::reference_t const previous(0xff11ff11ff11);
        description->set_uinteger("previous", previous);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_BLOB);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(0, 1));
        CATCH_REQUIRE(description->get_uinteger("count") == count);
        CATCH_REQUIRE(description->get_uinteger("size") == size);
        CATCH_REQUIRE(description->get_integer("change") == change);
        CATCH_REQUIRE(description->get_uinteger("next") == next);
        CATCH_REQUIRE(description->get_uinteger("previous") == previous);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure: structure with a string")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description2));

        CATCH_REQUIRE(description->get_static_size() == 0);

        description->init_buffer();

        //description->set_uinteger("_magic", static_cast<uint32_t>(prinbee::dbtype_t::BLOCK_TYPE_DATA));

        std::uint32_t flags(0x100105);
        description->set_uinteger("flags", flags);

        std::string const name("this is the name we want to include here");
        description->set_string("name", name);

        uint64_t size(1LL << 53);
        description->set_uinteger("size", size);

        uint16_t model(33);
        description->set_uinteger("model", model);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::FILE_TYPE_BLOOM_FILTER);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(15, 10231));
        CATCH_REQUIRE(description->get_uinteger("flags") == flags);
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_uinteger("size") == size);
        CATCH_REQUIRE(description->get_uinteger("model") == model);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure: structure with a bit field")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description3));

        CATCH_REQUIRE(description->get_static_size() == 93);

        description->init_buffer();

        //description->set_uinteger("magic", static_cast<std::uint32_t>(prinbee::dbtype_t::BLOCK_TYPE_DATA));

        std::uint32_t sub_field(SNAP_CATCH2_NAMESPACE::rand32());
        description->set_uinteger("sub_field", sub_field);

        prinbee::int512_t large_number;
        SNAP_CATCH2_NAMESPACE::rand512(large_number);
        description->set_large_integer("data", large_number);

        std::uint32_t major(SNAP_CATCH2_NAMESPACE::rand32());
        std::uint32_t minor(SNAP_CATCH2_NAMESPACE::rand32());
        std::uint32_t release(SNAP_CATCH2_NAMESPACE::rand32());
        std::uint32_t build(SNAP_CATCH2_NAMESPACE::rand32());
        description->set_uinteger("software_version.major", major);
        description->set_uinteger("software_version.minor", minor);
        description->set_uinteger("software_version.release", release);
        description->set_uinteger("software_version.build", build);

        std::uint32_t const null_value(rand() & 1);
        description->set_bits("eight_bits.null", null_value);

        std::uint32_t const advance_value(rand() & 15);
        description->set_bits("eight_bits.advance", advance_value);

        std::uint32_t const efficient_value(rand() & 3);
        description->set_bits("eight_bits.efficient", efficient_value);

        std::uint32_t const sign_value(rand() & 1);
        description->set_bits("eight_bits.sign", sign_value);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_FREE_SPACE);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(25, 312));
        CATCH_REQUIRE(description->get_uinteger("_structure_version") == prinbee::version_t(25, 312));
        CATCH_REQUIRE(description->get_uinteger("sub_field") == sub_field);
        CATCH_REQUIRE(description->get_large_integer("data") == large_number);
        CATCH_REQUIRE(description->get_uinteger("software_version.major") == major);
        CATCH_REQUIRE(description->get_uinteger("software_version.minor") == minor);
        CATCH_REQUIRE(description->get_uinteger("software_version.release") == release);
        CATCH_REQUIRE(description->get_uinteger("software_version.build") == build);
        CATCH_REQUIRE(description->get_bits("eight_bits.null") == null_value);
        CATCH_REQUIRE(description->get_bits("eight_bits.advance") == advance_value);
        CATCH_REQUIRE(description->get_bits("eight_bits.efficient") == efficient_value);
        CATCH_REQUIRE(description->get_bits("eight_bits.sign") == sign_value);

        // the get_field() allows you to search for a specific flag in a field and
        // you get the field pointer--not too sure that makes sense, but it works...
        //
        prinbee::field_t::pointer_t eight_bits(description->get_field("eight_bits"));
        CATCH_REQUIRE(description->get_field("eight_bits.null") == eight_bits);
        CATCH_REQUIRE(description->get_field("eight_bits.advance") == eight_bits);
        CATCH_REQUIRE(description->get_field("eight_bits.efficient") == eight_bits);
        CATCH_REQUIRE(description->get_field("eight_bits.sign") == eight_bits);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure: structure with a variable sub-structure")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description5));

        // uninitialized structures have no buffer
        //
        CATCH_REQUIRE_THROWS_MATCHES(
                  description->get_field("name")
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                    "logic_error: trying to access a structure field when the f_buffer pointer is still null."));

        // the BUFFER8 in the sub-structure makes this structure dynamic
        // (and later the P8STRING too)
        //
        CATCH_REQUIRE(description->get_static_size() == 0UL);
        CATCH_REQUIRE(description->get_current_size() == 52UL);

        description->init_buffer();

        // to change a VERSION type, we use the set_uinteger() function
        // but we must make sure that the structure version cannot be
        // updated (this is a read-only field)
        //
        CATCH_REQUIRE_THROWS_MATCHES(
                  description->set_uinteger("_structure_version", SNAP_CATCH2_NAMESPACE::rand32())
                , prinbee::type_mismatch
                , Catch::Matchers::ExceptionMessage(
                    "prinbee_exception: this description type is \"STRUCTURE_VERSION\""
                    " but we expected one of \"BITS8, BITS16, BITS32, BITS64, OID, REFERENCE, UINT8, UINT16, UINT32, UINT64, VERSION\"."));

        std::int64_t const sub_field(SNAP_CATCH2_NAMESPACE::rand64());
        description->set_integer("sub_field", sub_field);
        CATCH_REQUIRE(description->get_current_size() == 52UL);

        prinbee::int512_t large_number;
        SNAP_CATCH2_NAMESPACE::rand512(large_number);
        if((large_number.f_value[1] & (1UL << 63)) == 0)
        {
            large_number.f_value[2] = 0;
            large_number.f_value[3] = 0;
            large_number.f_value[4] = 0;
            large_number.f_value[5] = 0;
            large_number.f_value[6] = 0;
            large_number.f_high_value = 0;
        }
        else
        {
            large_number.f_value[2] = -1;
            large_number.f_value[3] = -1;
            large_number.f_value[4] = -1;
            large_number.f_value[5] = -1;
            large_number.f_value[6] = -1;
            large_number.f_high_value = -1;
        }
        description->set_large_integer("data", large_number);
        CATCH_REQUIRE(description->get_current_size() == 52UL);

        std::uint8_t const version_size((rand() & 7) + 1);
        prinbee::buffer_t version_parts(version_size);
        for(std::uint8_t idx(0); idx < version_size; ++idx)
        {
            version_parts[idx] = rand();
        }
        description->set_uinteger("early_version.size", version_size);
        CATCH_REQUIRE(description->get_current_size() == 52UL);
        description->set_buffer("early_version.parts", version_parts);
        CATCH_REQUIRE(description->get_current_size() == 52UL + version_size);

        std::uint32_t const bulk_value(rand() & 15);
        description->set_bits("sixteen_bits.bulk", bulk_value);
        CATCH_REQUIRE(description->get_current_size() == 52UL + version_size);

        std::uint32_t const more_value(rand() & 15);
        description->set_bits("sixteen_bits.more", more_value);
        CATCH_REQUIRE(description->get_current_size() == 52UL + version_size);

        std::uint32_t const raise_value(rand() & 1);
        description->set_bits("sixteen_bits.raise", raise_value);
        CATCH_REQUIRE(description->get_current_size() == 52UL + version_size);

        std::uint32_t const signal_value(rand() & 127);
        description->set_bits("sixteen_bits.signal", signal_value);
        CATCH_REQUIRE(description->get_current_size() == 52UL + version_size);

        //description->set_string("tag", ...); -- keep the default value instead

        std::string const name(SNAP_CATCH2_NAMESPACE::random_string(1, 255));
        description->set_string("name", name);
        CATCH_REQUIRE(description->get_current_size() == 52UL + version_size + name.length());

        // field must be given a name
        //
        CATCH_REQUIRE_THROWS_MATCHES(
                  description->get_uinteger(std::string())
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage(
                    "logic_error: called structure::get_field() with an empty field name."));

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(405, 119));
        CATCH_REQUIRE(description->get_integer("sub_field") == sub_field);
        CATCH_REQUIRE(description->get_large_integer("data") == large_number);
        CATCH_REQUIRE(description->get_uinteger("early_version.size") == version_size);
        CATCH_REQUIRE(description->get_buffer("early_version.parts") == version_parts);
        CATCH_REQUIRE(description->get_bits("sixteen_bits.bulk") == bulk_value);
        CATCH_REQUIRE(description->get_bits("sixteen_bits.more") == more_value);
        CATCH_REQUIRE(description->get_bits("sixteen_bits.raise") == raise_value);
        CATCH_REQUIRE(description->get_bits("sixteen_bits.signal") == signal_value);
        CATCH_REQUIRE(description->get_string("tag") == "image"); // from the default value
        CATCH_REQUIRE(description->get_string("name") == name);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure: structure with four types of strings")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description6));

        // the strings make this structure dynamic
        //
        CATCH_REQUIRE(description->get_static_size() == 0UL);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 1UL + 0UL + 2UL + 0UL + 4UL + 0UL + 15UL);

        description->init_buffer();
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 1UL + 5UL + 2UL + 62UL + 4UL + 98UL + 15UL);

        CATCH_REQUIRE_THROWS_MATCHES(
                  description->set_uinteger("_magic", SNAP_CATCH2_NAMESPACE::rand32())
                , prinbee::type_mismatch
                , Catch::Matchers::ExceptionMessage(
                    "prinbee_exception: this description type is \"MAGIC\""
                    " but we expected one of \"BITS8, BITS16, BITS32, BITS64, OID, REFERENCE, UINT8, UINT16, UINT32, UINT64, VERSION\"."));

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(15'345, 2'341));

        CATCH_REQUIRE(description->get_string("name") == "Henri");
        std::string const name(SNAP_CATCH2_NAMESPACE::random_string(1, 255));
        description->set_string("name", name);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 1UL + name.length() + 2UL + 62UL + 4UL + 98UL + 15UL);
        CATCH_REQUIRE(description->get_string("name") == name);

        CATCH_REQUIRE(description->get_string("description") == "King who fell from a horse and had a rotting foot as a result.");
        std::string const description_field(SNAP_CATCH2_NAMESPACE::random_string(256, 3'000));
        description->set_string("description", description_field);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 1UL + name.length() + 2UL + description_field.length() + 4UL + 98UL + 15UL);
        CATCH_REQUIRE(description->get_string("description") == description_field);

        CATCH_REQUIRE(description->get_string("essay") == "King who killed his wife to marry another. Later wives were lucky that the divorce was \"invented\".");
        CATCH_REQUIRE(description->get_string("dissertation") == "King who killed his wife to marry another. Later wives were lucky that the divorce was \"invented\".");
        std::string const essay(SNAP_CATCH2_NAMESPACE::random_string(1'000, 250'000));
SNAP_LOG_WARNING << "--- random essay length is " << essay.length() << SNAP_LOG_SEND;
        description->set_string("essay", essay);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 1UL + name.length() + 2UL + description_field.length() + 4UL + essay.length() + 15UL);
        CATCH_REQUIRE(description->get_string("essay") == essay);
        CATCH_REQUIRE(description->get_string("dissertation") == essay);

        std::string const dissertation(SNAP_CATCH2_NAMESPACE::random_string(1'000, 250'000));
SNAP_LOG_WARNING << "--- random dissertation length is " << dissertation.length() << SNAP_LOG_SEND;
        description->set_string("dissertation", dissertation);
SNAP_LOG_WARNING << "--- dissertation set_string() returned... verify" << SNAP_LOG_SEND;
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 1UL + name.length() + 2UL + description_field.length() + 4UL + dissertation.length() + 15UL);
        CATCH_REQUIRE(description->get_string("dissertation") == dissertation);
        CATCH_REQUIRE(description->get_string("essay") == dissertation);

SNAP_LOG_WARNING << "--- now do a set of the kingdom..." << SNAP_LOG_SEND;
        description->set_string("tag", "kingdom");
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 1UL + name.length() + 2UL + description_field.length() + 4UL + dissertation.length() + 15UL);
        CATCH_REQUIRE(description->get_string("tag") == "kingdom");
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_get_set", "[structure][valid]")
{
    CATCH_START_SECTION("structure: structure get/set functions")
    {
        typedef std::uint32_t           valid_func_t;

        constexpr valid_func_t          VALID_FUNC_FLAG             = 0x0001;
        constexpr valid_func_t          VALID_FUNC_BITS             = 0x0002;
        constexpr valid_func_t          VALID_FUNC_INTEGER          = 0x0004;
        constexpr valid_func_t          VALID_FUNC_UINTEGER         = 0x0008;
        constexpr valid_func_t          VALID_FUNC_LARGE_INTEGER    = 0x0010;
        constexpr valid_func_t          VALID_FUNC_LARGE_UINTEGER   = 0x0020;
        constexpr valid_func_t          VALID_FUNC_FLOAT32          = 0x0040;
        constexpr valid_func_t          VALID_FUNC_FLOAT64          = 0x0080;
        constexpr valid_func_t          VALID_FUNC_FLOAT128         = 0x0100;

        struct type_to_test_t
        {
            valid_func_t                f_valid_func = 0;
            prinbee::struct_type_t      f_type = prinbee::struct_type_t::STRUCT_TYPE_VOID;
            std::uint16_t               f_mask_size = 512;
        };

        type_to_test_t const type_to_test[] =
        {
            {
                .f_valid_func = 0,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_VOID,
                .f_mask_size = 512,
            },
            {
                .f_valid_func = VALID_FUNC_FLAG | VALID_FUNC_BITS | VALID_FUNC_UINTEGER | VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_BITS8,
                .f_mask_size = 8,
            },
            {
                .f_valid_func = VALID_FUNC_FLAG | VALID_FUNC_BITS | VALID_FUNC_UINTEGER | VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_BITS16,
                .f_mask_size = 16,
            },
            {
                .f_valid_func = VALID_FUNC_FLAG | VALID_FUNC_BITS | VALID_FUNC_UINTEGER | VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_BITS32,
                .f_mask_size = 32,
            },
            {
                .f_valid_func = VALID_FUNC_FLAG | VALID_FUNC_BITS | VALID_FUNC_UINTEGER | VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_BITS64,
                .f_mask_size = 64,
            },
            {
                .f_valid_func = VALID_FUNC_FLAG | VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_BITS128,
                .f_mask_size = 128,
            },
            {
                .f_valid_func = VALID_FUNC_FLAG | VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_BITS256,
                .f_mask_size = 256,
            },
            {
                .f_valid_func = VALID_FUNC_FLAG | VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_BITS512,
                .f_mask_size = 512,
            },
            {
                .f_valid_func = VALID_FUNC_INTEGER | VALID_FUNC_LARGE_INTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_INT8,
                .f_mask_size = 8,
            },
            {
                .f_valid_func = VALID_FUNC_UINTEGER | VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_UINT8,
                .f_mask_size = 8,
            },
            {
                .f_valid_func = VALID_FUNC_INTEGER | VALID_FUNC_LARGE_INTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_INT16,
                .f_mask_size = 16,
            },
            {
                .f_valid_func = VALID_FUNC_UINTEGER | VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_UINT16,
                .f_mask_size = 16,
            },
            {
                .f_valid_func = VALID_FUNC_INTEGER | VALID_FUNC_LARGE_INTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_INT32,
                .f_mask_size = 32,
            },
            {
                .f_valid_func = VALID_FUNC_UINTEGER | VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_UINT32,
                .f_mask_size = 32,
            },
            {
                .f_valid_func = VALID_FUNC_INTEGER | VALID_FUNC_LARGE_INTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_INT64,
                .f_mask_size = 64,
            },
            {
                .f_valid_func = VALID_FUNC_UINTEGER | VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_UINT64,
                .f_mask_size = 64,
            },
            {
                .f_valid_func = VALID_FUNC_LARGE_INTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_INT128,
                .f_mask_size = 128,
            },
            {
                .f_valid_func = VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_UINT128,
                .f_mask_size = 128,
            },
            {
                .f_valid_func = VALID_FUNC_LARGE_INTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_INT256,
                .f_mask_size = 256,
            },
            {
                .f_valid_func = VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_UINT256,
                .f_mask_size = 256,
            },
            {
                .f_valid_func = VALID_FUNC_LARGE_INTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_INT512,
                .f_mask_size = 512,
            },
            {
                .f_valid_func = VALID_FUNC_LARGE_UINTEGER,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_UINT512,
                .f_mask_size = 512,
            },
            {
                .f_valid_func = VALID_FUNC_FLOAT32,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_FLOAT32,
                .f_mask_size = 32,
            },
            {
                .f_valid_func = VALID_FUNC_FLOAT64,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_FLOAT64,
                .f_mask_size = 64,
            },
            {
                .f_valid_func = VALID_FUNC_FLOAT128,
                .f_type = prinbee::struct_type_t::STRUCT_TYPE_FLOAT128,
                .f_mask_size = 128,
            },
        };
//    STRUCT_TYPE_MAGIC,              // CHAR=4
//    STRUCT_TYPE_STRUCTURE_VERSION,  // UINT16:UINT16 (Major:Minor) -- version of the structure.cpp/h description
//    STRUCT_TYPE_VERSION,            // UINT16:UINT16 (Major:Minor)
//
//    STRUCT_TYPE_REFERENCE,          // UINT64 to another location in the file (offset 0 is start of file)
//    STRUCT_TYPE_OID,                // UINT64 similar to a REFERENCE, but points to the TIND/INDR blocks (sizeof(OID) == sizeof(REFERENCE) must be true)

        for(auto const & t : type_to_test)
        {
            //prinbee::int512_t integer;
            //SNAP_CATCH2_NAMESPACE::rand512(integer);
            //std::string string(SNAP_CATCH2_NAMESPACE::random_string(1, 255));
            //long double floating_point(drand48() * 1000.0L);

            char const * field_name = "test_field";
            if((t.f_valid_func & VALID_FUNC_FLAG) != 0)
            {
                field_name = "test_field=on/color:3/valid/side:2";
            }

SNAP_LOG_WARNING << "--- testing [" << prinbee::to_string(t.f_type)
<< "] - " << t.f_valid_func
<< " -> " << field_name
<< SNAP_LOG_SEND;
            prinbee::struct_description_t field_descriptions[] =
            {
                {
                    .f_field_name = "_magic",
                    .f_type = prinbee::struct_type_t::STRUCT_TYPE_MAGIC,
                    .f_flags = 0,
                    .f_default_value = prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_BLOB),
                    .f_min_version = prinbee::version_t(),
                    .f_max_version = prinbee::max_version(),
                    .f_sub_description = nullptr,
                },
                {
                    .f_field_name = "_structure_version",
                    .f_type = prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION,
                    .f_flags = 0,
                    .f_default_value = nullptr,
                    .f_min_version = prinbee::version_t(3, 7),
                    .f_max_version = prinbee::max_version(),
                    .f_sub_description = nullptr,
                },
                {
                    .f_field_name = field_name,
                    .f_type = t.f_type,
                    .f_flags = 0,
                    .f_default_value = nullptr,
                    .f_min_version = prinbee::version_t(),
                    .f_max_version = prinbee::max_version(),
                    .f_sub_description = nullptr,
                },
                {
                    .f_field_name = nullptr,
                    .f_type = prinbee::struct_type_t::STRUCT_TYPE_END,
                    .f_flags = 0,
                    .f_default_value = nullptr,
                    .f_min_version = prinbee::version_t(),
                    .f_max_version = prinbee::version_t(),
                    .f_sub_description = nullptr,
                },
            };

            prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(field_descriptions));
            description->init_buffer();

            CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_BLOB);
            CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(3, 7));

            // get_flag() function
            //
            prinbee::field_t::pointer_t f;
            if((t.f_valid_func & VALID_FUNC_FLAG) != 0)
            {
                prinbee::flag_definition::pointer_t flag(description->get_flag("test_field.on", f));
                CATCH_REQUIRE(flag->full_name() == "test_field.on");
                CATCH_REQUIRE(flag->field_name() == "test_field");
                CATCH_REQUIRE(flag->flag_name() == "on");
                CATCH_REQUIRE(flag->pos() == 0);
                CATCH_REQUIRE(flag->size() == 1);
                CATCH_REQUIRE(flag->mask() == 1);

                flag = description->get_flag("test_field.color", f);
                CATCH_REQUIRE(flag->full_name() == "test_field.color");
                CATCH_REQUIRE(flag->field_name() == "test_field");
                CATCH_REQUIRE(flag->flag_name() == "color");
                CATCH_REQUIRE(flag->pos() == 1);
                CATCH_REQUIRE(flag->size() == 3);
                CATCH_REQUIRE(flag->mask() == 0xe);

                flag = description->get_flag("test_field.valid", f);
                CATCH_REQUIRE(flag->full_name() == "test_field.valid");
                CATCH_REQUIRE(flag->field_name() == "test_field");
                CATCH_REQUIRE(flag->flag_name() == "valid");
                CATCH_REQUIRE(flag->pos() == 4);
                CATCH_REQUIRE(flag->size() == 1);
                CATCH_REQUIRE(flag->mask() == 0x10);

                flag = description->get_flag("test_field.side", f);
                CATCH_REQUIRE(flag->full_name() == "test_field.side");
                CATCH_REQUIRE(flag->field_name() == "test_field");
                CATCH_REQUIRE(flag->flag_name() == "side");
                CATCH_REQUIRE(flag->pos() == 5);
                CATCH_REQUIRE(flag->size() == 2);
                CATCH_REQUIRE(flag->mask() == 0x60);
            }
            else
            {
                CATCH_REQUIRE_THROWS_MATCHES(
                          description->get_flag(field_name, f)
                        , prinbee::field_not_found
                        , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: flag named \"test_field\" must at least include a field name and a flag name."));
            }

            // get_bits()/set_bits() functions
            //
            if((t.f_valid_func & VALID_FUNC_FLAG) != 0)
            {
                if((t.f_valid_func & VALID_FUNC_BITS) != 0)
                {
                    for(int v(0); v < 8; ++v)
                    {
                        description->set_bits("test_field.on", v & 1);
                        CATCH_REQUIRE(description->get_bits("test_field.on") == (v & 1));

                        description->set_bits("test_field.color", v & 7);
                        CATCH_REQUIRE(description->get_bits("test_field.color") == (v & 7));

                        description->set_bits("test_field.valid", v & 1);
                        CATCH_REQUIRE(description->get_bits("test_field.valid") == (v & 1));

                        description->set_bits("test_field.side", v & 3);
                        CATCH_REQUIRE(description->get_bits("test_field.side") == (v & 3));
                    }
                }
                else
                {
                    // these BITSxxx are not yet fully implemented
                    // (i.e. bits in large integers)
                    //
                    CATCH_REQUIRE_THROWS_MATCHES(
                              description->set_bits("test_field.on", rand() & 1)
                            , prinbee::type_mismatch
                            , Catch::Matchers::ExceptionMessage(
                                "prinbee_exception: this description type is \""
                              + prinbee::to_string(t.f_type)
                              + "\" but we expected one of"
                                " \"BITS8, BITS16, BITS32, BITS64\"."));

                    CATCH_REQUIRE_THROWS_MATCHES(
                              description->get_bits("test_field.on")
                            , prinbee::type_mismatch
                            , Catch::Matchers::ExceptionMessage(
                                "prinbee_exception: this description type is \""
                              + prinbee::to_string(t.f_type)
                              + "\" but we expected one of"
                                " \"BITS8, BITS16, BITS32, BITS64\"."));
                }
            }
            else
            {
SNAP_LOG_WARNING << "--- set_bits() ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE_THROWS_MATCHES(
                          description->set_bits(field_name, rand())
                        , prinbee::field_not_found
                        , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: flag named \"test_field\" must at least include a field name and a flag name."));

SNAP_LOG_WARNING << "--- get_bits() ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE_THROWS_MATCHES(
                          description->get_bits(field_name)
                        , prinbee::field_not_found
                        , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: flag named \"test_field\" must at least include a field name and a flag name."));
            }

            // get_uinteger()/set_uinteger()
            //
            if((t.f_valid_func & VALID_FUNC_UINTEGER) != 0)
            {
                for(int count(0); count < 10; ++count)
                {
                    std::uint64_t v(0);
                    SNAP_CATCH2_NAMESPACE::random(v);
                    description->set_uinteger(field_name, v);
                    std::uint64_t const r(v & (((static_cast<std::uint64_t>(1) << (t.f_mask_size - 1)) << 1) - 1));
                    CATCH_REQUIRE(description->get_uinteger(field_name) == r);
                }
            }
            else
            {
                std::uint64_t v(0);
                SNAP_CATCH2_NAMESPACE::random(v);
SNAP_LOG_WARNING << "--- set_uinteger() ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE_THROWS_MATCHES(
                          description->set_uinteger(field_name, v)
                        , prinbee::type_mismatch
                        , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: this description type is \""
                          + prinbee::to_string(t.f_type)
                          + "\""
                            " but we expected one of \"BITS8, BITS16, BITS32, BITS64, OID, REFERENCE, UINT8, UINT16, UINT32, UINT64, VERSION\"."));

SNAP_LOG_WARNING << "--- get_uinteger() ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE_THROWS_MATCHES(
                          description->get_uinteger(field_name)
                        , prinbee::type_mismatch
                        , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: this description type is \""
                          + prinbee::to_string(t.f_type)
                          + "\""
                            " but we expected one of \"BITS8, BITS16, BITS32, BITS64, MAGIC, OID, REFERENCE, STRUCTURE_VERSION, UINT8, UINT16, UINT32, UINT64, VERSION\"."));
            }

            // get_integer()/set_integer()
            //
            if((t.f_valid_func & VALID_FUNC_INTEGER) != 0)
            {
                for(int count(0); count < 10; ++count)
                {
                    std::int64_t v(0);
                    SNAP_CATCH2_NAMESPACE::random(v);
                    description->set_integer(field_name, v);
                    std::int64_t r(v & (((static_cast<std::uint64_t>(1) << (t.f_mask_size - 1)) << 1) - 1));
                    if(t.f_mask_size != 64
                    && (r & (static_cast<std::uint64_t>(1) << (t.f_mask_size - 1))) != 0)
                    {
                        // sign extend the value if less than 64 bits
                        //
                        r |= static_cast<std::int64_t>(-1) << t.f_mask_size;
                    }
                    CATCH_REQUIRE(description->get_integer(field_name) == r);
                }
            }
            else
            {
                std::int64_t v(0);
                SNAP_CATCH2_NAMESPACE::random(v);
SNAP_LOG_WARNING << "--- set_integer() ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE_THROWS_MATCHES(
                          description->set_integer(field_name, v)
                        , prinbee::type_mismatch
                        , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: this description type is \""
                          + prinbee::to_string(t.f_type)
                          + "\""
                            " but we expected one of \"INT8, INT16, INT32, INT64, MSTIME, TIME, USTIME\"."));

SNAP_LOG_WARNING << "--- get_integer() ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE_THROWS_MATCHES(
                          description->get_integer(field_name)
                        , prinbee::type_mismatch
                        , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: this description type is \""
                          + prinbee::to_string(t.f_type)
                          + "\""
                            " but we expected one of \"INT8, INT16, INT32, INT64, MSTIME, TIME, USTIME\"."));
SNAP_LOG_WARNING << "--- something else? ..." << SNAP_LOG_SEND;
            }

            // get_large_uinteger()/set_large_uinteger()
            //
            if((t.f_valid_func & VALID_FUNC_LARGE_UINTEGER) != 0)
            {
                for(int count(0); count < 10; ++count)
                {
                    prinbee::uint512_t v(0);
                    SNAP_CATCH2_NAMESPACE::rand512(v);
SNAP_LOG_WARNING << "--- first do set_large_uinteger()..." << SNAP_LOG_SEND;
                    description->set_large_uinteger(field_name, v);
                    prinbee::uint512_t r(v);
                    if(t.f_mask_size != 512)
                    {
                        prinbee::uint512_t mask(0);
                        prinbee::uint512_t one(1);
                        mask -= one;
                        mask >>= 512 - t.f_mask_size;
                        r &= mask;
                    }
SNAP_LOG_WARNING << "--- " << count << ". get_large_uinteger() without protection ..." << SNAP_LOG_SEND;
                    CATCH_REQUIRE(description->get_large_uinteger(field_name) == r);
SNAP_LOG_WARNING << "--- got the large integer get_large_uinteger() ..." << SNAP_LOG_SEND;
                }
SNAP_LOG_WARNING << "--- done with loop..." << SNAP_LOG_SEND;
            }
            else
            {
                prinbee::uint512_t v(0);
                SNAP_CATCH2_NAMESPACE::rand512(v);
SNAP_LOG_WARNING << "--- set_large_uinteger() ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE_THROWS_MATCHES(
                          description->set_large_uinteger(field_name, v)
                        , prinbee::type_mismatch
                        , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: this description type is \""
                          + prinbee::to_string(t.f_type)
                          + "\""
                            " but we expected one of \"BITS8, BITS16, BITS32, BITS64, OID, REFERENCE, UINT8, UINT16, UINT32, UINT64, UINT128, UINT256, UINT512, VERSION\"."));

SNAP_LOG_WARNING << "--- get_large_uinteger() ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE_THROWS_MATCHES(
                          description->get_large_uinteger(field_name)
                        , prinbee::type_mismatch
                        , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: this description type is \""
                          + prinbee::to_string(t.f_type)
                          + "\""
                            " but we expected one of \"BITS8, BITS16, BITS32, BITS64, MAGIC, OID, REFERENCE, STRUCTURE_VERSION, UINT8, UINT16, UINT32, UINT64, UINT128, UINT256, UINT512, VERSION\"."));
            }

            // get_integer()/set_integer()
            //
SNAP_LOG_WARNING << "--- large int?..." << SNAP_LOG_SEND;
            if((t.f_valid_func & VALID_FUNC_LARGE_INTEGER) != 0)
            {
SNAP_LOG_WARNING << "--- loop over large int?..." << SNAP_LOG_SEND;
                for(int count(0); count < 10; ++count)
                {
                    prinbee::int512_t v(0);
                    SNAP_CATCH2_NAMESPACE::rand512(v);
SNAP_LOG_WARNING << "--- set large int... inside loop" << SNAP_LOG_SEND;
                    description->set_large_integer(field_name, v);

                    prinbee::int512_t r(v);
                    if(t.f_mask_size != 512)
                    {
                        prinbee::int512_t const one(1);
                        prinbee::int512_t const sign(one << (t.f_mask_size - 1));
                        if((r & sign) == 0)
                        {
                            // positive number, clear upper bits
                            //
                            prinbee::uint512_t mask(0);
                            mask -= one;
                            mask >>= 512 - t.f_mask_size;
                            r &= mask;
                        }
                        else
                        {
                            // negative number, set upper bits to all ones
                            //
                            prinbee::int512_t mask(0);
                            mask -= one;
                            mask <<= t.f_mask_size;
                            r |= mask;
                        }
                    }
SNAP_LOG_WARNING << "--- " << count << ". get_large_integer() without protection ..." << SNAP_LOG_SEND;
                    CATCH_REQUIRE(description->get_large_integer(field_name) == r);
SNAP_LOG_WARNING << "--- got the get_large_integer() ..." << SNAP_LOG_SEND;
                }
            }
            else
            {
SNAP_LOG_WARNING << "--- random int512_t problem???..." << SNAP_LOG_SEND;
                prinbee::int512_t v(0);
                SNAP_CATCH2_NAMESPACE::rand512(v);
SNAP_LOG_WARNING << "--- set_large_integer() ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE_THROWS_MATCHES(
                          description->set_large_integer(field_name, v)
                        , prinbee::type_mismatch
                        , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: this description type is \""
                          + prinbee::to_string(t.f_type)
                          + "\""
                            " but we expected one of \"INT8, INT16, INT32, INT64, INT128, INT256, INT512, MSTIME, NSTIME, TIME, USTIME\"."));

SNAP_LOG_WARNING << "--- get_large_integer() ..." << SNAP_LOG_SEND;
                CATCH_REQUIRE_THROWS_MATCHES(
                          description->get_large_integer(field_name)
                        , prinbee::type_mismatch
                        , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: this description type is \""
                          + prinbee::to_string(t.f_type)
                          + "\""
                            " but we expected one of \"INT8, INT16, INT32, INT64, INT128, INT256, INT512, MSTIME, NSTIME, TIME, USTIME\"."));
            }
        }
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("structure_array", "[structure][array][valid]")
{
    CATCH_START_SECTION("structure: structure with an ARRAY8")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description7));

        // the strings make this structure dynamic
        //
        CATCH_REQUIRE(description->get_static_size() == 0UL);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 32UL + 1UL + 4UL);

        description->init_buffer();
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 32UL + 1UL + 4UL + 43UL);

        CATCH_REQUIRE_THROWS_MATCHES(
                  description->set_uinteger("_magic", SNAP_CATCH2_NAMESPACE::rand32())
                , prinbee::type_mismatch
                , Catch::Matchers::ExceptionMessage(
                    "prinbee_exception: this description type is \"MAGIC\""
                    " but we expected one of \"BITS8, BITS16, BITS32, BITS64, OID, REFERENCE, UINT8, UINT16, UINT32, UINT64, VERSION\"."));

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));

        CATCH_REQUIRE(description->get_string("name") == "users");
        std::string const name("different_name");
        description->set_string("name", name);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 32UL + 1UL + 4UL + 43UL);
        CATCH_REQUIRE(description->get_string("name") == name);

        CATCH_REQUIRE(description->get_string("comment") == "This represents a form of table definition.");
        std::string const comment_field(rand() % 1 == 0 ? "Short comment." : "This is a longer comment so we may test the pinsert() as well once in a while, but random is complicated to view the data.");
        description->set_string("comment", comment_field);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 32UL + 1UL + 4UL + comment_field.length());
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        // now create column definitions, one at a time and add them to
        // the array
        //
        prinbee::structure::pointer_t column1(description->new_array_item("columns"));

        CATCH_REQUIRE(column1->get_string("colname") == "_undefined");
        std::string const column1_name("col1");
        column1->set_string("colname", column1_name);
        CATCH_REQUIRE(column1->get_string("colname") == column1_name);

        CATCH_REQUIRE(column1->get_uinteger("max_size") == 256);
        std::uint16_t column1_max_size(0);
        SNAP_CATCH2_NAMESPACE::random(column1_max_size);
        column1->set_uinteger("max_size", column1_max_size);
        CATCH_REQUIRE(column1->get_uinteger("max_size") == column1_max_size);

        CATCH_REQUIRE(column1->get_uinteger("type") == 14);
        std::uint16_t column1_type(0);
        SNAP_CATCH2_NAMESPACE::random(column1_type);
        column1->set_uinteger("type", column1_type);
        CATCH_REQUIRE(column1->get_uinteger("type") == column1_type);

        // make sure the root was not affected
        //
        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        prinbee::structure::vector_t array(description->get_array("columns"));
        CATCH_REQUIRE(array.size() == 1);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 32UL
                + 1UL + 1UL + column1_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());

        // column #2
        //
        prinbee::structure::pointer_t column2(description->new_array_item("columns"));

        CATCH_REQUIRE(column2->get_string("colname") == "_undefined");
        std::string const column2_name("col2_long_name_here");
        column2->set_string("colname", column2_name);
        CATCH_REQUIRE(column2->get_string("colname") == column2_name);

        CATCH_REQUIRE(column2->get_uinteger("max_size") == 256);
        std::uint16_t column2_max_size(0);
        SNAP_CATCH2_NAMESPACE::random(column2_max_size);
        column2->set_uinteger("max_size", column2_max_size);
        CATCH_REQUIRE(column2->get_uinteger("max_size") == column2_max_size);

        CATCH_REQUIRE(column2->get_uinteger("type") == 14);
        std::uint16_t column2_type(0);
        SNAP_CATCH2_NAMESPACE::random(column2_type);
        column2->set_uinteger("type", column2_type);
        CATCH_REQUIRE(column2->get_uinteger("type") == column2_type);

        // make sure the root & column1 were not affected
        //
        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 2);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 32UL
                + 1UL + 1UL + column1_name.length() + 2UL + 2UL
                      + 1UL + column2_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());

        // column #3
        //
        prinbee::structure::pointer_t column3(description->new_array_item("columns"));

        CATCH_REQUIRE(column3->get_string("colname") == "_undefined");
        std::string const column3_name("col3__here");
        column3->set_string("colname", column3_name);
        CATCH_REQUIRE(column3->get_string("colname") == column3_name);

        CATCH_REQUIRE(column3->get_uinteger("max_size") == 256);
        std::uint16_t column3_max_size(0);
        SNAP_CATCH2_NAMESPACE::random(column3_max_size);
        column3->set_uinteger("max_size", column3_max_size);
        CATCH_REQUIRE(column3->get_uinteger("max_size") == column3_max_size);

        CATCH_REQUIRE(column3->get_uinteger("type") == 14);
        std::uint16_t column3_type(0);
        SNAP_CATCH2_NAMESPACE::random(column3_type);
        column3->set_uinteger("type", column3_type);
        CATCH_REQUIRE(column3->get_uinteger("type") == column3_type);

        // make sure the root & column 1/2 are not affected
        //
        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 3);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(column1->get_string("colname") == column1_name);
        CATCH_REQUIRE(column1->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(column1->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(column2->get_string("colname") == column2_name);
        CATCH_REQUIRE(column2->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(column2->get_uinteger("type") == column2_type);
        CATCH_REQUIRE(array[2] == column3);
        CATCH_REQUIRE(column3->get_string("colname") == column3_name);
        CATCH_REQUIRE(column3->get_uinteger("max_size") == column3_max_size);
        CATCH_REQUIRE(column3->get_uinteger("type") == column3_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 32UL
                + 1UL + 1UL + column1_name.length() + 2UL + 2UL
                      + 1UL + column2_name.length() + 2UL + 2UL
                      + 1UL + column3_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());
//{
//prinbee::reference_t start_offset(0);
//prinbee::virtual_buffer::pointer_t buffer(description->get_virtual_buffer(start_offset));
//std::cout << std::flush << "column3 at the end -- buffer start offset: " << start_offset << "\n" << *buffer << std::endl;
//description->display_offsets();
//}
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure: structure with an ARRAY16")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description8));

        // the strings make this structure dynamic
        //
        CATCH_REQUIRE(description->get_static_size() == 0UL);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + 2UL + 4UL);

        description->init_buffer();
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + 5UL + 2UL + 4UL + 33UL);

        CATCH_REQUIRE_THROWS_MATCHES(
                  description->set_uinteger("_magic", SNAP_CATCH2_NAMESPACE::rand32())
                , prinbee::type_mismatch
                , Catch::Matchers::ExceptionMessage(
                    "prinbee_exception: this description type is \"MAGIC\""
                    " but we expected one of \"BITS8, BITS16, BITS32, BITS64, OID, REFERENCE, UINT8, UINT16, UINT32, UINT64, VERSION\"."));

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));

        CATCH_REQUIRE(description->get_string("name") == "users");
        std::string name("different_name");
        description->set_string("name", name);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + name.length() + 2UL + 4UL + 33UL);
        CATCH_REQUIRE(description->get_string("name") == name);

        CATCH_REQUIRE(description->get_string("comment") == "Another form of table definition.");
        std::string comment_field(rand() % 1 == 0 ? "Perfect table." : "This is a longer comment so we may test the pinsert() as well once in a while, but random is complicated to view the data.");
        description->set_string("comment", comment_field);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + name.length() + 2UL + 4UL + comment_field.length());
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        // now create column definitions, one at a time and add them to
        // the array
        //
        prinbee::structure::pointer_t column1(description->new_array_item("columns"));

        CATCH_REQUIRE(column1->get_string("colname") == "_undefined");
        std::string column1_name("wide_col1");
        column1->set_string("colname", column1_name);
        CATCH_REQUIRE(column1->get_string("colname") == column1_name);

        CATCH_REQUIRE(column1->get_uinteger("max_size") == 256);
        std::uint16_t column1_max_size(0);
        SNAP_CATCH2_NAMESPACE::random(column1_max_size);
        column1->set_uinteger("max_size", column1_max_size);
        CATCH_REQUIRE(column1->get_uinteger("max_size") == column1_max_size);

        CATCH_REQUIRE(column1->get_uinteger("type") == 14);
        std::uint16_t column1_type(0);
        SNAP_CATCH2_NAMESPACE::random(column1_type);
        column1->set_uinteger("type", column1_type);
        CATCH_REQUIRE(column1->get_uinteger("type") == column1_type);

        // make sure the root was not affected
        //
        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        prinbee::structure::vector_t array(description->get_array("columns"));
        CATCH_REQUIRE(array.size() == 1);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + name.length()
                + 2UL + 1UL + column1_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());

        // now change the root structure (name) and make sure that column #1 is still fine
        //
        name = "change_the_name_to_a_longer_one";
        description->set_string("name", name);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 1);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);

        // change the root structure again (comment) and make sure that column #1 is still fine
        //
        comment_field = "Just another comment to stick at the end.";
        description->set_string("comment", comment_field);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 1);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + name.length()
                + 2UL + 1UL + column1_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());

        // also test changing the 1st column name
        //
        column1_name = "renamed_column1_to_make_sure_we_can_do_that";
        column1->set_string("colname", column1_name);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 1);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + name.length()
                + 2UL + 1UL + column1_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());

        // column #2
        //
        prinbee::structure::pointer_t column2(description->new_array_item("columns"));

        CATCH_REQUIRE(column2->get_string("colname") == "_undefined");
        std::string column2_name("wide_col2_long_name_here");
        column2->set_string("colname", column2_name);
        CATCH_REQUIRE(column2->get_string("colname") == column2_name);

        CATCH_REQUIRE(column2->get_uinteger("max_size") == 256);
        std::uint16_t column2_max_size(0);
        SNAP_CATCH2_NAMESPACE::random(column2_max_size);
        column2->set_uinteger("max_size", column2_max_size);
        CATCH_REQUIRE(column2->get_uinteger("max_size") == column2_max_size);

        CATCH_REQUIRE(column2->get_uinteger("type") == 14);
        std::uint16_t column2_type(0);
        SNAP_CATCH2_NAMESPACE::random(column2_type);
        column2->set_uinteger("type", column2_type);
        CATCH_REQUIRE(column2->get_uinteger("type") == column2_type);

        // make sure the root & column1 were not affected
        //
        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 2);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + name.length()
                + 2UL + 1UL + column1_name.length() + 2UL + 2UL
                      + 1UL + column2_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());

        // now change the root structure (name) and make sure that column #1 is still fine
        //
        name = "smaller_name_this_time";
        description->set_string("name", name);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 2);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);

        // change the root structure again (comment) and make sure that column #1 is still fine
        //
        comment_field = "So many ALTER happening here!";
        description->set_string("comment", comment_field);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 2);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + name.length()
                + 2UL + 1UL + column1_name.length() + 2UL + 2UL
                      + 1UL + column2_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());

        // also test changing the 1st & 2nd column names
        //
        column1_name = "renamed_column1_to_make_sure_we_can_do_that";
        column1->set_string("colname", column1_name);
        column2_name = "col2";
        column2->set_string("colname", column2_name);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 2);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + name.length()
                + 2UL + 1UL + column1_name.length() + 2UL + 2UL
                      + 1UL + column2_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());

        // column #3
        //
        prinbee::structure::pointer_t column3(description->new_array_item("columns"));

        CATCH_REQUIRE(column3->get_string("colname") == "_undefined");
        std::string column3_name("col3__here");
        column3->set_string("colname", column3_name);
        CATCH_REQUIRE(column3->get_string("colname") == column3_name);

        CATCH_REQUIRE(column3->get_uinteger("max_size") == 256);
        std::uint16_t column3_max_size(0);
        SNAP_CATCH2_NAMESPACE::random(column3_max_size);
        column3->set_uinteger("max_size", column3_max_size);
        CATCH_REQUIRE(column3->get_uinteger("max_size") == column3_max_size);

        CATCH_REQUIRE(column3->get_uinteger("type") == 14);
        std::uint16_t column3_type(0);
        SNAP_CATCH2_NAMESPACE::random(column3_type);
        column3->set_uinteger("type", column3_type);
        CATCH_REQUIRE(column3->get_uinteger("type") == column3_type);

        // make sure the root & column 1/2 are not affected
        //
        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 3);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(column1->get_string("colname") == column1_name);
        CATCH_REQUIRE(column1->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(column1->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(column2->get_string("colname") == column2_name);
        CATCH_REQUIRE(column2->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(column2->get_uinteger("type") == column2_type);
        CATCH_REQUIRE(array[2] == column3);
        CATCH_REQUIRE(column3->get_string("colname") == column3_name);
        CATCH_REQUIRE(column3->get_uinteger("max_size") == column3_max_size);
        CATCH_REQUIRE(column3->get_uinteger("type") == column3_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + name.length()
                + 2UL + 1UL + column1_name.length() + 2UL + 2UL
                      + 1UL + column2_name.length() + 2UL + 2UL
                      + 1UL + column3_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());

        // now change the root structure (name) and make sure that column #1 is still fine
        //
        name = "final_name";
        description->set_string("name", name);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 3);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);
        CATCH_REQUIRE(array[2] == column3);
        CATCH_REQUIRE(array[2]->get_string("colname") == column3_name);
        CATCH_REQUIRE(array[2]->get_uinteger("max_size") == column3_max_size);
        CATCH_REQUIRE(array[2]->get_uinteger("type") == column3_type);

        // change the root structure again (comment) and make sure that column #1 is still fine
        //
        comment_field = "Final comment.";
        description->set_string("comment", comment_field);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 3);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);
        CATCH_REQUIRE(array[2] == column3);
        CATCH_REQUIRE(array[2]->get_string("colname") == column3_name);
        CATCH_REQUIRE(array[2]->get_uinteger("max_size") == column3_max_size);
        CATCH_REQUIRE(array[2]->get_uinteger("type") == column3_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + name.length()
                + 2UL + 1UL + column1_name.length() + 2UL + 2UL
                      + 1UL + column2_name.length() + 2UL + 2UL
                      + 1UL + column3_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());

        // also test changing the 1st & 2nd column names
        //
        column1_name = "changing_again";
        column1->set_string("colname", column1_name);
        column2_name = "col2_final_name";
        column2->set_string("colname", column2_name);
        column3_name = "col3_new_name";
        column3->set_string("colname", column3_name);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 3);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);
        CATCH_REQUIRE(array[2] == column3);
        CATCH_REQUIRE(array[2]->get_string("colname") == column3_name);
        CATCH_REQUIRE(array[2]->get_uinteger("max_size") == column3_max_size);
        CATCH_REQUIRE(array[2]->get_uinteger("type") == column3_type);

        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 4UL + name.length()
                + 2UL + 1UL + column1_name.length() + 2UL + 2UL
                      + 1UL + column2_name.length() + 2UL + 2UL
                      + 1UL + column3_name.length() + 2UL + 2UL
                + 4UL + comment_field.length());
//{
//prinbee::reference_t start_offset(0);
//prinbee::virtual_buffer::pointer_t buffer(description->get_virtual_buffer(start_offset));
//std::cout << std::flush << "column3 at the end -- buffer start offset: " << start_offset << "\n" << *buffer << std::endl;
//description->display_offsets();
//}
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure: structure with an ARRAY32")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description9));

        // the strings make this structure dynamic
        //
        CATCH_REQUIRE(description->get_static_size() == 0UL);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 1UL + 4UL + 2UL);

        description->init_buffer();
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 1UL + 4UL + 4UL + 2UL + 56UL);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));

        CATCH_REQUIRE(description->get_string("name") == "page");
        std::string name("thirty_two_bits");
        description->set_string("name", name);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 1UL + name.length() + 4UL + 2UL + 56UL);
        CATCH_REQUIRE(description->get_string("name") == name);

        CATCH_REQUIRE(description->get_string("comment") == "Table with a number of columns that can go to 4 billion.");
        std::string comment_field(rand() % 1 == 0 ? "Randomly small comment." : "This one's small too...");
        description->set_string("comment", comment_field);
        CATCH_REQUIRE(description->get_current_size() == 4UL + 4UL + 1UL + name.length() + 4UL + 2UL + comment_field.length());
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        // now create column definitions, one at a time and add them to
        // the array
        //
        prinbee::structure::pointer_t column1(description->new_array_item("columns"));

        CATCH_REQUIRE(column1->get_string("colname") == "_undefined");
        std::string const column1_name("wide_col1");
        column1->set_string("colname", column1_name);
        CATCH_REQUIRE(column1->get_string("colname") == column1_name);

        CATCH_REQUIRE(column1->get_uinteger("max_size") == 256);
        std::uint16_t column1_max_size(0);
        SNAP_CATCH2_NAMESPACE::random(column1_max_size);
        column1->set_uinteger("max_size", column1_max_size);
        CATCH_REQUIRE(column1->get_uinteger("max_size") == column1_max_size);

        CATCH_REQUIRE(column1->get_uinteger("type") == 14);
        std::uint16_t column1_type(0);
        SNAP_CATCH2_NAMESPACE::random(column1_type);
        column1->set_uinteger("type", column1_type);
        CATCH_REQUIRE(column1->get_uinteger("type") == column1_type);

        // make sure the root was not affected
        //
        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        prinbee::structure::vector_t array(description->get_array("columns"));
        CATCH_REQUIRE(array.size() == 1);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);

        // now change the root structure (name) and make sure that column #1 is still fine
        //
        name = "we_want_to_test_with_a_very_long_name";
        description->set_string("name", name);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 1);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);

        // change the root structure again (comment) and make sure that column #1 is still fine
        //
        comment_field = "Just another comment to stick at the end.";
        description->set_string("comment", comment_field);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 1);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);

        {
            prinbee::reference_t start_offset(0);
            prinbee::virtual_buffer::pointer_t b(description->get_virtual_buffer(start_offset));
            CATCH_REQUIRE(b != nullptr);
            CATCH_REQUIRE(b->size() == description->get_current_size());

            prinbee::buffer_t buffer(b->size());
            CATCH_REQUIRE(b->pread(buffer.data(), buffer.size(), 0) == static_cast<int>(buffer.size()));

            prinbee::virtual_buffer::pointer_t n(std::make_shared<prinbee::virtual_buffer>());
            n->pwrite(buffer.data(), buffer.size(), 0, true);

            prinbee::structure::pointer_t d(std::make_shared<prinbee::structure>(g_description9));
            d->set_virtual_buffer(n, 0);

            CATCH_REQUIRE(d->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
            CATCH_REQUIRE(d->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
            CATCH_REQUIRE(d->get_string("name") == name);
            CATCH_REQUIRE(d->get_string("comment") == comment_field);

            array = d->get_array("columns");
            CATCH_REQUIRE(array.size() == 1);
            CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
            CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
            CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        }

        // column #2
        //
        prinbee::structure::pointer_t column2(description->new_array_item("columns"));

        CATCH_REQUIRE(column2->get_string("colname") == "_undefined");
        std::string const column2_name("wide_col2_long_name_here");
        column2->set_string("colname", column2_name);
        CATCH_REQUIRE(column2->get_string("colname") == column2_name);

        CATCH_REQUIRE(column2->get_uinteger("max_size") == 256);
        std::uint16_t column2_max_size(0);
        SNAP_CATCH2_NAMESPACE::random(column2_max_size);
        column2->set_uinteger("max_size", column2_max_size);
        CATCH_REQUIRE(column2->get_uinteger("max_size") == column2_max_size);

        CATCH_REQUIRE(column2->get_uinteger("type") == 14);
        std::uint16_t column2_type(0);
        SNAP_CATCH2_NAMESPACE::random(column2_type);
        column2->set_uinteger("type", column2_type);
        CATCH_REQUIRE(column2->get_uinteger("type") == column2_type);

        // make sure the root & column1 were not affected
        //
        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 2);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);

        // now change the root structure (name) and make sure that column #1 is still fine
        //
        name = "smaller_name_this_time";
        description->set_string("name", name);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 2);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);

        // change the root structure again (comment) and make sure that column #1 is still fine
        //
        comment_field = "So many ALTER happening here! I just can't believe it's all working.";
        description->set_string("comment", comment_field);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 2);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);

        {
            prinbee::reference_t start_offset(0);
            prinbee::virtual_buffer::pointer_t b(description->get_virtual_buffer(start_offset));
            CATCH_REQUIRE(b != nullptr);
            CATCH_REQUIRE(b->size() == description->get_current_size());

            prinbee::buffer_t buffer(b->size());
            CATCH_REQUIRE(b->pread(buffer.data(), buffer.size(), 0) == static_cast<int>(buffer.size()));

            prinbee::virtual_buffer::pointer_t n(std::make_shared<prinbee::virtual_buffer>());
            n->pwrite(buffer.data(), buffer.size(), 0, true);

            prinbee::structure::pointer_t d(std::make_shared<prinbee::structure>(g_description9));
            d->set_virtual_buffer(n, 0);

            CATCH_REQUIRE(d->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
            CATCH_REQUIRE(d->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
            CATCH_REQUIRE(d->get_string("name") == name);
            CATCH_REQUIRE(d->get_string("comment") == comment_field);

            array = d->get_array("columns");
            CATCH_REQUIRE(array.size() == 2);
            CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
            CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
            CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
            CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
            CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
            CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);
        }

        // column #3
        //
        prinbee::structure::pointer_t column3(description->new_array_item("columns"));

        CATCH_REQUIRE(column3->get_string("colname") == "_undefined");
        std::string const column3_name("col3__here");
        column3->set_string("colname", column3_name);
        CATCH_REQUIRE(column3->get_string("colname") == column3_name);

        CATCH_REQUIRE(column3->get_uinteger("max_size") == 256);
        std::uint16_t column3_max_size(0);
        SNAP_CATCH2_NAMESPACE::random(column3_max_size);
        column3->set_uinteger("max_size", column3_max_size);
        CATCH_REQUIRE(column3->get_uinteger("max_size") == column3_max_size);

        CATCH_REQUIRE(column3->get_uinteger("type") == 14);
        std::uint16_t column3_type(0);
        SNAP_CATCH2_NAMESPACE::random(column3_type);
        column3->set_uinteger("type", column3_type);
        CATCH_REQUIRE(column3->get_uinteger("type") == column3_type);

        // make sure the root & column 1/2 are not affected
        //
        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 3);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(column1->get_string("colname") == column1_name);
        CATCH_REQUIRE(column1->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(column1->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(column2->get_string("colname") == column2_name);
        CATCH_REQUIRE(column2->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(column2->get_uinteger("type") == column2_type);
        CATCH_REQUIRE(array[2] == column3);
        CATCH_REQUIRE(column3->get_string("colname") == column3_name);
        CATCH_REQUIRE(column3->get_uinteger("max_size") == column3_max_size);
        CATCH_REQUIRE(column3->get_uinteger("type") == column3_type);

        // now change the root structure (name) and make sure that column #1 is still fine
        //
        name = "pretty_much_final_name";
        description->set_string("name", name);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 3);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);
        CATCH_REQUIRE(array[2] == column3);
        CATCH_REQUIRE(array[2]->get_string("colname") == column3_name);
        CATCH_REQUIRE(array[2]->get_uinteger("max_size") == column3_max_size);
        CATCH_REQUIRE(array[2]->get_uinteger("type") == column3_type);

        // change the root structure again (comment) and make sure that column #1 is still fine
        //
        comment_field = "Final comment.";
        description->set_string("comment", comment_field);

        CATCH_REQUIRE(description->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
        CATCH_REQUIRE(description->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
        CATCH_REQUIRE(description->get_string("name") == name);
        CATCH_REQUIRE(description->get_string("comment") == comment_field);

        array = description->get_array("columns");
        CATCH_REQUIRE(array.size() == 3);
        CATCH_REQUIRE(array[0] == column1);
        CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
        CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
        CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
        CATCH_REQUIRE(array[1] == column2);
        CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
        CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
        CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);
        CATCH_REQUIRE(array[2] == column3);
        CATCH_REQUIRE(array[2]->get_string("colname") == column3_name);
        CATCH_REQUIRE(array[2]->get_uinteger("max_size") == column3_max_size);
        CATCH_REQUIRE(array[2]->get_uinteger("type") == column3_type);
//{
//prinbee::reference_t start_offset(0);
//prinbee::virtual_buffer::pointer_t buffer(description->get_virtual_buffer(start_offset));
//std::cout << std::flush << "column3 at the end -- buffer start offset: " << start_offset << "\n" << *buffer << std::endl;
//description->display_offsets();
//}
        {
            prinbee::reference_t start_offset(0);
            prinbee::virtual_buffer::pointer_t b(description->get_virtual_buffer(start_offset));
            CATCH_REQUIRE(b != nullptr);
            CATCH_REQUIRE(b->size() == description->get_current_size());

            prinbee::buffer_t buffer(b->size());
            CATCH_REQUIRE(b->pread(buffer.data(), buffer.size(), 0) == static_cast<int>(buffer.size()));

            prinbee::virtual_buffer::pointer_t n(std::make_shared<prinbee::virtual_buffer>());
            n->pwrite(buffer.data(), buffer.size(), 0, true);

            prinbee::structure::pointer_t d(std::make_shared<prinbee::structure>(g_description9));
            d->set_virtual_buffer(n, 0);

            CATCH_REQUIRE(d->get_magic() == prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS);
            CATCH_REQUIRE(d->get_version(prinbee::g_system_field_name_structure_version) == prinbee::version_t(1, 2));
            CATCH_REQUIRE(d->get_string("name") == name);
            CATCH_REQUIRE(d->get_string("comment") == comment_field);

            array = d->get_array("columns");
            CATCH_REQUIRE(array.size() == 3);
            CATCH_REQUIRE(array[0]->get_string("colname") == column1_name);
            CATCH_REQUIRE(array[0]->get_uinteger("max_size") == column1_max_size);
            CATCH_REQUIRE(array[0]->get_uinteger("type") == column1_type);
            CATCH_REQUIRE(array[1]->get_string("colname") == column2_name);
            CATCH_REQUIRE(array[1]->get_uinteger("max_size") == column2_max_size);
            CATCH_REQUIRE(array[1]->get_uinteger("type") == column2_type);
            CATCH_REQUIRE(array[2]->get_string("colname") == column3_name);
            CATCH_REQUIRE(array[2]->get_uinteger("max_size") == column3_max_size);
            CATCH_REQUIRE(array[2]->get_uinteger("type") == column3_type);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("structure_invalid", "[structure][invalid]")
{
    CATCH_START_SECTION("structure_invalid: missing description (nullptr)")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  new prinbee::structure(nullptr)
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage("logic_error: the description parameter of a structure object cannot be null."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: missing description (empty)")
    {
        prinbee::struct_description_t description[] =
        {
            prinbee::end_descriptions()
        };
        CATCH_REQUIRE_THROWS_MATCHES(
                  new prinbee::structure(description)
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage("logic_error: the root description of a structure must start with a magic field followed by a structure version."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: structure version missing in description")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<prinbee::structure>(g_description4a)
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage("logic_error: the root description of a structure must start with a magic field followed by a structure version."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: magic missing in description")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<prinbee::structure>(g_description4b)
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage("logic_error: the root description of a structure must start with a magic field followed by a structure version."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: invalid CHAR size (missing equal)")
    {
        prinbee::struct_description_t description = {
            .f_field_name = "char123",
            .f_type = prinbee::struct_type_t::STRUCT_TYPE_CHAR,
        };

#ifdef _DEBUG
        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<prinbee::field_t>(&description)
                , prinbee::logic_error
                , Catch::Matchers::ExceptionMessage("logic_error: char field name & length \"char123\" are not valid."));
#else
        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<prinbee::field_t>(&description)
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: the name of a field of type CHAR must have a size"
                          " defined as in \"foo=123\"; \"char123\" is missing an equal (=) character."));
#endif
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: invalid CHAR size (size too large)")
    {
        prinbee::struct_description_t description = {
            .f_field_name = "char=9999999999999999999",
            .f_type = prinbee::struct_type_t::STRUCT_TYPE_CHAR,
        };

        CATCH_REQUIRE_THROWS_MATCHES(
                  std::make_shared<prinbee::field_t>(&description)
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                        "prinbee_exception: the size in field"
                        " \"char=9999999999999999999\" must be a valid decimal number."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: get unknown field")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description9));
        description->init_buffer();

        try
        {
            description->get_field("unknown.field.name");
            throw prinbee::logic_error("the expected exception did not occur.");
        }
        catch(prinbee::field_not_found const & e)
        {
            CATCH_REQUIRE(e.what() == std::string("prinbee_exception: this description does not include a field named \"unknown\"."));
            CATCH_REQUIRE(e.get_parameter("field_name") == "unknown");
            CATCH_REQUIRE(e.get_parameter("full_field_name") == "unknown.field.name");
        }

        //CATCH_REQUIRE_THROWS_MATCHES(
        //          description->get_field("unknown.field.name")
        //        , prinbee::field_not_found
        //        , Catch::Matchers::ExceptionMessage(
        //                "prinbee_exception: this description does not include a field named \"unknown\"."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: get unknown field with a sub-name")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description5));
        description->init_buffer();

        try
        {
            description->get_field("early_version.field");
            throw prinbee::logic_error("the expected exception did not occur.");
        }
        catch(prinbee::field_not_found const & e)
        {
            CATCH_REQUIRE(e.what() == std::string("prinbee_exception: this description does not include a field named \"field\"."));
            CATCH_REQUIRE(e.get_parameter("field_name") == "field");
            CATCH_REQUIRE(e.get_parameter("full_field_name") == "early_version.field");
        }

        //CATCH_REQUIRE_THROWS_MATCHES(
        //          description->get_field("early_version.field")
        //        , prinbee::field_not_found
        //        , Catch::Matchers::ExceptionMessage(
        //                "prinbee_exception: this description does not include a field named \"field\"."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: get field with wrong type")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description5));
        description->init_buffer();

        CATCH_REQUIRE_THROWS_MATCHES(
                  description->get_field("early_version.size", prinbee::struct_type_t::STRUCT_TYPE_UINT16)
                , prinbee::type_mismatch
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: this field type is \""
                        + prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT8)
                        + "\" but we expected \""
                        + prinbee::to_string(prinbee::struct_type_t::STRUCT_TYPE_UINT16)
                        + "\"."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: get field with wrong type")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description5));
        description->init_buffer();

        CATCH_REQUIRE_THROWS_MATCHES(
                  description->get_field("data.bad_type", prinbee::struct_type_t::STRUCT_TYPE_UINT16)
                , prinbee::type_mismatch
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: field \"data\" is not of"
                          " type structure or bit field so you can't get a"
                          " sub-field (i.e. have a period in the name)."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: get field.flag with wrong flag name")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description3));
        description->init_buffer();

        try
        {
            description->get_field("eight_bits.unknown_flag");
            throw prinbee::logic_error("the expected exception did not occur.");
        }
        catch(prinbee::field_not_found const & e)
        {
            CATCH_REQUIRE(e.what() == std::string("prinbee_exception: flag named \"unknown_flag\" not found."));
            CATCH_REQUIRE(e.get_parameter("flag_name") == "unknown_flag");
        }

        //CATCH_REQUIRE_THROWS_MATCHES(
        //          description->get_field("eight_bits.unknown_flag")
        //        , prinbee::field_not_found
        //        , Catch::Matchers::ExceptionMessage(
        //                  "prinbee_exception: flag named \"unknown_flag\" not found."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: get field.flag with wrong type")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description3));
        description->init_buffer();

        CATCH_REQUIRE_THROWS_MATCHES(
                  description->get_field("eight_bits.efficient", prinbee::struct_type_t::STRUCT_TYPE_BITS16)
                , prinbee::type_mismatch
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: this field type is \"BITS8\" but"
                          " we expected \"BITS16\"."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: get flag with flag name missing")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description3));
        description->init_buffer();

        prinbee::field_t::pointer_t f;
        try
        {
            description->get_flag("eight_bits", f);
            throw prinbee::logic_error("the expected exception did not occur.");
        }
        catch(prinbee::field_not_found const & e)
        {
            CATCH_REQUIRE(e.what() == std::string(
                    "prinbee_exception: flag named \"eight_bits\" must"
                    " at least include a field name and a flag name."));
            CATCH_REQUIRE(e.get_parameter("flag_name") == "eight_bits");
        }

        //CATCH_REQUIRE_THROWS_MATCHES(
        //          description->get_flag("eight_bits", f)
        //        , prinbee::field_not_found
        //        , Catch::Matchers::ExceptionMessage(
        //                  "prinbee_exception: flag named \"eight_bits\" must"
        //                  " at least include a field name and a flag name."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: get flag with UINT32 type")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description3));
        description->init_buffer();

        prinbee::field_t::pointer_t f;
        CATCH_REQUIRE_THROWS_MATCHES(
                  description->get_flag("sub_field.efficient", f)
                , prinbee::type_mismatch
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: expected a field of type"
                          " BITS<size> for flag named \"sub_field.efficient\"."
                          " Got a UINT32 instead."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("structure_invalid: get field which is RENAMED with an invalid destination name")
    {
        prinbee::structure::pointer_t description(std::make_shared<prinbee::structure>(g_description10));
        description->init_buffer();

        prinbee::field_t::pointer_t f;
        try
        {
            description->get_field("buffer_size");
            throw prinbee::logic_error("the expected exception did not occur.");
        }
        catch(prinbee::field_not_found const & e)
        {
            CATCH_REQUIRE(e.what() == std::string("prinbee_exception: this description renames field"
                          " \"buffer_size\" to \"unknown\" but we could not"
                          " find the latter field."));
            CATCH_REQUIRE(e.get_parameter("field_name") == "buffer_size");
            CATCH_REQUIRE(e.get_parameter("new_name") == "unknown");
            CATCH_REQUIRE(e.get_parameter("full_field_name") == "buffer_size");
        }

        //CATCH_REQUIRE_THROWS_MATCHES(
        //          description->get_field("buffer_size")
        //        , prinbee::field_not_found
        //        , Catch::Matchers::ExceptionMessage(
        //                  "prinbee_exception: this description renames field"
        //                  " \"buffer_size\" to \"unknown\" but we could not"
        //                  " find the latter field."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
