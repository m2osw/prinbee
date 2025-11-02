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
#pragma once


/** \file
 * \brief Types found in files.
 *
 * Each file and block has a few bytes at the start which generally defines
 * the type of the file and block.
 *
 * This file lists the various types we currently support. It is used by
 * the dbfile.cpp/.h and block.cpp/.h files.
 */

// C++
//
#include    <cstdint>
#include    <string>


namespace prinbee
{


#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define DBTYPE_NAME(s)       (static_cast<std::uint32_t>((s[0]<<24)|(s[1]<<16)|(s[2]<<8)|(s[3]<<0)))
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define DBTYPE_NAME(s)       (static_cast<std::uint32_t>((s[0]<<0)|(s[1]<<8)|(s[2]<<16)|(s[3]<<24)))
#else
#error "Unsupported endianess"
#endif

enum class dbtype_t : std::uint32_t
{
    DBTYPE_UNKNOWN                  = DBTYPE_NAME("????"),

    FILE_TYPE_COMPLEX_TYPE          = DBTYPE_NAME("CXTP"),      // User Defined Types (in "<prinbee-path>/contexts/<context-name>/complex-types.pb")
    FILE_TYPE_CONTEXT               = DBTYPE_NAME("CTXT"),      // Context (in "<prinbee-path>/contexts/<context-name>/context.pb")
    FILE_TYPE_SCHEMA                = DBTYPE_NAME("SCHM"),      // Table Schema (in "<prinbee-path>/contexts/<context-name>/<table-name>/table-<version>.pb")

    FILE_TYPE_TABLE                 = DBTYPE_NAME("PTBL"),      // Table Data (in "<prinbee-path>/contexts/<context-name>/<table-name>/TBD.pb")
    FILE_TYPE_PRIMARY_INDEX         = DBTYPE_NAME("PIDX"),      // Primary Index (a.k.a. OID Index)
    FILE_TYPE_INDEX                 = DBTYPE_NAME("INDX"),      // User Defined Index (key -> OID)
    FILE_TYPE_BLOOM_FILTER          = DBTYPE_NAME("BLMF"),      // Bloom Filter

    BLOCK_TYPE_BLOB                 = DBTYPE_NAME("BLOB"),
    BLOCK_TYPE_DATA                 = DBTYPE_NAME("DATA"),
    BLOCK_TYPE_ENTRY_INDEX          = DBTYPE_NAME("EIDX"),
    BLOCK_TYPE_FREE_BLOCK           = DBTYPE_NAME("FREE"),
    BLOCK_TYPE_FREE_SPACE           = DBTYPE_NAME("FSPC"),
    BLOCK_TYPE_INDEX_POINTERS       = DBTYPE_NAME("IDXP"),
    BLOCK_TYPE_INDIRECT_INDEX       = DBTYPE_NAME("INDR"),
    BLOCK_TYPE_SECONDARY_INDEX      = DBTYPE_NAME("SIDX"),
    BLOCK_TYPE_SCHEMA_LIST          = DBTYPE_NAME("SCHL"),
    BLOCK_TYPE_TOP_INDEX            = DBTYPE_NAME("TIDX"),
    BLOCK_TYPE_TOP_INDIRECT_INDEX   = DBTYPE_NAME("TIND"),
};


char const *                        to_name(dbtype_t type);


constexpr char const * to_string(dbtype_t type)
{
    switch(type)
    {
    case dbtype_t::DBTYPE_UNKNOWN:
        return "????";

    case dbtype_t::FILE_TYPE_CONTEXT:
        return "CTXT";

    case dbtype_t::FILE_TYPE_TABLE:
        return "PTBL";

    case dbtype_t::FILE_TYPE_INDEX:
        return "INDX";

    case dbtype_t::FILE_TYPE_BLOOM_FILTER:
        return "BLMF";

    case dbtype_t::FILE_TYPE_PRIMARY_INDEX:
        return "PIDX";

    case dbtype_t::FILE_TYPE_COMPLEX_TYPE:
        return "CXTP";

    case dbtype_t::FILE_TYPE_SCHEMA:
        return "SCHM";

    case dbtype_t::BLOCK_TYPE_BLOB:
        return "BLOB";

    case dbtype_t::BLOCK_TYPE_DATA:
        return "DATA";

    case dbtype_t::BLOCK_TYPE_ENTRY_INDEX:
        return "EIDX";

    case dbtype_t::BLOCK_TYPE_FREE_BLOCK:
        return "FREE";

    case dbtype_t::BLOCK_TYPE_FREE_SPACE:
        return "FSPC";

    case dbtype_t::BLOCK_TYPE_INDEX_POINTERS:
        return "IDXP";

    case dbtype_t::BLOCK_TYPE_INDIRECT_INDEX:
        return "INDR";

    case dbtype_t::BLOCK_TYPE_SECONDARY_INDEX:
        return "SIDX";

    case dbtype_t::BLOCK_TYPE_SCHEMA_LIST:
        return "SCHL";

    case dbtype_t::BLOCK_TYPE_TOP_INDEX:
        return "TIDX";

    case dbtype_t::BLOCK_TYPE_TOP_INDIRECT_INDEX:
        return "TIND";

    }

    return "INVL";
}



} // namespace prinbee
// vim: ts=4 sw=4 et
