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


/** \file
 * \brief Various convertions between data types.
 *
 * At this point, we mainly want to convert a structure data type to a string
 * and vice versa. This is useful to convert values defined in the
 * configuration file such as the default value.
 *
 * We also have functions to convert strings to integers of 8, 16, 32, 64,
 * 128, 256, and 512 bits.
 */

// self
//
#include    "prinbee/data/convert.h"


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/hexadecimal_string.h>
#include    <snapdev/math.h>
#include    <snapdev/to_upper.h>
//#include    <snapdev/timestamp.h>
#include    <snapdev/timespec_ex.h>
#include    <snapdev/trim_string.h>


// C++
//
#include    <iostream>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



namespace
{

enum class number_type_t
{
    NUMBER_TYPE_BINARY,
    NUMBER_TYPE_OCTAL,
    NUMBER_TYPE_DECIMAL,
    NUMBER_TYPE_HEXADECIMAL
};


struct name_to_size_multiplicator_t
{
    char const * const  f_name = nullptr;
    std::uint64_t const f_multiplicator[2] = {};
};

#define NAME_TO_SIZE_MULTIPLICATOR(name, lo, hi) \
                        { name, { lo, hi } }

constexpr name_to_size_multiplicator_t const g_size_name_to_multiplicator[] =
{
    // WARNING: Keep in alphabetical order
    //
    // TODO: I remove the word "BYTE[S]" before testing these, but many of
    //       these include a "B" which already means byte(s), i.e. KB / KIB
    //
    NAME_TO_SIZE_MULTIPLICATOR("EB",        1000000000000000000ULL, 0                   ),  // 1000^6
    NAME_TO_SIZE_MULTIPLICATOR("EIB",       0x1000000000000000ULL,  0                   ),  // 2^60 = 1024^6
    NAME_TO_SIZE_MULTIPLICATOR("EXA",       1000000000000000000ULL, 0                   ),  // 1000^6
    NAME_TO_SIZE_MULTIPLICATOR("EXBI",      0x1000000000000000ULL,  0                   ),  // 2^60 = 1024^6
    NAME_TO_SIZE_MULTIPLICATOR("GB",        1'000'000'000ULL,       0                   ),  // 1000^3
    NAME_TO_SIZE_MULTIPLICATOR("GIB",       0x0000000040000000ULL,  0                   ),  // 2^30 = 1024^3
    NAME_TO_SIZE_MULTIPLICATOR("GIBI",      0x0000000040000000ULL,  0                   ),  // 2^30 = 1024^3
    NAME_TO_SIZE_MULTIPLICATOR("GIGA",      1000000000ULL,          0                   ),  // 1000^3
    NAME_TO_SIZE_MULTIPLICATOR("KB",        1000ULL,                0                   ),  // 1000^1
    NAME_TO_SIZE_MULTIPLICATOR("KIB",       0x0000000000000400ULL,  0                   ),  // 2^10 = 1024^1
    NAME_TO_SIZE_MULTIPLICATOR("KIBI",      0x0000000000000400ULL,  0                   ),  // 2^10 = 1024^1
    NAME_TO_SIZE_MULTIPLICATOR("KILO",      1000ULL,                0                   ),  // 1000^1
    NAME_TO_SIZE_MULTIPLICATOR("MB",        1000000ULL,             0                   ),  // 1000^2
    NAME_TO_SIZE_MULTIPLICATOR("MEBI",      0x0000000000100000ULL,  0                   ),  // 2^20 = 1024^2
    NAME_TO_SIZE_MULTIPLICATOR("MEGA",      1000000ULL,             0                   ),  // 1000^2
    NAME_TO_SIZE_MULTIPLICATOR("MIB",       0x0000000000100000ULL,  0                   ),  // 2^20 = 1024^2
    NAME_TO_SIZE_MULTIPLICATOR("PB",        1000000000000000ULL,    0                   ),  // 1000^5
    NAME_TO_SIZE_MULTIPLICATOR("PEBI",      0x0004000000000000ULL,  0                   ),  // 2^50 = 1024^5
    NAME_TO_SIZE_MULTIPLICATOR("PETA",      1000000000000000ULL,    0                   ),  // 1000^5
    NAME_TO_SIZE_MULTIPLICATOR("PIB",       0x0004000000000000ULL,  0                   ),  // 2^50 = 1024^5
    NAME_TO_SIZE_MULTIPLICATOR("QUETTA",    0x4674EDEA40000000,     0x0000000C9F2C9CD0  ),  // 1000^10
    NAME_TO_SIZE_MULTIPLICATOR("QUETTAI",   0,                      0x0000001000000000  ),  // 2^100 = 1024^10
    NAME_TO_SIZE_MULTIPLICATOR("RONNAB",    0x9FD0803CE8000000ULL,  0x00000000033B2E3C  ),  // 1000^9
    NAME_TO_SIZE_MULTIPLICATOR("RONNAIB",   0,                      0x0000000004000000  ),  // 2^90 = 1024^9
    NAME_TO_SIZE_MULTIPLICATOR("TB",        1000000000000ULL,       0                   ),  // 1000^4
    NAME_TO_SIZE_MULTIPLICATOR("TEBI",      0x0000010000000000ULL,  0                   ),  // 2^40 = 1024^4
    NAME_TO_SIZE_MULTIPLICATOR("TERA",      1000000000000ULL,       0                   ),  // 1000^4
    NAME_TO_SIZE_MULTIPLICATOR("TIB",       0x0000010000000000ULL,  0                   ),  // 2^40 = 1024^4
    NAME_TO_SIZE_MULTIPLICATOR("YB",        0x1BCECCEDA1000000,     0x000000000000D3C2  ),  // 1000^8
    NAME_TO_SIZE_MULTIPLICATOR("YIB",       0,                      0x0000000000010000  ),  // 2^80 = 1024^8
    NAME_TO_SIZE_MULTIPLICATOR("YOBI",      0,                      0x0000000000010000  ),  // 2^80 = 1024^8
    NAME_TO_SIZE_MULTIPLICATOR("YOTTA",     0x1BCECCEDA1000000,     0x000000000000D3C2  ),  // 1000^8
    NAME_TO_SIZE_MULTIPLICATOR("ZB",        0x35C9ADC5DEA00000,     0x0000000000000036  ),  // 1000^7
    NAME_TO_SIZE_MULTIPLICATOR("ZEBI",      0,                      0x0000000000000040  ),  // 2^70 = 1024^7
    NAME_TO_SIZE_MULTIPLICATOR("ZETTA",     0x35C9ADC5DEA00000,     0x0000000000000036  ),  // 1000^7
    NAME_TO_SIZE_MULTIPLICATOR("ZIB",       0,                      0x0000000000000040  ),  // 2^70 = 1024^7
};


uint512_t size_to_multiplicator(char const * s)
{
#ifdef _DEBUG
    // verify in debug because if not in order we can't do a binary search
    //
    static int g_checked = false;
    if(!g_checked)
    {
        g_checked = true;
        for(size_t idx(1);
            idx < sizeof(g_size_name_to_multiplicator) / sizeof(g_size_name_to_multiplicator[0]);
            ++idx)
        {
            if(strcmp(g_size_name_to_multiplicator[idx - 1].f_name
                    , g_size_name_to_multiplicator[idx].f_name) >= 0)
            {
                // LCOV_EXCL_START
                throw logic_error(
                          "names in g_name_to_struct_type area not in alphabetical order: "
                        + std::string(g_size_name_to_multiplicator[idx - 1].f_name)
                        + " >= "
                        + g_size_name_to_multiplicator[idx].f_name
                        + " (position: "
                        + std::to_string(idx)
                        + ").");
                // LCOV_EXCL_STOP
            }
        }
    }
#endif

    std::string size(s);
    size = snapdev::to_upper(snapdev::trim_string(size));

    // remove the word "byte[s]" if present
    //
    if(size.length() >= 5
    && size.compare(size.length() - 5, 5, "BYTES") == 0)
    {
        size = snapdev::trim_string(size.substr(0, size.length() - 5));
    }
    else if(size.length() >= 4
         && size.compare(size.length() - 4, 4, "BYTE") == 0)
    {
        size = snapdev::trim_string(size.substr(0, size.length() - 4));
    }

    if(!size.empty())
    {
        int j(sizeof(g_size_name_to_multiplicator) / sizeof(g_size_name_to_multiplicator[0]));
        int i(0);
        while(i < j)
        {
            int const p((j - i) / 2 + i);
            int const r(size.compare(g_size_name_to_multiplicator[p].f_name));
            if(r > 0)
            {
                i = p + 1;
            }
            else if(r < 0)
            {
                j = p;
            }
            else
            {
                return uint512_t{
                        g_size_name_to_multiplicator[p].f_multiplicator[0],
                        g_size_name_to_multiplicator[p].f_multiplicator[1],
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                    };
            }
        }
    }

    uint512_t one;
    one.f_value[0] = 1;
    return one;
}


std::size_t value_byte_size(buffer_t const & value, bool is_signed)
{
    std::size_t size(value.size());
    if(size > 0)
    {
        std::uint8_t const expected(
                   is_signed
                && static_cast<std::int8_t>(value[size - 1]) < 0
                                ? 0xFF
                                : 0x00);
        for(; size > 0; --size)
        {
            if(value[size - 1] != expected)
            {
                if(expected == 0xFF
                && static_cast<std::int8_t>(value[size - 1]) >= 0)
                {
                    // that last byte is not negative so we need one 0xFF
                    // to keep the correct number
                    //
                    if(size != value.size())
                    {
                        ++size;
                    }
                }
                break;
            }
        }
    }

    return size;
}


uint512_t string_to_int(std::string const & number, bool accept_negative_values, unit_t unit)
{
    bool negative(false);
    char const * n(number.c_str());
    while(std::isspace(*n))
    {
        ++n;
    }
    if(*n == '+')
    {
        ++n;
    }
    else if(*n == '-')
    {
        if(!accept_negative_values)
        {
            throw invalid_number(
                      "negative values are not accepted, \""
                    + number
                    + "\" is not valid.");
        }

        ++n;
        negative = true;
    }
    uint512_t result;
    bool expect_quote(false);
    number_type_t t(number_type_t::NUMBER_TYPE_DECIMAL);
    if(*n == '0')
    {
        if(n[1] == 'x'
        || n[1] == 'X')
        {
            n += 2;
            t = number_type_t::NUMBER_TYPE_HEXADECIMAL;
        }
        else if(n[1] == 'b'
             || n[1] == 'B')
        {
            n += 2;
            t = number_type_t::NUMBER_TYPE_BINARY;
        }
        else
        {
            ++n;
            t = number_type_t::NUMBER_TYPE_OCTAL;
        }
    }
    else if(*n == 'x'
         || *n == 'X')
    {
        if(n[1] == '\'')
        {
            n += 2;
            t = number_type_t::NUMBER_TYPE_HEXADECIMAL;
            expect_quote = true;
        }
    }

    uint512_t digit;
    switch(t)
    {
    case number_type_t::NUMBER_TYPE_BINARY:
        while(*n >= '0' && *n <= '1')
        {
            digit.f_value[0] = *n - '0';

            // do `result * 2` with one add
            result += result;       // x2

            result += digit;
            ++n;
        }
        break;

    case number_type_t::NUMBER_TYPE_OCTAL:
        while(*n >= '0' && *n <= '7')
        {
            digit.f_value[0] = *n - '0';

            // do `result * 8` with a few adds
            result += result;       // x2
            result += result;       // x4
            result += result;       // x8

            result += digit;
            ++n;
        }
        break;

    case number_type_t::NUMBER_TYPE_DECIMAL:
        while(*n >= '0' && *n <= '9')
        {
            digit.f_value[0] = *n - '0';

            // do `result * 10` with a few adds
            result += result;       // x2
            uint512_t eight(result);
            eight += eight;         // x4
            eight += eight;         // x8
            result += eight;        // x2 + x8 = x10

            result += digit;
            ++n;
        }
        break;

    case number_type_t::NUMBER_TYPE_HEXADECIMAL:
        for(;;)
        {
            if(*n >= '0' && *n <= '9')
            {
                digit.f_value[0] = *n - '0';
            }
            else if((*n >= 'a' && *n <= 'f') || (*n >= 'A' && *n <= 'F'))
            {
                digit.f_value[0] = (*n & 0x5F) - ('A' - 10);
            }
            else
            {
                break;
            }

            // do `result * 16` with a few adds
            result += result;       // x2
            result += result;       // x4
            result += result;       // x8
            result += result;       // x16

            result += digit;
            ++n;
        }
        break;

    }

    if(expect_quote)
    {
        if(*n != '\'')
        {
            throw invalid_number(
                      "closing quote missing in \""
                    + number
                    + "\".");
        }
        ++n;
    }

    while(std::isspace(*n))
    {
        ++n;
    }

    if(*n != '\0')
    {
        uint512_t multiplicator;
        switch(unit)
        {
        case unit_t::UNIT_NONE:
            throw invalid_number(
                      "could not convert number \""
                    + number
                    + "\" to a valid uint512_t value (spurious data found after number).");

        case unit_t::UNIT_SIZE:
            multiplicator = size_to_multiplicator(n);
            break;

        }

        result *= multiplicator;
    }

    return negative ? -result : result;
}


buffer_t string_to_uinteger(std::string const & value, std::size_t max_size)
{
    buffer_t result;
    uint512_t const n(string_to_int(value, false, unit_t::UNIT_NONE));

    if(max_size != 512 && n.bit_size() > max_size)
    {
        throw out_of_range(
                  "number \""
                + value
                + "\" too large for "
                + (max_size == 8 ? "an " : "a ")
                + std::to_string(max_size)
                + " bit value.");
    }

    result.insert(result.end()
                , reinterpret_cast<uint8_t const *>(&n.f_value)
                , reinterpret_cast<uint8_t const *>(&n.f_value) + max_size / 8);

    return result;
}


std::string uinteger_to_string(buffer_t const & value, int bytes_for_size, int base)
{
    std::size_t const size(value_byte_size(value, false));
    if(size > static_cast<size_t>(bytes_for_size))
    {
        throw out_of_range(
                  "value too large ("
                + std::to_string(value.size() * 8)
                + " bits) for this field (max: "
                + std::to_string(bytes_for_size * 8)
                + " bits).");
    }

    // the uint512::to_string() is optimized so the only penaty here is
    // the memcpy()
    //
    uint512_t v;
    std::memcpy(
          reinterpret_cast<std::uint8_t *>(v.f_value)
        , reinterpret_cast<std::uint8_t const *>(value.data())
        , value.size());

    return v.to_string(base, true, true);
}


buffer_t string_to_integer(std::string const & value, std::size_t max_size)
{
    buffer_t result;
    int512_t const n(string_to_int(value, true, unit_t::UNIT_NONE));

    std::size_t const bit_size(n.bit_size());
    if(max_size != 512 && bit_size > max_size - 1)
    {
        bool is_zero(false);
        if(bit_size == max_size)
        {
            // this is a special case where we may have 0x800..000
            //
            int512_t z(n);
            switch(max_size)
            {
#if 0
// this is not required since we do not enter the if block when max_size != 512
// 512 bit overflows are detected when we call string_to_int()
            case 512:
                z.f_high_value ^= 1LL << 63;
                is_zero = z.f_high_value == 0
                       && z.f_value[0] == 0
                       && z.f_value[1] == 0
                       && z.f_value[2] == 0
                       && z.f_value[3] == 0
                       && z.f_value[4] == 0
                       && z.f_value[5] == 0
                       && z.f_value[6] == 0;
                break;
#endif

            case 256:
                z.f_value[3] ^= 1ULL << 63;
                is_zero = z.f_value[0] == 0
                       && z.f_value[1] == 0
                       && z.f_value[2] == 0
                       && z.f_value[3] == 0;
                break;

            case 128:
                z.f_value[1] ^= 1ULL << 63;
                is_zero = z.f_value[0] == 0
                       && z.f_value[1] == 0;
                break;

            case 64:
                z.f_value[0] ^= 1ULL << 63;
                is_zero = z.f_value[0] == 0;
                break;

            case 32:
                z.f_value[0] ^= ~((1ULL << 31) - 1);
                is_zero = z.f_value[0] == 0;
                break;

            case 16:
                z.f_value[0] ^= ~((1ULL << 15) - 1);
                is_zero = z.f_value[0] == 0;
                break;

            case 8:
                z.f_value[0] ^= ~((1ULL << 7) - 1);
                is_zero = z.f_value[0] == 0;
                break;

            default:                                        // LCOV_EXCL_LINE
                throw logic_error("unexpected bit size.");  // LCOV_EXCL_LINE

            }
        }
        if(!is_zero)
        {
            throw out_of_range(
                      "number \""
                    + value
                    + "\" too large for a signed "
                    + std::to_string(max_size)
                    + " bit value.");
        }
    }

    result.insert(result.end()
                , reinterpret_cast<std::uint8_t const *>(&n.f_value)
                , reinterpret_cast<std::uint8_t const *>(&n.f_value) + max_size / 8);

    return result;
}


std::string integer_to_string(buffer_t const & value, int bytes_for_size, int base)
{
    // WARNING: this first test is only working on little endian computers
    //
    if(static_cast<int8_t>(value.data()[value.size() - 1]) < 0)
    {
        std::size_t const size(value_byte_size(value, true));
        if(size > static_cast<size_t>(bytes_for_size))
        {
            throw out_of_range(
                      "value too large ("
                    + std::to_string(value.size() * 8)
                    + " bits) for this field (max: "
                    + std::to_string(bytes_for_size * 8)
                    + " bits).");
        }

        // TODO: this is a tad bit ugly... (i.e. triple memcpy()!!!)
        //
        int512_t v;
        std::memcpy(v.f_value, value.data(), size);
        if(size < sizeof(v.f_value))
        {
            std::memset(
                      reinterpret_cast<std::uint8_t *>(v.f_value) + size
                    , 255
                    , sizeof(v.f_value) - size);
            v.f_high_value = -1;
        }
        else
        {
            std::size_t const clear(size - sizeof(v.f_value));
            if(clear < sizeof(v.f_high_value))
            {
                std::memset(
                          reinterpret_cast<std::uint8_t *>(&v.f_high_value) + clear
                        , 255
                        , sizeof(v.f_high_value) - clear);
            }
        }
        v = -v;
        buffer_t const neg(reinterpret_cast<uint8_t const *>(v.f_value), reinterpret_cast<uint8_t const *>(v.f_value + 8));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wrestrict"
        return "-" + uinteger_to_string(neg, bytes_for_size, base);
#pragma GCC diagnostic pop
    }
    else
    {
        return uinteger_to_string(value, bytes_for_size, base);
    }
}



template<typename T>
buffer_t string_to_float(std::string const & value, std::function<T(char const *, char **)> f)
{
    buffer_t result;
    char * e(nullptr);
    errno = 0;
    T const r(f(value.c_str(), &e));
    if(errno == ERANGE)
    {
        throw out_of_range(
                  "floating point number \""
                + value
                + "\" out of range.");
    }

    // ignore ending spaces
    //
    while(std::isspace(*e))
    {
        ++e;
    }

    if(*e != '\0')
    {
        throw invalid_number(
                  "floating point number \""
                + value
                + "\" includes invalid characters.");
    }

    result.insert(result.end()
                , reinterpret_cast<std::uint8_t const *>(&r)
                , reinterpret_cast<std::uint8_t const *>(&r) + sizeof(r));

    return result;
}


buffer_t string_to_dbtype(std::string const & value)
{
    if(value.length() != sizeof(dbtype_t))
    {
        throw invalid_type("dbtype must be exactly 4 characters.");
    }

    buffer_t result(4);
    result[0] = value[0];
    result[1] = value[1];
    result[2] = value[2];
    result[3] = value[3];
    return result;
}


template<typename T>
std::string float_to_string(buffer_t const & value)
{
    // TBD: we may want to specify the format
    if(value.size() != sizeof(T))
    {
        throw out_of_range(
                  "value buffer has an unexpected size ("
                + std::to_string(value.size())
                + ") for this field (expected floating point size: "
                + std::to_string(sizeof(T))
                + ").");
    }
    std::ostringstream ss;
    ss << *reinterpret_cast<T const *>(value.data());
    return ss.str();
}


std::string dbtype_to_string(buffer_t const & value)
{
    if(value.size() != sizeof(dbtype_t))
    {
        throw out_of_range(
                  "value buffer has an unexpected size ("
                + std::to_string(value.size())
                + ") for this field (expected magic size: "
                + std::to_string(sizeof(dbtype_t))
                + ").");
    }
    dbtype_t type(dbtype_t::DBTYPE_UNKNOWN);
    memcpy(&type, value.data(), sizeof(dbtype_t));
    return to_string(type);
}


buffer_t string_to_version(std::string const & value)
{
    std::string::size_type const pos(value.find('.'));
    if(pos == std::string::npos)
    {
        throw invalid_parameter(
                  "version \""
                + value
                + "\" must include a period (.) between the major and minor numbers.");
    }

    // allow spaces and a 'v' or 'V' introducer as in '  v1.3'
    //
    std::string::size_type skip(0);
    while(skip < value.length() && std::isspace(value[skip]))
    {
        ++skip;
    }
    if(skip < value.length() && (value[skip] == 'v' || value[skip] == 'V'))
    {
        ++skip;
    }

    std::string const version_major(value.substr(skip, pos - skip));
    std::string const version_minor(value.substr(pos + 1));

    uint512_t const a(string_to_int(version_major, false, unit_t::UNIT_NONE));
    uint512_t const b(string_to_int(version_minor, false, unit_t::UNIT_NONE));

    if(a.bit_size() > 16
    || b.bit_size() > 16)
    {
        throw out_of_range(
                  "one or both of the major or minor numbers from version \""
                + value
                + "\" are too large for a version number (max. is 65535).");
    }

    version_t const v(a.f_value[0], b.f_value[0]);
    std::uint32_t const binary(v.to_binary());

    buffer_t result;
    result.insert(result.end()
                , reinterpret_cast<uint8_t const *>(&binary)
                , reinterpret_cast<uint8_t const *>(&binary) + sizeof(binary));

    return result;
}


std::string version_to_string(buffer_t const & value)
{
    if(value.size() != sizeof(version_t))
    {
        throw out_of_range(
                  "a buffer representing a version must be exactly "
                + std::to_string(sizeof(version_t))
                + " bytes, not "
                + std::to_string(value.size())
                + ".");
    }
    version_t v(*reinterpret_cast<std::uint32_t const *>(value.data()));
    return v.to_string();
}


// At this time I don't think I'd want a c-string anywhere
//buffer_t cstring_to_buffer(std::string const & value)
//{
//    buffer_t result;
//    result.insert(result.end(), value.begin(), value.end());
//
//    // null terminated
//    char zero(0);
//    result.insert(result.end(), reinterpret_cast<uint8_t *>(&zero),  reinterpret_cast<uint8_t *>(&zero) + sizeof(zero));
//
//    return result;
//}
//
//
//std::string buffer_to_cstring(buffer_t const & value)
//{
//    if(value.empty())
//    {
//        throw out_of_range(
//                  "a C-String cannot be saved in an empty buffer ('\\0' missing).");
//    }
//
//    if(value[value.size() - 1] != '\0')
//    {
//        throw out_of_range(
//                  "C-String last byte cannot be anything else than '\\0'.");
//    }
//
//    return std::string(value.data(), value.data() + value.size() - 1);
//}


buffer_t char_to_buffer(std::string const & value, std::size_t size)
{
    if(size == 0)
    {
        throw out_of_range("char_to_buffer(): size out of range, it must be 1 or more.");
    }

    buffer_t result(size);
    memcpy(result.data(), value.data(), value.length());
    if(value.length() < size)
    {
        memset(result.data() + value.length(), 0, size - value.length());
    }
    return result;
}


buffer_t string_to_buffer(std::string const & value, std::size_t bytes_for_size)
{
#ifdef _DEBUG
    switch(bytes_for_size)
    {
    case 1:
    case 2:
    case 4:
        break;

    default:                                                                                    // LCOV_EXCL_LINE
        throw logic_error("string_to_buffer(): bytes_for_size must be one of 1, 2, or 4.");     // LCOV_EXCL_LINE

    }
#endif

    std::uint32_t size(value.length());

    std::uint64_t const max_size(1ULL << bytes_for_size * 8);

    if(size >= max_size)
    {
        throw out_of_range(
                  "string too long ("
                + std::to_string(size)
                + ") for this field (max: "
                + std::to_string(max_size - 1)
                + ").");
    }

    // WARNING: this copy works in Little Endian only
    //
    buffer_t result;
    result.insert(
              result.end()
            , reinterpret_cast<uint8_t *>(&size)
            , reinterpret_cast<uint8_t *>(&size) + bytes_for_size);

    result.insert(result.end(), value.begin(), value.end());

    return result;
}


std::string buffer_to_char(buffer_t const & value, std::size_t size)
{
    if(value.size() < size)
    {
        throw out_of_range(
                  "buffer too small for the CHAR string (size: "
                + std::to_string(size)
                + ", character bytes in buffer: "
                + std::to_string(value.size())
                + ").");
    }

    // field may have zeroes if string is smaller than size
    //
    std::size_t const len(strnlen(reinterpret_cast<char const *>(value.data()), size));
    return std::string(value.data(), value.data() + len);
}


std::string buffer_to_string(buffer_t const & value, std::size_t bytes_for_size)
{
    if(value.size() < bytes_for_size)
    {
        throw out_of_range(
                  "buffer too small to incorporate the P-String size ("
                + std::to_string(value.size())
                + ", expected at least: "
                + std::to_string(bytes_for_size)
                + ").");
    }

    std::uint32_t size(0);
    memcpy(&size, value.data(), bytes_for_size);

    if(bytes_for_size + size > value.size())
    {
        throw out_of_range(
                  "buffer too small for the P-String characters (size: "
                + std::to_string(size)
                + ", character bytes in buffer: "
                + std::to_string(value.size() - bytes_for_size)
                + ").");
    }

    return std::string(
                  value.data() + bytes_for_size
                , value.data() + bytes_for_size + size);
}


// TODO: add support for getdate()
buffer_t string_to_unix_time(std::string const & value, std::uint32_t fraction_exp)
{
    struct tm t = {};
    std::string format("%Y-%m-%dT%T");
    std::string::size_type const pos(value.find('.'));
    int f(0);
    if(pos != std::string::npos)
    {
        std::string date_time(value.substr(0, pos));
        std::string::size_type zone(value.find_first_of("+-", pos));
        if(zone == std::string::npos)
        {
            zone = value.size();
        }
        else
        {
            format += "%z";
            date_time += value.substr(zone);
        }
        std::string frac(value.substr(pos + 1, zone - pos - 1));
        while(frac.length() > fraction_exp && frac[frac.length() - 1] == '0')
        {
            frac.resize(frac.length() - 1);
        }
        int const l(fraction_exp - frac.length());
        if(l < 0)
        {
            throw out_of_range(
                      "time fraction is out of bounds in \""
                    + value
                    + "\" (expected "
                    + std::to_string(fraction_exp)
                    + " digits, found "
                    + std::to_string(frac.length())
                    + ").");
        }
        if(l > 0)
        {
            frac += std::string(l, '0');
        }
        f = std::atoi(frac.c_str());

        strptime(date_time.c_str(), format.c_str(), &t);
    }
    else
    {
        std::string::size_type const zone(value.find_first_of("+-"));
        if(zone != std::string::npos)
        {
            format += "%z";
        }

        strptime(value.c_str(), format.c_str(), &t);
    }

    // the mktime() function takes the input as local time, we want UTC
    // so instead we use timegm()
    //
    // you can use this one instead if you do not have timegm()
    //time_t const v(snapdev::unix_timestamp(
    //          t.tm_year + 1900
    //        , t.tm_mon + 1
    //        , t.tm_mday
    //        , t.tm_hour
    //        , t.tm_min
    //        , t.tm_sec));
    time_t const v(timegm(&t));
    std::uint64_t const with_fraction(v * snapdev::pow(10, fraction_exp) + f);

    return buffer_t(reinterpret_cast<uint8_t const *>(&with_fraction), reinterpret_cast<uint8_t const *>(&with_fraction + 1));
}


buffer_t string_to_ns_time(std::string const & value)
{
    snapdev::timespec_ex time(value);
    return buffer_t(reinterpret_cast<std::uint8_t const *>(&time), reinterpret_cast<std::uint8_t const *>(&time + 1));
}


std::string unix_time_to_string(buffer_t const & value, std::int64_t fraction)
{
    std::uint64_t time;
    if(value.size() != sizeof(time))
    {
        throw out_of_range(
                  "buffer size is invalid for a time value (size: "
                + std::to_string(value.size())
                + ", expected size: "
                + std::to_string(sizeof(time))
                + ").");
    }
    memcpy(&time, value.data(), sizeof(time));
    time_t const v(time / fraction);
    struct tm t;
    gmtime_r(&v, &t);

    char buf[256];
    strftime(buf, sizeof(buf) - 1, "%FT%T", &t);
    buf[sizeof(buf) - 1] = '\0';

    std::string result(buf);

    if(fraction != 1)
    {
        result += ".";
        std::string frac(std::to_string(time % fraction));
        std::size_t const sz(fraction == 1'000 ? 3 : 6);
        result += std::string(sz - frac.size(), '0');
        result += frac;
    }

    return result + "+0000";
}


std::string ns_time_to_string(buffer_t const & value)
{
    snapdev::timespec_ex time;
    if(value.size() != sizeof(time))
    {
        throw out_of_range(
                  "buffer size is invalid for a time value with nanoseconds (size: "
                + std::to_string(value.size())
                + ", expected size: "
                + std::to_string(sizeof(time))
                + ").");
    }
    memcpy(static_cast<timespec *>(&time), value.data(), sizeof(time));
    return time.to_timestamp(true);
}


buffer_t string_to_pbuffer(std::string const & value, std::uint32_t bytes_for_size)
{
    std::string const bin(snapdev::hex_to_bin(value));
    std::size_t const size(bin.size());
    if(size >= (1ULL << bytes_for_size * 8))
    {
        throw out_of_range(
                  "number of bytes in value is too large ("
                + std::to_string(size)
                + ") for a buffer"
                + std::to_string(bytes_for_size * 8)
                + ".");
    }
    buffer_t result(bytes_for_size + size);
    memcpy(result.data(), &size, bytes_for_size);
    memcpy(result.data() + bytes_for_size, bin.data(), size);
    return result;
}


std::string pbuffer_to_string(buffer_t const & value, std::size_t bytes_for_size)
{
    if(value.size() < bytes_for_size)
    {
        throw out_of_range(
                  "buffer too small to incorporate the P-Buffer size ("
                + std::to_string(value.size())
                + ", expected at least: "
                + std::to_string(bytes_for_size)
                + ").");
    }

    std::uint32_t size(0);
    memcpy(&size, value.data(), bytes_for_size);
    if(bytes_for_size + size > value.size())
    {
        throw out_of_range(
                  "buffer (size: "
                + std::to_string(value.size())
                + " including "
                + std::to_string(bytes_for_size)
                + " bytes for the size) too small for the requested number of bytes ("
                + std::to_string(bytes_for_size + size)
                + ").");
    }

    return snapdev::bin_to_hex(std::string(value.begin() + bytes_for_size, value.begin() + bytes_for_size + size));
}


} // no name namespace





buffer_t string_to_typed_buffer(struct_type_t type, std::string const & value, std::size_t size)
{
    switch(type)
    {
    case struct_type_t::STRUCT_TYPE_BITS8:
    case struct_type_t::STRUCT_TYPE_UINT8:
        return string_to_uinteger(value, 8);

    case struct_type_t::STRUCT_TYPE_BITS16:
    case struct_type_t::STRUCT_TYPE_UINT16:
        return string_to_uinteger(value, 16);

    case struct_type_t::STRUCT_TYPE_BITS32:
    case struct_type_t::STRUCT_TYPE_UINT32:
        return string_to_uinteger(value, 32);

    case struct_type_t::STRUCT_TYPE_BITS64:
    case struct_type_t::STRUCT_TYPE_UINT64:
    case struct_type_t::STRUCT_TYPE_OID:
    case struct_type_t::STRUCT_TYPE_REFERENCE:
        return string_to_uinteger(value, 64);

    case struct_type_t::STRUCT_TYPE_BITS128:
    case struct_type_t::STRUCT_TYPE_UINT128:
        return string_to_uinteger(value, 128);

    case struct_type_t::STRUCT_TYPE_BITS256:
    case struct_type_t::STRUCT_TYPE_UINT256:
        return string_to_uinteger(value, 256);

    case struct_type_t::STRUCT_TYPE_BITS512:
    case struct_type_t::STRUCT_TYPE_UINT512:
        return string_to_uinteger(value, 512);

    case struct_type_t::STRUCT_TYPE_INT8:
        return string_to_integer(value, 8);

    case struct_type_t::STRUCT_TYPE_INT16:
        return string_to_integer(value, 16);

    case struct_type_t::STRUCT_TYPE_INT32:
        return string_to_integer(value, 32);

    case struct_type_t::STRUCT_TYPE_INT64:
        return string_to_integer(value, 64);

    case struct_type_t::STRUCT_TYPE_INT128:
        return string_to_integer(value, 128);

    case struct_type_t::STRUCT_TYPE_INT256:
        return string_to_integer(value, 256);

    case struct_type_t::STRUCT_TYPE_INT512:
        return string_to_integer(value, 512);

    case struct_type_t::STRUCT_TYPE_FLOAT32:
        return string_to_float<float>(value, std::strtof);

    case struct_type_t::STRUCT_TYPE_FLOAT64:
        return string_to_float<double>(value, std::strtod);

    case struct_type_t::STRUCT_TYPE_FLOAT128:
        return string_to_float<long double>(value, std::strtold);

    case struct_type_t::STRUCT_TYPE_MAGIC:
        return string_to_dbtype(value);

    case struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION:
    case struct_type_t::STRUCT_TYPE_VERSION:
        return string_to_version(value);

    case struct_type_t::STRUCT_TYPE_TIME:
        return string_to_unix_time(value, 0);

    case struct_type_t::STRUCT_TYPE_MSTIME:
        return string_to_unix_time(value, 3);

    case struct_type_t::STRUCT_TYPE_USTIME:
        return string_to_unix_time(value, 6);

    case struct_type_t::STRUCT_TYPE_NSTIME:
        return string_to_ns_time(value);

    case struct_type_t::STRUCT_TYPE_CHAR:
        return char_to_buffer(value, size);

    case struct_type_t::STRUCT_TYPE_P8STRING:
        return string_to_buffer(value, 1);

    case struct_type_t::STRUCT_TYPE_P16STRING:
        return string_to_buffer(value, 2);

    case struct_type_t::STRUCT_TYPE_P32STRING:
        return string_to_buffer(value, 4);

    case struct_type_t::STRUCT_TYPE_BUFFER8:
        return string_to_pbuffer(value, 1);

    case struct_type_t::STRUCT_TYPE_BUFFER16:
        return string_to_pbuffer(value, 2);

    case struct_type_t::STRUCT_TYPE_BUFFER32:
        return string_to_pbuffer(value, 4);

    default:
        //struct_type_t::STRUCT_TYPE_ARRAY8:
        //struct_type_t::STRUCT_TYPE_ARRAY16:
        //struct_type_t::STRUCT_TYPE_ARRAY32:
        //struct_type_t::STRUCT_TYPE_STRUCTURE:
        //struct_type_t::STRUCT_TYPE_END
        //struct_type_t::STRUCT_TYPE_VOID
        //struct_type_t::STRUCT_TYPE_RENAMED
        throw logic_error(
              "unexpected structure type ("
            + std::to_string(static_cast<int>(type))
            + ") to convert a string to a buffer.");

    }
}


std::string typed_buffer_to_string(struct_type_t type, buffer_t const & value, int base_or_size)
{
    switch(type)
    {
    case struct_type_t::STRUCT_TYPE_BITS8:
    case struct_type_t::STRUCT_TYPE_UINT8:
        return uinteger_to_string(value, 1, base_or_size);

    case struct_type_t::STRUCT_TYPE_BITS16:
    case struct_type_t::STRUCT_TYPE_UINT16:
        return uinteger_to_string(value, 2, base_or_size);

    case struct_type_t::STRUCT_TYPE_BITS32:
    case struct_type_t::STRUCT_TYPE_UINT32:
        return uinteger_to_string(value, 4, base_or_size);

    case struct_type_t::STRUCT_TYPE_BITS64:
    case struct_type_t::STRUCT_TYPE_UINT64:
    case struct_type_t::STRUCT_TYPE_REFERENCE:
    case struct_type_t::STRUCT_TYPE_OID:
        return uinteger_to_string(value, 8, base_or_size);

    case struct_type_t::STRUCT_TYPE_BITS128:
    case struct_type_t::STRUCT_TYPE_UINT128:
        return uinteger_to_string(value, 16, base_or_size);

    case struct_type_t::STRUCT_TYPE_BITS256:
    case struct_type_t::STRUCT_TYPE_UINT256:
        return uinteger_to_string(value, 32, base_or_size);

    case struct_type_t::STRUCT_TYPE_BITS512:
    case struct_type_t::STRUCT_TYPE_UINT512:
        return uinteger_to_string(value, 64, base_or_size);

    case struct_type_t::STRUCT_TYPE_INT8:
        return integer_to_string(value, 1, base_or_size);

    case struct_type_t::STRUCT_TYPE_INT16:
        return integer_to_string(value, 2, base_or_size);

    case struct_type_t::STRUCT_TYPE_INT32:
        return integer_to_string(value, 4, base_or_size);

    case struct_type_t::STRUCT_TYPE_INT64:
        return integer_to_string(value, 8, base_or_size);

    case struct_type_t::STRUCT_TYPE_INT128:
        return integer_to_string(value, 16, base_or_size);

    case struct_type_t::STRUCT_TYPE_INT256:
        return integer_to_string(value, 32, base_or_size);

    case struct_type_t::STRUCT_TYPE_INT512:
        return integer_to_string(value, 64, base_or_size);

    case struct_type_t::STRUCT_TYPE_FLOAT32:
        return float_to_string<float>(value);

    case struct_type_t::STRUCT_TYPE_FLOAT64:
        return float_to_string<double>(value);

    case struct_type_t::STRUCT_TYPE_FLOAT128:
        return float_to_string<long double>(value);

    case struct_type_t::STRUCT_TYPE_MAGIC:
        return dbtype_to_string(value);

    case struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION:
    case struct_type_t::STRUCT_TYPE_VERSION:
        return version_to_string(value);

    case struct_type_t::STRUCT_TYPE_TIME:
        return unix_time_to_string(value, 1);

    case struct_type_t::STRUCT_TYPE_MSTIME:
        return unix_time_to_string(value, 1'000);

    case struct_type_t::STRUCT_TYPE_USTIME:
        return unix_time_to_string(value, 1'000'000);

    case struct_type_t::STRUCT_TYPE_NSTIME:
        return ns_time_to_string(value);

    case struct_type_t::STRUCT_TYPE_CHAR:
        return buffer_to_char(value, base_or_size);

    case struct_type_t::STRUCT_TYPE_P8STRING:
        return buffer_to_string(value, 1);

    case struct_type_t::STRUCT_TYPE_P16STRING:
        return buffer_to_string(value, 2);

    case struct_type_t::STRUCT_TYPE_P32STRING:
        return buffer_to_string(value, 4);

    case struct_type_t::STRUCT_TYPE_BUFFER8:
        return pbuffer_to_string(value, 1);

    case struct_type_t::STRUCT_TYPE_BUFFER16:
        return pbuffer_to_string(value, 2);

    case struct_type_t::STRUCT_TYPE_BUFFER32:
        return pbuffer_to_string(value, 4);

    default:
        //struct_type_t::STRUCT_TYPE_STRUCTURE:
        //struct_type_t::STRUCT_TYPE_ARRAY8:
        //struct_type_t::STRUCT_TYPE_ARRAY16:
        //struct_type_t::STRUCT_TYPE_ARRAY32:
        //struct_type_t::STRUCT_TYPE_END
        //struct_type_t::STRUCT_TYPE_VOID
        //struct_type_t::STRUCT_TYPE_RENAMED
        throw logic_error(
              "unexpected structure type ("
            + std::to_string(static_cast<int>(type))
            + ") to convert a string to a buffer.");

    }
}


std::int64_t convert_to_int(std::string const & value, size_t max_size, unit_t unit)
{
    int512_t const n(string_to_int(value, true, unit));

    if(n.bit_size() > max_size)
    {
        throw out_of_range(
                  "number \""
                + value
                + "\" too large for a signed "
                + std::to_string(max_size)
                + " bit value.");
    }

    return n.f_value[0];
}


std::uint64_t convert_to_uint(std::string const & value, size_t max_size, unit_t unit)
{
    uint512_t const n(string_to_int(value, false, unit));

    if(n.bit_size() > max_size)
    {
        throw out_of_range(
                  "number \""
                + value
                + "\" too large for an unsigned "
                + std::to_string(max_size)
                + " bit value.");
    }

    return n.f_value[0];
}



} // namespace prinbee
// vim: ts=4 sw=4 et
