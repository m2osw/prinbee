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

// self
//
#include    "catch_main.h"


// prinbee
//
#include    <prinbee/exception.h>
#include    <prinbee/data/dbtype.h>


//// snapdev
////
//#include    <snapdev/hexadecimal_string.h>
//#include    <snapdev/math.h>
//#include    <snapdev/ostream_int128.h>


//// C++
////
//#include    <fstream>
//#include    <iomanip>


//// C
////
//#include    <sys/stat.h>
//#include    <sys/types.h>


// last include
//
#include    <snapdev/poison.h>



namespace
{


bool is_valid_type(prinbee::dbtype_t type)
{
    switch(type)
    {
    case prinbee::dbtype_t::DBTYPE_UNKNOWN:
    case prinbee::dbtype_t::FILE_TYPE_TABLE:
    case prinbee::dbtype_t::FILE_TYPE_INDEX:
    case prinbee::dbtype_t::FILE_TYPE_BLOOM_FILTER:
    case prinbee::dbtype_t::FILE_TYPE_SCHEMA:
    case prinbee::dbtype_t::FILE_TYPE_PRIMARY_INDEX:
    case prinbee::dbtype_t::FILE_TYPE_COMPLEX_TYPE:
    case prinbee::dbtype_t::BLOCK_TYPE_BLOB:
    case prinbee::dbtype_t::BLOCK_TYPE_DATA:
    case prinbee::dbtype_t::BLOCK_TYPE_ENTRY_INDEX:
    case prinbee::dbtype_t::BLOCK_TYPE_FREE_BLOCK:
    case prinbee::dbtype_t::BLOCK_TYPE_FREE_SPACE:
    case prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS:
    case prinbee::dbtype_t::BLOCK_TYPE_INDIRECT_INDEX:
    case prinbee::dbtype_t::BLOCK_TYPE_SECONDARY_INDEX:
    case prinbee::dbtype_t::BLOCK_TYPE_SCHEMA_LIST:
    case prinbee::dbtype_t::BLOCK_TYPE_TOP_INDEX:
    case prinbee::dbtype_t::BLOCK_TYPE_TOP_INDIRECT_INDEX:
        return true;

    }

    return false;
}



} // no name namespace



CATCH_TEST_CASE("dbfile_dbtype", "[dbfile] [valid]")
{
    CATCH_START_SECTION("dbfile_dbtype: to_name()")
    {
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::DBTYPE_UNKNOWN)) == "Unknown");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::FILE_TYPE_TABLE)) == "Prinbee Table (PTBL)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::FILE_TYPE_INDEX)) == "Index (INDX)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::FILE_TYPE_BLOOM_FILTER)) == "Bloom Filter (BLMF)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::FILE_TYPE_PRIMARY_INDEX)) == "Primary Index (PIDX)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::FILE_TYPE_SCHEMA)) == "Schema (SCHM)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::FILE_TYPE_COMPLEX_TYPE)) == "Complex Type (CXTP)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::BLOCK_TYPE_BLOB)) == "Blob (BLOB)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::BLOCK_TYPE_DATA)) == "Data (DATA)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::BLOCK_TYPE_ENTRY_INDEX)) == "Entry Index (EIDX)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::BLOCK_TYPE_FREE_BLOCK)) == "Free Block (FREE)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::BLOCK_TYPE_FREE_SPACE)) == "Free Space (FSPC)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS)) == "Index Pointer (IDXP)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::BLOCK_TYPE_INDIRECT_INDEX)) == "Indirect Index (INDR)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::BLOCK_TYPE_SECONDARY_INDEX)) == "Secondary Index (SIDX)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::BLOCK_TYPE_SCHEMA_LIST)) == "Schema List (SCHL)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::BLOCK_TYPE_TOP_INDEX)) == "Top Index (TIDX)");
        CATCH_REQUIRE(std::string(prinbee::to_name(prinbee::dbtype_t::BLOCK_TYPE_TOP_INDIRECT_INDEX)) == "Top Indirect Index (TIND)");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("dbfile_dbtype: to_string()")
    {
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::DBTYPE_UNKNOWN)) == "????");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::FILE_TYPE_TABLE)) == "PTBL");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::FILE_TYPE_INDEX)) == "INDX");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::FILE_TYPE_BLOOM_FILTER)) == "BLMF");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::FILE_TYPE_PRIMARY_INDEX)) == "PIDX");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::FILE_TYPE_SCHEMA)) == "SCHM");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::FILE_TYPE_COMPLEX_TYPE)) == "CXTP");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_BLOB)) == "BLOB");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_DATA)) == "DATA");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_ENTRY_INDEX)) == "EIDX");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_FREE_BLOCK)) == "FREE");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_FREE_SPACE)) == "FSPC");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_INDEX_POINTERS)) == "IDXP");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_INDIRECT_INDEX)) == "INDR");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_SECONDARY_INDEX)) == "SIDX");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_SCHEMA_LIST)) == "SCHL");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_TOP_INDEX)) == "TIDX");
        CATCH_REQUIRE(std::string(prinbee::to_string(prinbee::dbtype_t::BLOCK_TYPE_TOP_INDIRECT_INDEX)) == "TIND");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("dbfile_dbtype: to_string() -- invalid types")
    {
        // anything else is an invalid type
        //
        for(int i(0); i < 100; ++i)
        {
            prinbee::dbtype_t type(prinbee::dbtype_t::DBTYPE_UNKNOWN);
            do
            {
                type = static_cast<prinbee::dbtype_t>(SNAP_CATCH2_NAMESPACE::rand32());
            }
            while(is_valid_type(type));

            CATCH_REQUIRE(std::string(prinbee::to_name(type)) == "Invalid");
            CATCH_REQUIRE(std::string(prinbee::to_string(type)) == "INVL");
        }
    }
    CATCH_END_SECTION()
}




// vim: ts=4 sw=4 et
