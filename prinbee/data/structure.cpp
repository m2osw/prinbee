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
#include    "prinbee/data/structure.h"

#include    "prinbee/data/convert.h"
#include    "prinbee/utils.h"


// advgetopt
//
#include    <advgetopt/validator_integer.h>


// snaplogger
//
#include    <snaplogger/message.h>


// cppthread
//
#include    <cppthread/thread.h>


// snapdev
//
#include    <snapdev/hexadecimal_string.h>
#include    <snapdev/timespec_ex.h>
#include    <snapdev/to_upper.h>


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


struct name_to_struct_type_t
{
    char const *        f_name = nullptr;
    struct_type_t       f_type = struct_type_t::STRUCT_TYPE_END;
};

#define NAME_TO_STRUCT_TYPE(name)   { #name, struct_type_t::STRUCT_TYPE_##name }

name_to_struct_type_t g_name_to_struct_type[] =
{
    // WARNING: Keep in alphabetical order
    //
    NAME_TO_STRUCT_TYPE(ARRAY16),
    NAME_TO_STRUCT_TYPE(ARRAY32),
    NAME_TO_STRUCT_TYPE(ARRAY8),
    NAME_TO_STRUCT_TYPE(BITS128),
    NAME_TO_STRUCT_TYPE(BITS16),
    NAME_TO_STRUCT_TYPE(BITS256),
    NAME_TO_STRUCT_TYPE(BITS32),
    NAME_TO_STRUCT_TYPE(BITS512),
    NAME_TO_STRUCT_TYPE(BITS64),
    NAME_TO_STRUCT_TYPE(BITS8),
    NAME_TO_STRUCT_TYPE(BUFFER16),
    NAME_TO_STRUCT_TYPE(BUFFER32),
    NAME_TO_STRUCT_TYPE(BUFFER8),
    NAME_TO_STRUCT_TYPE(CHAR),
    NAME_TO_STRUCT_TYPE(END),       // to end a list
    NAME_TO_STRUCT_TYPE(FLOAT128),
    NAME_TO_STRUCT_TYPE(FLOAT32),
    NAME_TO_STRUCT_TYPE(FLOAT64),
    NAME_TO_STRUCT_TYPE(INT128),
    NAME_TO_STRUCT_TYPE(INT16),
    NAME_TO_STRUCT_TYPE(INT256),
    NAME_TO_STRUCT_TYPE(INT32),
    NAME_TO_STRUCT_TYPE(INT512),
    NAME_TO_STRUCT_TYPE(INT64),
    NAME_TO_STRUCT_TYPE(INT8),
    NAME_TO_STRUCT_TYPE(MAGIC),
    NAME_TO_STRUCT_TYPE(MSTIME),
    NAME_TO_STRUCT_TYPE(NSTIME),
    NAME_TO_STRUCT_TYPE(OID),
    NAME_TO_STRUCT_TYPE(P16STRING),
    NAME_TO_STRUCT_TYPE(P32STRING),
    NAME_TO_STRUCT_TYPE(P8STRING),
    NAME_TO_STRUCT_TYPE(REFERENCE),
    NAME_TO_STRUCT_TYPE(RENAMED),
    NAME_TO_STRUCT_TYPE(STRUCTURE),
    NAME_TO_STRUCT_TYPE(STRUCTURE_VERSION),
    NAME_TO_STRUCT_TYPE(TIME),
    NAME_TO_STRUCT_TYPE(UINT128),
    NAME_TO_STRUCT_TYPE(UINT16),
    NAME_TO_STRUCT_TYPE(UINT256),
    NAME_TO_STRUCT_TYPE(UINT32),
    NAME_TO_STRUCT_TYPE(UINT512),
    NAME_TO_STRUCT_TYPE(UINT64),
    NAME_TO_STRUCT_TYPE(UINT8),
    NAME_TO_STRUCT_TYPE(USTIME),
    NAME_TO_STRUCT_TYPE(VERSION),
    NAME_TO_STRUCT_TYPE(VOID)
};






struct field_sizes_t
{
    ssize_t             f_size = 0;
    ssize_t             f_field_size = 0;
};


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
constexpr field_sizes_t const g_struct_type_sizes[] =
{
    [static_cast<int>(struct_type_t::STRUCT_TYPE_END)]                  = { INVALID_SIZE,                 0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_VOID)]                 = { 0,                            0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_BITS8)]                = { sizeof(std::uint8_t),         0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_BITS16)]               = { sizeof(std::uint16_t),        0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_BITS32)]               = { sizeof(std::uint32_t),        0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_BITS64)]               = { sizeof(std::uint64_t),        0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_BITS128)]              = { sizeof(std::uint64_t) * 2,    0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_BITS256)]              = { sizeof(std::uint64_t) * 4,    0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_BITS512)]              = { sizeof(std::uint64_t) * 8,    0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_INT8)]                 = { sizeof(std::int8_t),          0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_UINT8)]                = { sizeof(std::uint8_t),         0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_INT16)]                = { sizeof(std::int16_t),         0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_UINT16)]               = { sizeof(std::uint16_t),        0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_INT32)]                = { sizeof(std::int32_t),         0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_UINT32)]               = { sizeof(std::uint32_t),        0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_INT64)]                = { sizeof(std::int64_t),         0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_UINT64)]               = { sizeof(std::uint64_t),        0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_INT128)]               = { sizeof(std::int64_t) * 2,     0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_UINT128)]              = { sizeof(std::uint64_t) * 2,    0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_INT256)]               = { sizeof(std::int64_t) * 4,     0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_UINT256)]              = { sizeof(std::uint64_t) * 4,    0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_INT512)]               = { sizeof(std::int64_t) * 8,     0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_UINT512)]              = { sizeof(std::uint64_t) * 8,    0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_FLOAT32)]              = { sizeof(float),                0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_FLOAT64)]              = { sizeof(double),               0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_FLOAT128)]             = { sizeof(long double),          0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_MAGIC)]                = { sizeof(std::uint32_t),        0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)]    = { sizeof(std::uint32_t),        0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_VERSION)]              = { sizeof(std::uint32_t),        0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_TIME)]                 = { sizeof(time_t),               0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_MSTIME)]               = { sizeof(std::int64_t),         0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_USTIME)]               = { sizeof(std::int64_t),         0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_NSTIME)]               = { sizeof(snapdev::timespec_ex), 0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_CHAR)]                 = { INVALID_SIZE,                 0 }, // must be handled specially
    [static_cast<int>(struct_type_t::STRUCT_TYPE_P8STRING)]             = { VARIABLE_SIZE,                1 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_P16STRING)]            = { VARIABLE_SIZE,                2 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_P32STRING)]            = { VARIABLE_SIZE,                4 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_STRUCTURE)]            = { VARIABLE_SIZE,                0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_ARRAY8)]               = { VARIABLE_SIZE,                1 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_ARRAY16)]              = { VARIABLE_SIZE,                2 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_ARRAY32)]              = { VARIABLE_SIZE,                4 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_BUFFER8)]              = { VARIABLE_SIZE,                1 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_BUFFER16)]             = { VARIABLE_SIZE,                2 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_BUFFER32)]             = { VARIABLE_SIZE,                4 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_REFERENCE)]            = { sizeof(std::uint64_t),        0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_OID)]                  = { sizeof(std::uint64_t),        0 },
    [static_cast<int>(struct_type_t::STRUCT_TYPE_RENAMED)]              = { INVALID_SIZE,                 0 }
};
#pragma GCC diagnostic pop


void verify_size(struct_type_t type, std::size_t size)
{
    if(static_cast<std::size_t>(type) >= std::size(g_struct_type_sizes))
    {
        // LCOV_EXCL_START
        throw out_of_range(
                  "type out of range for converting it to a size ("
                + to_string(type)
                + ", max: "
                + std::to_string(std::size(g_struct_type_sizes))
                + ").");
        // LCOV_EXCL_STOP
    }

    if(g_struct_type_sizes[static_cast<int>(type)].f_size != static_cast<ssize_t>(size))
    {
        // LCOV_EXCL_START
        throw out_of_range(
                  "value ("
                + std::to_string(size)
                + ") and type ("
                + to_string(type)
                + ") sizes do not correspond (expected size: "
                + std::to_string(g_struct_type_sizes[static_cast<int>(type)].f_size)
                + ").");
        // LCOV_EXCL_STOP
    }
}



}
// no name namespace






std::string to_string(struct_type_t const & type)
{
    for(std::size_t idx(0);
        idx < std::size(g_name_to_struct_type);
        ++idx)
    {
        if(g_name_to_struct_type[idx].f_type == type)
        {
            return g_name_to_struct_type[idx].f_name;
        }
    }

    return std::string("*unknown struct type (" + std::to_string(static_cast<int>(type)) + ")*");
}


struct_type_t name_to_struct_type(std::string const & type_name)
{
#ifdef _DEBUG
    // verify in debug because if not in order we can't do a binary search
    for(size_t idx(1);
        idx < std::size(g_name_to_struct_type);
        ++idx)
    {
        if(strcmp(g_name_to_struct_type[idx - 1].f_name
                , g_name_to_struct_type[idx].f_name) >= 0)
        {
            // LCOV_EXCL_START
            throw logic_error(
                      "names in g_name_to_struct_type area not in alphabetical order: "
                    + std::string(g_name_to_struct_type[idx - 1].f_name)
                    + " >= "
                    + g_name_to_struct_type[idx].f_name
                    + " (position: "
                    + std::to_string(idx)
                    + ").");
            // LCOV_EXCL_STOP
        }
    }
#endif

    std::string const uc(snapdev::to_upper(type_name));

    int j(std::size(g_name_to_struct_type));
    int i(0);
    while(i < j)
    {
        int const p((j - i) / 2 + i);
        int const r(uc.compare(g_name_to_struct_type[p].f_name));
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
            return g_name_to_struct_type[p].f_type;
        }
    }

    return INVALID_STRUCT_TYPE;
}


/** \brief Check whether \p type represents a fixed size type.
 *
 * This function returns true if the \p type parameter represents a
 * type which will never change in size. However, a row can still change
 * in size even if all of its columns have a fixed size since most columns
 * are optional (which saves space if you do not include them).
 *
 * So we do not offer a way to determine whether a schema is fixed or not
 * because some of the system columns are optional and as such it means
 * that all tables have rows of varying sizes even if your own columns are
 * all marked as mandatory and are of fixed size.
 *
 * \return true if the type represents a fixed size.
 */
bool type_with_fixed_size(struct_type_t type)
{
    switch(type)
    {
    case struct_type_t::STRUCT_TYPE_END:
    case struct_type_t::STRUCT_TYPE_VOID:
    case struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION:
    case struct_type_t::STRUCT_TYPE_BITS8:
    case struct_type_t::STRUCT_TYPE_BITS16:
    case struct_type_t::STRUCT_TYPE_BITS32:
    case struct_type_t::STRUCT_TYPE_BITS64:
    case struct_type_t::STRUCT_TYPE_BITS128:
    case struct_type_t::STRUCT_TYPE_BITS256:
    case struct_type_t::STRUCT_TYPE_BITS512:
    case struct_type_t::STRUCT_TYPE_INT8:
    case struct_type_t::STRUCT_TYPE_UINT8:
    case struct_type_t::STRUCT_TYPE_INT16:
    case struct_type_t::STRUCT_TYPE_UINT16:
    case struct_type_t::STRUCT_TYPE_INT32:
    case struct_type_t::STRUCT_TYPE_UINT32:
    case struct_type_t::STRUCT_TYPE_INT64:
    case struct_type_t::STRUCT_TYPE_UINT64:
    case struct_type_t::STRUCT_TYPE_INT128:
    case struct_type_t::STRUCT_TYPE_UINT128:
    case struct_type_t::STRUCT_TYPE_INT256:
    case struct_type_t::STRUCT_TYPE_UINT256:
    case struct_type_t::STRUCT_TYPE_INT512:
    case struct_type_t::STRUCT_TYPE_UINT512:
    case struct_type_t::STRUCT_TYPE_FLOAT32:
    case struct_type_t::STRUCT_TYPE_FLOAT64:
    case struct_type_t::STRUCT_TYPE_FLOAT128:
    case struct_type_t::STRUCT_TYPE_VERSION:
    case struct_type_t::STRUCT_TYPE_TIME:
    case struct_type_t::STRUCT_TYPE_MSTIME:
    case struct_type_t::STRUCT_TYPE_USTIME:
    case struct_type_t::STRUCT_TYPE_NSTIME:
    case struct_type_t::STRUCT_TYPE_REFERENCE:
    case struct_type_t::STRUCT_TYPE_OID:
    case struct_type_t::STRUCT_TYPE_RENAMED:
        return true;

    default:
        return false;

    }
}



flag_definition::flag_definition(
          std::string const & field_name
        , std::string const & flag_name
        , std::size_t pos
        , std::size_t size)
    : f_field_name(field_name)
    , f_flag_name(flag_name)
    , f_pos(pos)
    , f_size(size)
    , f_mask(((1ULL << size) - 1ULL) << pos)  // fails if size == 64...
{
    if(field_name.empty()
    || flag_name.empty())
    {
        throw invalid_parameter("the flag definition must have a non-empty field name and flag name.");
    }

    if(size == 0ULL)
    {
        throw invalid_parameter(
                  "bit field named \""
                + field_name
                + "."
                + flag_name
                + "\" cannot have a size of 0.");
    }
    if(size >= 64ULL)
    {
        throw invalid_parameter(
                  "bit field named \""
                + field_name
                + "."
                + flag_name
                + "\" is too large ("
                + std::to_string(size)
                + " >= 64).");
    }
    if(pos >= 64ULL || pos + size > 64ULL)
    {
        throw invalid_parameter(
                  "the mask of the bit field named \""
                + field_name
                + "."
                + flag_name
                + "\" does not fit in a uint64_t.");
    }
}


std::string flag_definition::full_name() const
{
    return f_field_name + "." + f_flag_name;
}


std::string flag_definition::field_name() const
{
    return f_field_name;
}


std::string flag_definition::flag_name() const
{
    return f_flag_name;
}


std::size_t flag_definition::pos() const
{
    return f_pos;
}


std::size_t flag_definition::size() const
{
    return f_size;
}


flags_t flag_definition::mask() const
{
    return f_mask;
}






/** \brief Create a field.
 *
 * This function initialize a field. The description doesn't get parsed
 * right away, however, the name is checked for validity when in debug
 * mode.
 *
 * A field name must match the following pattern:
 *
 * \code
 * [A-Za-z_][A-Za-z0-9_]*
 * \endcode
 *
 * If the name is in a bit field, then the matching changes to accept
 * a set of flag definitions:
 *
 * \code
 * [A-Za-z_][A-Za-z0-9_]*=([A-Za-z_][A-Za-z0-9_]*)*[A-Za-z_][A-Za-z0-9_]
 * \endcode
 *
 * Similarly, a field of type CHAR has to include a size which means
 * it has a pattern like this:
 *
 * \code
 * [A-Za-z_][A-Za-z0-9_]*=[0-9_]+
 * \endcode
 *
 * This will allow us to create languages where those field names can
 * easily be used (as in a language such as pbql).
 *
 * \param[in] description  The field description. This pointer data is not
 * expected to ever change. As a result, the field saves it as is.
 */
field_t::field_t(struct_description_t const * description)
    : f_description(description)
{
#ifdef _DEBUG
    // field names are defined statically so we don't really need to
    // verify them in a runtime environment; but do that in debug mode
    //
    if(f_description->f_field_name == nullptr)
    {
        throw logic_error("a field must have a name, null is not valid.");
    }
    if(*f_description->f_field_name == '\0')
    {
        throw logic_error("a field must have a name, an empty string (\"\") is not valid.");
    }
    switch(f_description->f_type)
    {
    case struct_type_t::STRUCT_TYPE_BITS8:
    case struct_type_t::STRUCT_TYPE_BITS16:
    case struct_type_t::STRUCT_TYPE_BITS32:
    case struct_type_t::STRUCT_TYPE_BITS64:
    case struct_type_t::STRUCT_TYPE_BITS128:
    case struct_type_t::STRUCT_TYPE_BITS256:
    case struct_type_t::STRUCT_TYPE_BITS512:
        if(!validate_bit_field_name(f_description->f_field_name))
        {
            throw logic_error(
                  "bit field name & definition \""
                + std::string(f_description->f_field_name)
                + "\" are not valid.");
        }
        break;

    case struct_type_t::STRUCT_TYPE_CHAR:
        if(!validate_char_field_name(f_description->f_field_name))
        {
            throw logic_error(
                  "char field name & length \""
                + std::string(f_description->f_field_name)
                + "\" are not valid.");
        }
        break;

    default:
        if(!validate_name(f_description->f_field_name))
        {
            throw logic_error(
                  "field name \""
                + std::string(f_description->f_field_name)
                + "\" is not valid (unsupported characters).");
        }
        break;

    }
#endif

    switch(f_description->f_type)
    {
    case struct_type_t::STRUCT_TYPE_BITS8:
    case struct_type_t::STRUCT_TYPE_BITS16:
    case struct_type_t::STRUCT_TYPE_BITS32:
    case struct_type_t::STRUCT_TYPE_BITS64:
    case struct_type_t::STRUCT_TYPE_BITS128:
    case struct_type_t::STRUCT_TYPE_BITS256:
    case struct_type_t::STRUCT_TYPE_BITS512:
        {
            char const * const equal(strchrnul(f_description->f_field_name, '='));
            f_field_name_len = equal - f_description->f_field_name;
        }
        break;

    case struct_type_t::STRUCT_TYPE_CHAR:
        {
            // immediately retrieve the size from the field name
            //
            char const * const equal(strchr(f_description->f_field_name, '='));
            if(equal == nullptr)
            {
                // Coverage in _DEBUG cannot hit this throw because the
                // name validation above prevents it
                //
                // LCOV_EXCL_START
                throw invalid_parameter(
                          "the name of a field of type CHAR must have a size defined as in \"foo=123\"; \""
                        + std::string(f_description->f_field_name)
                        + "\" is missing an equal (=) character.");
                // LCOV_EXCL_STOP
            }
            f_field_name_len = equal - f_description->f_field_name;

            std::int64_t size(0);
            if(!advgetopt::validator_integer::convert_string(equal + 1, size))
            {
                throw invalid_parameter(
                          "the size in field \""
                        + std::string(f_description->f_field_name)
                        + "\" must be a valid decimal number.");
            }
            if(size <= 0
            || size >= (1LL << 32))
            {
                throw out_of_range(
                          "the size in field \""
                        + std::string(f_description->f_field_name)
                        + "\" must be at least 1 and no more then 2^32 - 1.");
            }
            f_size = size;
        }
        break;

    default:
        f_field_name_len = strlen(f_description->f_field_name);
        break;

    }
}


field_t::~field_t()
{
    pointer_t n(next());
    pointer_t p(previous());
    if(n != nullptr)
    {
        n->set_previous(p);
    }
    if(p != nullptr)
    {
        p->set_next(n);
    }
}


struct_description_t const * field_t::description() const
{
    return f_description;
}


field_t::pointer_t field_t::next() const
{
    return f_next.lock();
}


void field_t::set_next(pointer_t next)
{
    f_next = next;
}


field_t::pointer_t field_t::previous() const
{
    return f_previous.lock();
}


void field_t::set_previous(pointer_t previous)
{
    f_previous = previous;
}


field_t::pointer_t field_t::first() const
{
    pointer_t p(f_previous.lock());
    if(p == nullptr)
    {
        return const_cast<field_t *>(this)->shared_from_this();
    }
    for(;;)
    {
        pointer_t q(p->f_previous.lock());
        if(q == nullptr)
        {
            return p;
        }
        p = q;
    }
    snapdev::NOT_REACHED();
}


field_t::pointer_t field_t::last() const
{
    pointer_t n(f_next.lock());
    if(n == nullptr)
    {
        return const_cast<field_t *>(this)->shared_from_this();
    }
    for(;;)
    {
        pointer_t m(n->f_next.lock());
        if(m == nullptr)
        {
            return n;
        }
        n = m;
    }
}


struct_type_t field_t::type() const
{
    return f_description->f_type;
}


ssize_t field_t::field_size() const
{
    if(static_cast<std::size_t>(f_description->f_type) >= std::size(g_struct_type_sizes))
    {
        throw out_of_range(
                  "type out of range for converting it to a field size ("
                + to_string(f_description->f_type)
                + ", max: "
                + std::to_string(std::size(g_struct_type_sizes))
                + ").");
    }

    return g_struct_type_sizes[static_cast<int>(f_description->f_type)].f_size;
}


ssize_t field_t::type_field_size() const
{
    if(static_cast<size_t>(f_description->f_type) >= std::size(g_struct_type_sizes))
    {
        throw out_of_range(
                  "type out of range for converting it to a type field size ("
                + to_string(f_description->f_type)
                + ", max: "
                + std::to_string(std::size(g_struct_type_sizes))
                + ").");
    }

    if(f_description->f_type == struct_type_t::STRUCT_TYPE_CHAR)
    {
        // the constructor took care of the extract of the size and saved
        // it in f_size; this is what we want to use here
        //
        return f_size;
    }

    return g_struct_type_sizes[static_cast<int>(f_description->f_type)].f_field_size;
}


std::string field_t::field_name() const
{
    return std::string(f_description->f_field_name, f_field_name_len);
}


std::string field_t::new_name() const
{
    if(type() != struct_type_t::STRUCT_TYPE_RENAMED)
    {
        throw type_mismatch(
                  "field \""
                + field_name()
                + "\" is not a RENAMED field, it has no new name.");
    }

    if(f_description->f_sub_description == nullptr)
    {
        throw logic_error(
                  "field \""
                + field_name()
                + "\" is marked as having a new name (RENAMED) but it has no f_sub_description to define the new name.");
    }
    if(f_description->f_sub_description->f_field_name == nullptr)
    {
        throw logic_error(
                  "field \""
                + field_name()
                + "\" is marked as having a new name (RENAMED) but it has no entries in its f_sub_description defining the new name.");
    }

    return f_description->f_sub_description->f_field_name;
}


/** \brief Return the size of this field in bytes.
 *
 * This function returns the size of this field as defined by the set_size()
 * function.
 *
 * \warning
 * The size of a field_t is set by the structure parser. If you create
 * a field on your own, this is zero until you call set_size().
 *
 * \return The size of this field.
 *
 * \sa set_size()
 */
std::uint32_t field_t::size() const
{
    return f_size;
}


void field_t::set_size(std::uint32_t size)
{
    f_size = size;
}


bool field_t::has_flags(field_flags_t flags) const
{
    return (f_flags & flags) != 0;
}


field_t::field_flags_t field_t::flags() const
{
    return f_flags;
}


void field_t::set_flags(field_flags_t flags)
{
    f_flags = flags;
}


void field_t::add_flags(field_flags_t flags)
{
    f_flags |= flags;
}


void field_t::clear_flags(field_flags_t flags)
{
    f_flags &= ~flags;
}


flag_definition::pointer_t field_t::find_flag_definition(std::string const & name) const
{
    auto const & flag(f_flag_definitions.find(name));
    if(flag == f_flag_definitions.end())
    {
        field_not_found e(
                  "flag named \""
                + name
                + "\" not found.");
        e.set_parameter("flag_name", name);
        throw e;
    }

    return flag->second;
}


void field_t::add_flag_definition(flag_definition::pointer_t bits)
{
    f_flag_definitions[bits->flag_name()] = bits;
}


std::uint64_t field_t::offset() const
{
    return f_offset;
}


void field_t::set_offset(std::uint64_t offset)
{
    f_offset = offset;
}


void field_t::adjust_offset(std::int64_t adjust)
{
    f_offset += adjust;
}


structure::vector_t const & field_t::sub_structures() const
{
    return f_sub_structures;
}


structure::vector_t & field_t::sub_structures()
{
    return f_sub_structures;
}


structure::pointer_t field_t::operator [] (int idx) const
{
    if(static_cast<uint32_t>(idx) >= f_sub_structures.size())
    {
        if(f_sub_structures.empty())
        {
            throw out_of_bounds(
                  "index ("
                + std::to_string(idx)
                + ") is out of bounds since there are no sub-structures.");
        }
        throw out_of_bounds(
              "index ("
            + std::to_string(idx)
            + ") is out of bounds (0.."
            + std::to_string(f_sub_structures.size() - 1)
            + ")");
    }
    return f_sub_structures[idx];
}










structure::structure(struct_description_t const * descriptions, pointer_t parent)
    : f_descriptions(descriptions)
    , f_parent(parent)
{
    if(f_descriptions == nullptr)
    {
        throw logic_error("the description parameter of a structure object cannot be null.");
    }

    if(parent == nullptr)
    {
        // no parent, then the description must start with sequence:
        // 1. a magic field
        // 2. a structure version
        //
        if(f_descriptions[0].f_type != struct_type_t::STRUCT_TYPE_MAGIC
        || f_descriptions[1].f_type != struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        {
            throw logic_error("the root description of a structure must start with a magic field followed by a structure version.");
        }
    }
}


void structure::set_block(block::pointer_t b, std::uint64_t offset, std::uint64_t size)
{
    f_buffer = std::make_shared<virtual_buffer>(b, offset, size);
}


void structure::init_buffer()
{
    std::size_t const size(parse());

    f_buffer = std::make_shared<virtual_buffer>();
    buffer_t d(size);
    f_buffer->pwrite(d.data(), size, 0, true);

    set_defaults();
}


void structure::set_defaults()
{
    for(struct_description_t const * def(f_descriptions);
        def->f_type != struct_type_t::STRUCT_TYPE_END;
        ++def)
    {
        if(def->f_default_value == nullptr)
        {
            // skip entries without a default
            //
            // there is, however, one exception which is the structure version
            // which we define in the f_min_version
            //
            if(def->f_type == struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
            {
                // the structure version field name must be valid as is
                //
                field_t::pointer_t f(find_field(def->f_field_name));
                buffer_t const value(string_to_typed_buffer(struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION, def->f_min_version.to_string()));
                f_buffer->pwrite(value.data(), value.size(), f->offset());
            }
        }
        else
        {
            // the field name here may include sub-fields after an equal sign
            //
            char const * equal(strchrnul(def->f_field_name, '='));
            field_t::pointer_t f(find_field(std::string(def->f_field_name, equal)));

            // TODO: when we change the length of a string, array, buffer,
            //       then the later offsets need to be adjusted (I think they
            //       already are, but it needs verification)
            //
            switch(def->f_type)
            {
            case struct_type_t::STRUCT_TYPE_CHAR:
                {
                    buffer_t const value(string_to_typed_buffer(def->f_type, def->f_default_value, f->type_field_size()));
                    f_buffer->pwrite(value.data(), value.size(), f->offset());
                }
                break;

            case struct_type_t::STRUCT_TYPE_P8STRING:
            case struct_type_t::STRUCT_TYPE_P16STRING:
            case struct_type_t::STRUCT_TYPE_P32STRING:
                // P-strings are a special case which is easier to deal with
                // in the following way
                //
                set_string(def->f_field_name, def->f_default_value);
                break;

            default:
                {
                    buffer_t const value(string_to_typed_buffer(def->f_type, def->f_default_value));
                    f_buffer->pwrite(value.data(), value.size(), f->offset());
                }
                break;

            }
        }
    }
}


void structure::set_virtual_buffer(virtual_buffer::pointer_t buffer, reference_t start_offset)
{
    f_buffer = buffer;
    f_start_offset = start_offset;
    f_fields_by_name.clear(); // force a new parse() when the buffer changes
}


virtual_buffer::pointer_t structure::get_virtual_buffer(reference_t & start_offset) const
{
    start_offset = f_start_offset;
    return f_buffer;
}




/** \brief Get the static size or get 0.
 *
 * This function returns the size of the structure if the size is static.
 *
 * Most structures are no static, though, they will have variable fields
 * such as a string or a buffer. This function returns 0 for those
 * structures. You can still get a size using the get_current_size()
 * function, just keep in mind that the size may change as the data
 * varies in the structure.
 *
 * \note
 * A sub-structure is considered static as long as all of its fields are
 * static fields (i.e. no strings/arrays).
 *
 * \return The size of the structure is static, 0 if the structure size is variable.
 */
std::size_t structure::get_static_size() const
{
    std::size_t result(0);

    parse();

    for(auto const & f : f_fields_by_name)
    {
        if(f.second->has_flags(field_t::FIELD_FLAG_VARIABLE_SIZE))
        {
            return 0;
        }

        if(f.second->type() == struct_type_t::STRUCT_TYPE_RENAMED)
        {
            continue;
        }

        // the size of the structure field is ignored, it's always 1
        // and it has nothing to do with the size of the resulting
        // binary
        //
        if(f.second->type() != struct_type_t::STRUCT_TYPE_STRUCTURE)
        {
            result += f.second->size();
        }

        for(auto const & s : f.second->sub_structures())
        {
            std::size_t const size(s->get_static_size());
            if(size == 0)
            {
                return 0;
            }
            result += size;
        }
    }

    return result;
}


std::size_t structure::get_current_size(std::size_t start_offset) const
{
    if(!f_fields_by_name.empty())
    {
        for(field_t::pointer_t f(f_fields_by_name.begin()->second->first()); f != nullptr; f = f->next())
        {
            if(f->type() == struct_type_t::STRUCT_TYPE_RENAMED)
            {
                continue;
            }
#ifdef _DEBUG
            if(f->offset() != start_offset
            && cppthread::gettid() == f_verify_offset)
            {
                // LCOV_EXCL_START
                std::cout << std::flush;
                std::cerr << std::flush;
                SNAP_LOG_FATAL << "-------------- offsets are off?" << SNAP_LOG_SEND;
                display_offsets();
                throw logic_error(
                        "offset of field \""
                        + f->field_name()
                        + "\" seems off; size so far is "
                        + std::to_string(start_offset)
                        + " but field offset is "
                        + std::to_string(f->offset())
                        + ".");
                // LCOV_EXCL_STOP
            }
#endif

            // the size of the structure field is ignored, it's always 1
            // and it has nothing to do with the size of the resulting
            // binary
            //
            switch(f->type())
            {
            case struct_type_t::STRUCT_TYPE_STRUCTURE:
                break;

            // for the STRING & BUFFER fields, we need to add a few bytes for
            // the space holding the size
            //
            case struct_type_t::STRUCT_TYPE_P8STRING:
            case struct_type_t::STRUCT_TYPE_BUFFER8:
                start_offset += 1 + f->size();
                break;

            case struct_type_t::STRUCT_TYPE_P16STRING:
            case struct_type_t::STRUCT_TYPE_BUFFER16:
                start_offset += 2 + f->size();
                break;

            case struct_type_t::STRUCT_TYPE_P32STRING:
            case struct_type_t::STRUCT_TYPE_BUFFER32:
                start_offset += 4 + f->size();
                break;

            // right here, the size of arrays is the space holding the number
            // of items, not the total byte size which we get by calling
            // get_current_size() recursively
            //
            case struct_type_t::STRUCT_TYPE_ARRAY8:
                start_offset += 1;
                break;

            case struct_type_t::STRUCT_TYPE_ARRAY16:
                start_offset += 2;
                break;

            case struct_type_t::STRUCT_TYPE_ARRAY32:
                start_offset += 4;
                break;

            default:
                start_offset += f->size();
                break;

            }

            for(auto const & s : f->sub_structures())
            {
                start_offset = s->get_current_size(start_offset);
            }
        }
    }

    return start_offset;
}


structure::pointer_t structure::parent() const
{
    return f_parent.lock();
}


field_t::pointer_t structure::get_field(std::string const & field_name, struct_type_t type) const
{
    if(f_buffer == nullptr)
    {
        throw logic_error(
                  "trying to access a structure field when the f_buffer"
                  " pointer is still null.");
    }

    if(field_name.empty())
    {
        throw logic_error(
                  "called structure::get_field() with an empty field name.");
    }

    // make sure we parsed the descriptions
    //
    parse();

    structure::pointer_t s(const_cast<structure *>(this)->shared_from_this());
    field_t::pointer_t f(nullptr);
    char const * n(field_name.c_str());
    for(;;)
    {
        // Note: if the name includes an equal, we treat it like a NUL at
        //       the end of the string (i.e. char_field=32)
        //
        // Note: at this time, we do not support accessing arrays (i.e. having
        //       '[<index>]') because I don't see the point since indexes need to
        //       be dynamic pretty much 100% of the time
        //
        char const * e(n);
        while(*e != '.' && *e != '=' && *e != '\0')
        {
            ++e;
        }
        std::string const sub_field_name(n, e - n);

        // catch the "not found" errors so we can put the full name of the
        // field in the message
        //
        try
        {
            f = s->find_field(sub_field_name);
        }
        catch(field_not_found & not_found)
        {
            not_found.set_parameter("full_field_name", field_name);
            throw not_found;
        }
        // LCOV_EXCL_START
        catch(...)
        {
            throw;
        }
        // LCOV_EXCL_STOP
        if(*e == '='
        || *e == '\0')
        {
            if(type != struct_type_t::STRUCT_TYPE_END
            && f->type() != type)
            {
                throw type_mismatch(
                          "this field type is \""
                        + to_string(f->type())
                        + "\" but we expected \""
                        + to_string(type)
                        + "\".");
            }

            return f;
        }

        switch(f->type())
        {
        case struct_type_t::STRUCT_TYPE_STRUCTURE:
            break;

        case struct_type_t::STRUCT_TYPE_BITS8:
        case struct_type_t::STRUCT_TYPE_BITS16:
        case struct_type_t::STRUCT_TYPE_BITS32:
        case struct_type_t::STRUCT_TYPE_BITS64:
        case struct_type_t::STRUCT_TYPE_BITS128:
        case struct_type_t::STRUCT_TYPE_BITS256:
        case struct_type_t::STRUCT_TYPE_BITS512:
            // the sub-name is not a field name, it's a flag name in this case
            //
            {
                flag_definition::pointer_t flag(f->find_flag_definition(e + 1));
                if(type != struct_type_t::STRUCT_TYPE_END
                && f->type() != type)
                {
                    throw type_mismatch(
                              "this field type is \""
                            + to_string(f->type())
                            + "\" but we expected \""
                            + to_string(type)
                            + "\".");
                }
                return f;
            } // LCOV_EXCL_LINE

        default:
            throw type_mismatch(
                      "field \""
                    + sub_field_name
                    + "\" is not of type structure or bit field so you can't"
                      " get a sub-field (i.e. have a period in the name).");

        }

#ifdef _DEBUG
        // since we test the type above, not having a sub-structure of 1
        // should just never happen
        //
        if(f->sub_structures().size() != 1)
        {
            // LCOV_EXCL_START
            throw invalid_size(
                      "a structure requires a sub_structure vector of size 1 (got "
                    + std::to_string(f->sub_structures().size())
                    + " instead).");
            // LCOV_EXCL_STOP
        }
#endif

        s = (*f)[0];
        n = e + 1;  // +1 to skip the '.'
    }
}


flag_definition::pointer_t structure::get_flag(std::string const & flag_name, field_t::pointer_t & f) const
{
    char const * s(flag_name.c_str());
    char const * e(s + flag_name.length());
    while(e > s && e[0] != '.')
    {
        --e;
    }
    if(e == s)
    {
        field_not_found not_found(
                  "flag named \""
                + flag_name
                + "\" must at least include a field name and a flag name.");
        not_found.set_parameter("flag_name", flag_name);
        throw not_found;
    }

    std::string const field_name(s, e - s);
    f = get_field(field_name);

    // bit fields have sub-names we can check for `field_name`
    //
    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_BITS8:
    case struct_type_t::STRUCT_TYPE_BITS16:
    case struct_type_t::STRUCT_TYPE_BITS32:
    case struct_type_t::STRUCT_TYPE_BITS64:
    case struct_type_t::STRUCT_TYPE_BITS128:
    case struct_type_t::STRUCT_TYPE_BITS256:
    case struct_type_t::STRUCT_TYPE_BITS512:
        return f->find_flag_definition(e + 1);

    default:
        // incorrect type
        //
        throw type_mismatch(
                  "expected a field of type BITS<size> for flag named \""
                + flag_name
                + "\". Got a "
                + to_string(f->type())
                + " instead.");

    }
}


field_t::pointer_t structure::find_field(std::string const & field_name)
{
    auto field(f_fields_by_name.find(field_name));
    if(field == f_fields_by_name.end())
    {
        // we can't return a field and yet it is mandatory, throw an error
        // (if we change a description to still include old fields, we need
        // to have a way to point to the new field--see the RENAMED flag).
        //
        field_not_found e(
                  "this description does not include a field named \""
                + field_name
                + "\".");
        e.set_parameter("field_name", field_name);
        throw e;
    }

    field_t::pointer_t f(field->second);
    if(f->type() == struct_type_t::STRUCT_TYPE_RENAMED)
    {
        std::string const new_name(f->new_name());
        field = f_fields_by_name.find(new_name);
        if(field == f_fields_by_name.end())
        {
            field_not_found e(
                      "this description renames field \""
                    + field_name
                    + "\" to \""
                    + new_name
                    + "\" but we could not find the latter field.");
            e.set_parameter("field_name", field_name);
            e.set_parameter("new_name", new_name);
            throw e;
        }
        f = field->second;

        // let programmers know that the old name is deprecated
        //
        SNAP_LOG_CONFIGURATION_WARNING
            << "deprecated field name \""
            << field_name
            << "\" was changed to \""
            << new_name
            << "\". Please change your code to use the new name."
            << SNAP_LOG_SEND;
    }

    return f;
} // LCOV_EXCL_LINE


std::int64_t structure::get_integer(std::string const & field_name) const
{
    auto f(get_field(field_name));

    verify_size(f->type(), f->size());

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_INT8:
        {
            std::int8_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return value;
        }

    case struct_type_t::STRUCT_TYPE_INT16:
        {
            std::int16_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return value;
        }

    case struct_type_t::STRUCT_TYPE_INT32:
        {
            std::int32_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return value;
        }

    case struct_type_t::STRUCT_TYPE_INT64:
    case struct_type_t::STRUCT_TYPE_TIME:
    case struct_type_t::STRUCT_TYPE_MSTIME:
    case struct_type_t::STRUCT_TYPE_USTIME:
        {
            std::int64_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return value;
        }

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_INT8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_MSTIME)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_TIME)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_USTIME)
                + "\".");

    }
}


void structure::set_integer(std::string const & field_name, std::int64_t value)
{
    auto f(get_field(field_name));

    verify_size(f->type(), f->size());

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_INT8:
        {
            std::int8_t const v(value);
            f_buffer->pwrite(&v, sizeof(v), f->offset());
        }
        return;

    case struct_type_t::STRUCT_TYPE_INT16:
        {
            std::int16_t const v(value);
            f_buffer->pwrite(&v, sizeof(v), f->offset());
        }
        return;

    case struct_type_t::STRUCT_TYPE_INT32:
        {
            std::int32_t const v(value);
            f_buffer->pwrite(&v, sizeof(v), f->offset());
        }
        return;

    case struct_type_t::STRUCT_TYPE_INT64:
    case struct_type_t::STRUCT_TYPE_TIME:
    case struct_type_t::STRUCT_TYPE_MSTIME:
    case struct_type_t::STRUCT_TYPE_USTIME:
        f_buffer->pwrite(&value, sizeof(value), f->offset());
        return;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_INT8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_MSTIME)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_TIME)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_USTIME)
                + "\".");

    }
}


std::uint64_t structure::get_uinteger(std::string const & field_name) const
{
    auto f(get_field(field_name));

    verify_size(f->type(), f->size());

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_BITS8:
    case struct_type_t::STRUCT_TYPE_UINT8:
        {
            std::uint8_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return value;
        }

    case struct_type_t::STRUCT_TYPE_BITS16:
    case struct_type_t::STRUCT_TYPE_UINT16:
        {
            std::uint16_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return value;
        }

    case struct_type_t::STRUCT_TYPE_BITS32:
    case struct_type_t::STRUCT_TYPE_UINT32:
    case struct_type_t::STRUCT_TYPE_MAGIC:
    case struct_type_t::STRUCT_TYPE_VERSION:
    case struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION:
        {
            std::uint32_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return value;
        }

    case struct_type_t::STRUCT_TYPE_BITS64:
    case struct_type_t::STRUCT_TYPE_UINT64:
    case struct_type_t::STRUCT_TYPE_REFERENCE:
    case struct_type_t::STRUCT_TYPE_OID:
        {
            std::uint64_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return value;
        }

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_BITS8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_MAGIC)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_OID)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_REFERENCE)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_VERSION)
                + "\".");

    }
}


void structure::set_uinteger(std::string const & field_name, std::uint64_t value)
{
    auto f(get_field(field_name));

    verify_size(f->type(), f->size());

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_BITS8:
    case struct_type_t::STRUCT_TYPE_UINT8:
        {
            std::uint8_t const v(value);
            f_buffer->pwrite(&v, sizeof(v), f->offset());
        }
        return;

    case struct_type_t::STRUCT_TYPE_BITS16:
    case struct_type_t::STRUCT_TYPE_UINT16:
        {
            std::uint16_t const v(value);
            f_buffer->pwrite(&v, sizeof(v), f->offset());
        }
        return;

    case struct_type_t::STRUCT_TYPE_BITS32:
    case struct_type_t::STRUCT_TYPE_UINT32:
    case struct_type_t::STRUCT_TYPE_VERSION:
        {
            std::uint32_t const v(value);
            f_buffer->pwrite(&v, sizeof(v), f->offset());
        }
        return;

    case struct_type_t::STRUCT_TYPE_BITS64:
    case struct_type_t::STRUCT_TYPE_UINT64:
    case struct_type_t::STRUCT_TYPE_REFERENCE:
    case struct_type_t::STRUCT_TYPE_OID:
        f_buffer->pwrite(&value, sizeof(value), f->offset());
        return;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_BITS8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_OID)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_REFERENCE)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_VERSION)
                + "\".");

    }
}


std::uint64_t structure::get_bits(std::string const & flag_name) const
{
    field_t::pointer_t f;
    flag_definition::pointer_t flag(get_flag(flag_name, f));

    verify_size(f->type(), f->size());

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_BITS8:
        {
            std::uint8_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return (value & flag->mask()) >> flag->pos();
        }

    case struct_type_t::STRUCT_TYPE_BITS16:
        {
            std::uint16_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return (value & flag->mask()) >> flag->pos();
        }

    case struct_type_t::STRUCT_TYPE_BITS32:
        {
            std::uint32_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return (value & flag->mask()) >> flag->pos();
        }

    case struct_type_t::STRUCT_TYPE_BITS64:
        {
            std::uint64_t value(0);
            f_buffer->pread(&value, sizeof(value), f->offset());
            return (value & flag->mask()) >> flag->pos();
        }

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_BITS8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS64)
                + "\".");

    }
}


void structure::set_bits(std::string const & flag_name, std::uint64_t value)
{
    field_t::pointer_t f;
    flag_definition::pointer_t flag(get_flag(flag_name, f));

    verify_size(f->type(), f->size());

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_BITS8:
    case struct_type_t::STRUCT_TYPE_BITS16:
    case struct_type_t::STRUCT_TYPE_BITS32:
    case struct_type_t::STRUCT_TYPE_BITS64:
        break;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_BITS8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS64)
                + "\".");

    }

    if((value & (flag->mask() >> flag->pos())) != value)
    {
        throw invalid_number(
                  "value \""
                + std::to_string(value)
                + "\" does not fit in flag field \""
                + flag->full_name()
                + "\".");
    }

    std::uint64_t v(get_uinteger(f->field_name()));
    v &= ~flag->mask();
    v |= value << flag->pos();
    set_uinteger(f->field_name(), v);
}


dbtype_t structure::get_magic() const
{
    return static_cast<dbtype_t>(get_uinteger(g_system_field_name_magic));
}


version_t structure::get_version(std::string const & field_name) const
{
    return get_uinteger(field_name);
}


void structure::set_version(std::string const & field_name, version_t const & version)
{
    set_uinteger(field_name, version.to_binary());
}


int512_t structure::get_large_integer(std::string const & field_name) const
{
    auto f(get_field(field_name));

    verify_size(f->type(), f->size());

    int512_t result;
    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_INT8:
        f_buffer->pread(&result.f_value, sizeof(int8_t), f->offset());
        result.f_value[0] = static_cast<int8_t>(result.f_value[0]); // sign extend
sign_extend_64bit:
        result.f_value[1] = static_cast<int64_t>(result.f_value[0]) < 0
                                    ? -1
                                    : 0;
        result.f_value[2] = result.f_value[1];
        result.f_value[3] = result.f_value[1];
        result.f_value[4] = result.f_value[1];
        result.f_value[5] = result.f_value[1];
        result.f_value[6] = result.f_value[1];
        result.f_high_value = result.f_value[1];
        return result;

    case struct_type_t::STRUCT_TYPE_INT16:
        f_buffer->pread(&result.f_value, sizeof(int16_t), f->offset());
        result.f_value[0] = static_cast<int16_t>(result.f_value[0]); // sign extend
        goto sign_extend_64bit;

    case struct_type_t::STRUCT_TYPE_INT32:
        f_buffer->pread(&result.f_value, sizeof(int32_t), f->offset());
        result.f_value[0] = static_cast<int32_t>(result.f_value[0]); // sign extend
        goto sign_extend_64bit;

    case struct_type_t::STRUCT_TYPE_INT64:
    case struct_type_t::STRUCT_TYPE_TIME:
    case struct_type_t::STRUCT_TYPE_MSTIME:
    case struct_type_t::STRUCT_TYPE_USTIME:
        f_buffer->pread(&result.f_value, sizeof(int64_t), f->offset());
        result.f_value[0] = static_cast<int64_t>(result.f_value[0]); // sign extend
        goto sign_extend_64bit;

    case struct_type_t::STRUCT_TYPE_INT128:
    case struct_type_t::STRUCT_TYPE_NSTIME:
        f_buffer->pread(&result.f_value, sizeof(int64_t) * 2, f->offset());

        result.f_value[2] = static_cast<int64_t>(result.f_value[1]) < 0
                                    ? -1
                                    : 0;
        result.f_value[3] = result.f_value[2];
        result.f_value[4] = result.f_value[2];
        result.f_value[5] = result.f_value[2];
        result.f_value[6] = result.f_value[2];
        result.f_high_value = result.f_value[2];
        return result;

    case struct_type_t::STRUCT_TYPE_INT256:
        f_buffer->pread(&result.f_value, sizeof(int64_t) * 4, f->offset());

        result.f_value[4] = static_cast<int64_t>(result.f_value[3]) < 0
                                    ? -1
                                    : 0;
        result.f_value[5] = result.f_value[4];
        result.f_value[6] = result.f_value[4];
        result.f_high_value = result.f_value[4];
        return result;

    case struct_type_t::STRUCT_TYPE_INT512:
        f_buffer->pread(&result.f_value, sizeof(int64_t) * 8, f->offset());
        return result;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_INT8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT128)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT256)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT512)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_MSTIME)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_NSTIME)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_TIME)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_USTIME)
                + "\".");

    }
}


void structure::set_large_integer(std::string const & field_name, int512_t const & value)
{
    auto f(get_field(field_name));

    verify_size(f->type(), f->size());

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_INT8:
    case struct_type_t::STRUCT_TYPE_INT16:
    case struct_type_t::STRUCT_TYPE_INT32:
    case struct_type_t::STRUCT_TYPE_INT64:
    case struct_type_t::STRUCT_TYPE_INT128:
    case struct_type_t::STRUCT_TYPE_INT256:
    case struct_type_t::STRUCT_TYPE_INT512:
    case struct_type_t::STRUCT_TYPE_MSTIME:
    case struct_type_t::STRUCT_TYPE_NSTIME:
    case struct_type_t::STRUCT_TYPE_TIME:
    case struct_type_t::STRUCT_TYPE_USTIME:
        f_buffer->pwrite(value.f_value, f->size(), f->offset());
        return;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_INT8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT128)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT256)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_INT512)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_MSTIME)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_NSTIME)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_TIME)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_USTIME)
                + "\".");

    }
}


uint512_t structure::get_large_uinteger(std::string const & field_name) const
{
    auto f(get_field(field_name));

    verify_size(f->type(), f->size());

    uint512_t result;
    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_BITS8:
    case struct_type_t::STRUCT_TYPE_BITS16:
    case struct_type_t::STRUCT_TYPE_BITS32:
    case struct_type_t::STRUCT_TYPE_BITS64:
    case struct_type_t::STRUCT_TYPE_BITS128:
    case struct_type_t::STRUCT_TYPE_BITS256:
    case struct_type_t::STRUCT_TYPE_BITS512:
    case struct_type_t::STRUCT_TYPE_MAGIC:
    case struct_type_t::STRUCT_TYPE_REFERENCE:
    case struct_type_t::STRUCT_TYPE_OID:
    case struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION:
    case struct_type_t::STRUCT_TYPE_UINT8:
    case struct_type_t::STRUCT_TYPE_UINT16:
    case struct_type_t::STRUCT_TYPE_UINT32:
    case struct_type_t::STRUCT_TYPE_UINT64:
    case struct_type_t::STRUCT_TYPE_UINT128:
    case struct_type_t::STRUCT_TYPE_UINT256:
    case struct_type_t::STRUCT_TYPE_UINT512:
    case struct_type_t::STRUCT_TYPE_VERSION:
        f_buffer->pread(&result.f_value, f->size(), f->offset());
        break;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_BITS8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_MAGIC)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_OID)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_REFERENCE)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT128)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT256)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT512)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_VERSION)
                + "\".");

    }

    return result;
}


void structure::set_large_uinteger(std::string const & field_name, uint512_t const & value)
{
    auto f(get_field(field_name));

    verify_size(f->type(), f->size());

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_BITS8:
    case struct_type_t::STRUCT_TYPE_BITS16:
    case struct_type_t::STRUCT_TYPE_BITS32:
    case struct_type_t::STRUCT_TYPE_BITS64:
    case struct_type_t::STRUCT_TYPE_BITS128:
    case struct_type_t::STRUCT_TYPE_BITS256:
    case struct_type_t::STRUCT_TYPE_BITS512:
    case struct_type_t::STRUCT_TYPE_OID:
    case struct_type_t::STRUCT_TYPE_REFERENCE:
    case struct_type_t::STRUCT_TYPE_UINT8:
    case struct_type_t::STRUCT_TYPE_UINT16:
    case struct_type_t::STRUCT_TYPE_UINT32:
    case struct_type_t::STRUCT_TYPE_UINT64:
    case struct_type_t::STRUCT_TYPE_UINT128:
    case struct_type_t::STRUCT_TYPE_UINT256:
    case struct_type_t::STRUCT_TYPE_UINT512:
    case struct_type_t::STRUCT_TYPE_VERSION:
        f_buffer->pwrite(value.f_value, f->size(), f->offset());
        break;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_BITS8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BITS64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_OID)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_REFERENCE)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT32)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT64)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT128)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT256)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_UINT512)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_VERSION)
                + "\".");

    }
}


snapdev::timespec_ex structure::get_nstime(std::string const & field_name) const
{
    uint512_t const u128(get_large_uinteger(field_name));
    snapdev::timespec_ex result;
    result.tv_sec = u128.f_value[0];
    result.tv_nsec = u128.f_value[1];
    return result;
}


void structure::set_nstime(std::string const & field_name, snapdev::timespec_ex const & value)
{
    uint512_t u128 = {};
    u128.f_value[0] = value.tv_sec;
    u128.f_value[1] = value.tv_nsec;
    set_large_uinteger(field_name, u128);
}


float structure::get_float32(std::string const & field_name) const
{
    auto f(get_field(field_name, struct_type_t::STRUCT_TYPE_FLOAT32));

    verify_size(struct_type_t::STRUCT_TYPE_FLOAT32, f->size());

    float result;
    f_buffer->pread(&result, sizeof(float), f->offset());
    return result;
}


void structure::set_float32(std::string const & field_name, float value)
{
    auto f(get_field(field_name, struct_type_t::STRUCT_TYPE_FLOAT32));

    verify_size(struct_type_t::STRUCT_TYPE_FLOAT32, f->size());

    f_buffer->pwrite(&value, sizeof(float), f->offset());
}


double structure::get_float64(std::string const & field_name) const
{
    auto f(get_field(field_name, struct_type_t::STRUCT_TYPE_FLOAT64));

    verify_size(struct_type_t::STRUCT_TYPE_FLOAT64, f->size());

    double result;
    f_buffer->pread(&result, sizeof(double), f->offset());
    return result;
}


void structure::set_float64(std::string const & field_name, double value)
{
    auto f(get_field(field_name, struct_type_t::STRUCT_TYPE_FLOAT64));

    verify_size(struct_type_t::STRUCT_TYPE_FLOAT64, f->size());

    f_buffer->pwrite(&value, sizeof(double), f->offset());
}


long double structure::get_float128(std::string const & field_name) const
{
    auto f(get_field(field_name, struct_type_t::STRUCT_TYPE_FLOAT128));

    verify_size(struct_type_t::STRUCT_TYPE_FLOAT128, f->size());

    long double result;
    f_buffer->pread(&result, sizeof(long double), f->offset());
    return result;
}


void structure::set_float128(std::string const & field_name, long double value)
{
    auto f(get_field(field_name, struct_type_t::STRUCT_TYPE_FLOAT128));

    verify_size(struct_type_t::STRUCT_TYPE_FLOAT128, f->size());

    f_buffer->pwrite(&value, sizeof(long double), f->offset());
}


std::string structure::get_string(std::string const & field_name) const
{
    auto f(get_field(field_name));
    ssize_t const field_size(f->type_field_size());

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_CHAR:
        {
            std::string result(field_size, '\0');
            f_buffer->pread(const_cast<char *>(result.data()), field_size, f->offset());
            std::size_t const len(strnlen(result.c_str(), field_size));
            return result.substr(0, len);
        }

    case struct_type_t::STRUCT_TYPE_P8STRING:
    case struct_type_t::STRUCT_TYPE_P16STRING:
    case struct_type_t::STRUCT_TYPE_P32STRING:
        break;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_CHAR)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_P8STRING)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_P16STRING)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_P32STRING)
                + "\".");

    }

    // TBD: should we ignore this check in release mode?
    std::uint32_t length(0);
    f_buffer->pread(&length, field_size, f->offset());
    // in big endian we have to swap the bytes in length if field_size != 1
    if(length != f->size())
    {
        throw corrupted_data(
                  "the size of string field \""
                + field_name
                + "\" ("
                + std::to_string(f->size())
                + ") is different from the size found in the file ("
                + std::to_string(length)
                + " found at offset "
                + std::to_string(f->offset())
                + " over "
                + std::to_string(field_size)
                + " bytes).");
    }

    std::string result(length, '\0');
    f_buffer->pread(const_cast<char *>(result.data()), length, f->offset() + field_size);
    return result;
}


void structure::set_string(std::string const & field_name, std::string const & value)
{
    auto f(get_field(field_name));
    ssize_t const field_size(f->type_field_size());

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_CHAR:
        if(value.length() > static_cast<std::size_t>(field_size))
        {
            throw out_of_range(
                      "the CHAR field is limited to "
                    + std::to_string(field_size)
                    + " characters. The input string is "
                    + std::to_string(value.length())
                    + " characters. It does not fit this field.");
        }
SNAP_LOG_WARNING << "CHAR case: offset (1) = " << f->offset() << SNAP_LOG_SEND;
        f_buffer->pwrite(value.data(), value.length(), f->offset());
        if(static_cast<std::size_t>(field_size) > value.length())
        {
            // fill the reset with zeroes
            //
            buffer_t const zeroes(field_size - value.length());
SNAP_LOG_WARNING << "CHAR case: offset (2) = " << f->offset() << " + " << value.length() << SNAP_LOG_SEND;
            f_buffer->pwrite(zeroes.data(), zeroes.size(), f->offset() + value.length());
        }
        return;

    case struct_type_t::STRUCT_TYPE_P8STRING:
    case struct_type_t::STRUCT_TYPE_P16STRING:
    case struct_type_t::STRUCT_TYPE_P32STRING:
        break;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_CHAR)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_P8STRING)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_P16STRING)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_P32STRING)
                + "\".");

    }

    // check the length
    //
    // WARNING: the pread() works as is in little endian, in big endian
    //          we would have to "bswap" the bytes
    //
    std::uint32_t length(0);
    f_buffer->pread(&length, field_size, f->offset());
    if(length != f->size())
    {
        throw corrupted_data(
                  "the size of string field \""
                + field_name
                + "\" ("
                + std::to_string(f->size())
                + ") is different from the size found in the file ("
                + std::to_string(length)
                + " found at offset "
                + std::to_string(f->offset())
                + " over "
                + std::to_string(field_size)
                + " bytes).");
    }

    std::uint32_t const size(value.length());
    std::uint64_t const max_size(1ULL << (field_size * 8));
    if(size >= max_size)
    {
        throw out_of_range(
                  "the input string is too large for this string field ("
                + std::to_string(size)
                + " >= "
                + std::to_string(max_size)
                + ").");
    }

    if(size == length)
    {
        // just do a write of the string
        // (the size remains the same)
        //
SNAP_LOG_WARNING << "size == length case: offset (1) = " << f->offset() << " + " << field_size << SNAP_LOG_SEND;
        f_buffer->pwrite(value.data(), size, f->offset() + field_size);
    }
    else if(size > length)
    {
        // WARNING: this write assumes size is in a little endian integer
        //
SNAP_LOG_WARNING << "size > length case: offset (1) = " << f->offset() << SNAP_LOG_SEND;
        f_buffer->pwrite(&size, field_size, f->offset());
SNAP_LOG_WARNING << "size > length case: offset (2) = " << f->offset() << " + " << field_size << SNAP_LOG_SEND;
        f_buffer->pwrite(value.data(), length, f->offset() + field_size);
SNAP_LOG_WARNING << "size > length case: offset (3) = " << f->offset() << " + " << field_size << " + " << length << SNAP_LOG_SEND;
        f_buffer->pinsert(value.data() + length, size - length, f->offset() + field_size + length);
    }
    else //if(size < length)
    {
        // WARNING: this write assumes size is in a little endian integer
        //
SNAP_LOG_WARNING << "size < length case: offset (1) = " << f->offset() << SNAP_LOG_SEND;
        f_buffer->pwrite(&size, field_size, f->offset());
SNAP_LOG_WARNING << "size < length case: offset (2) = " << f->offset() << " + " << field_size << SNAP_LOG_SEND;
        f_buffer->pwrite(value.data(), size, f->offset() + field_size);
SNAP_LOG_WARNING << "size < length case: offset (3) = " << f->offset() << " + " << field_size << " + " << size << " erase: " << length - size << " bytes" << SNAP_LOG_SEND;
        f_buffer->perase(length - size, f->offset() + field_size + size);
    }

    f->set_size(size);
    adjust_offsets(f->offset(), static_cast<std::int32_t>(size - length));

    verify_buffer_size();
}


structure::pointer_t structure::get_structure(std::string const & field_name) const
{
    auto f(get_field(field_name, struct_type_t::STRUCT_TYPE_STRUCTURE));

    if(f->sub_structures().size() != 1)
    {
        // LCOV_EXCL_START
        // if not initialized, the get_field() fails, so we cannot reach
        // this throw in a test at the moment
        throw invalid_size(
                  "a structure requires a sub_structure vector of size 1 (got "
                + std::to_string(f->sub_structures().size())
                + " instead).");
        // LCOV_EXCL_STOP
    }

    return (*f)[0];
}


//void structure::set_structure(std::string const & field_name, structure::pointer_t & value)
//{
//    auto f(get_field(field_name, struct_type_t::STRUCT_TYPE_STRUCTURE));
//
//    if(f->sub_structures().size() != 1)
//    {
//        throw invalid_size(
//                  "a structure requires a sub_structure vector of size 1 (got "
//                + std::to_string(f->sub_structures().size())
//                + " instead).");
//    }
//
//    if(f->sub_structures().empty()) // as per the above test, this is not possible at the moment, but TBD
//    {
//        f->sub_structures().push_back(value);
//    }
//    else
//    {
//        f->sub_structures()[0] = value;
//    }
//}


structure::vector_t const & structure::get_array(std::string const & field_name) const
{
    auto f(get_field(field_name));

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_ARRAY8:
    case struct_type_t::STRUCT_TYPE_ARRAY16:
    case struct_type_t::STRUCT_TYPE_ARRAY32:
        break;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY32)
                + "\".");

    }

    return f->sub_structures();
}


structure::vector_t & structure::get_array(std::string const & field_name)
{
    auto f(get_field(field_name));

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_ARRAY8:
    case struct_type_t::STRUCT_TYPE_ARRAY16:
    case struct_type_t::STRUCT_TYPE_ARRAY32:
        break;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY32)
                + "\".");

    }

    return f->sub_structures();
}


//void structure::set_array(std::string const & field_name, vector_t const & value)
//{
//    auto f(get_field(field_name));
//
//    switch(f->type())
//    {
//    case struct_type_t::STRUCT_TYPE_ARRAY8:
//    case struct_type_t::STRUCT_TYPE_ARRAY16:
//    case struct_type_t::STRUCT_TYPE_ARRAY32:
//        break;
//
//    default:
//        throw type_mismatch(
//                  "the set_array() function expected a STRUCT_TYPE_ARRAY<size> field instead of \""
//                + to_string(f->type())
//                + "\".");
//
//    }
//
//    // remove the existing array items
//    //
//    while(!f->sub_structures().empty())
//    {
//        delete_array_item(field_name, f->sub_structures().size() - 1);
//    }
//
//    // add the new items
//    //
//    for(auto const & a : value)
//    {
//        structure::pointer_t s(new_array_item(field_name));
//        s.copy_structure(a);
//    }
//}


structure::pointer_t structure::new_array_item(std::string const & field_name)
{
    auto f(get_field(field_name));

    std::uint64_t size(0);
    std::uint64_t max(0);
    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_ARRAY8:
        f_buffer->pread(&size, sizeof(std::uint8_t), f->offset());
        max = 1ULL << 8;
        break;

    case struct_type_t::STRUCT_TYPE_ARRAY16:
        f_buffer->pread(&size, sizeof(std::uint16_t), f->offset());
        max = 1ULL << 16;
        break;

    case struct_type_t::STRUCT_TYPE_ARRAY32:
        f_buffer->pread(&size, sizeof(std::uint32_t), f->offset());
        max = 1ULL << 32;
        break;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY32)
                + "\".");

    }

    // make sure we can add another item
    //
    ++size;
    if(size >= max)
    {
        throw out_of_range(
                  "the new_array_item() function cannot be used because the array is already full with "
                + std::to_string(max)
                + " items.");
    }

    reference_t offset(0);
    field_t::pointer_t n(f->next());
    if(n == nullptr)
    {
        // no next, add item at the very end
        //
        offset = get_current_size();
    }
    else
    {
        // insert item just before the next field
        //
        offset = n->offset();
    }

    // create the structure and define the offsets before we specify
    // the buffer (this is very important because we need the size of
    // that new buffer and that is known only after the parse() function
    // returns)
    //
    structure::pointer_t s(std::make_shared<structure>(f->description()->f_sub_description, shared_from_this()));
    s->f_start_offset = offset;
    reference_t const new_offset(s->parse());

    // now add the buffer area for that new sub-structure
    //
    std::size_t const add(s->get_current_size());
#ifdef _DEBUG
    if(add != new_offset - offset)
    {
        // LCOV_EXCL_START
        throw logic_error(
                  "sub-structure says its size is "
                + std::to_string(add)
                + " but the offsets say it's "
                + std::to_string(new_offset - offset)
                + ".");
        // LCOV_EXCL_STOP
    }
#endif
    std::vector<std::uint8_t> value(add, 0);
    f_buffer->pinsert(value.data(), add, offset);
    //s->set_virtual_buffer(f_buffer, offset); -- this cancels the parse()
    s->f_buffer = f_buffer;

    // increment the array counter and save it
    //
    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_ARRAY8:
        f_buffer->pwrite(&size, sizeof(uint8_t), f->offset());
        break;

    case struct_type_t::STRUCT_TYPE_ARRAY16:
        f_buffer->pwrite(&size, sizeof(uint16_t), f->offset());
        break;

    case struct_type_t::STRUCT_TYPE_ARRAY32:
        f_buffer->pwrite(&size, sizeof(uint32_t), f->offset());
        break;

    default: // LCOV_EXCL_LINE
        // we already throw above so we should never reach this case
        //
        snapdev::NOT_REACHED(); // LCOV_EXCL_LINE
        break;

    }

    adjust_offsets(offset - 1, new_offset - offset);
    //adjust_offsets(f->offset(), new_offset - offset);

    // WARNING: for the adjust_offsets() to work properly we MUST have this
    //          push_back() after it; otherwise the sub-fields would also
    //          get moved
    //
    f->sub_structures().push_back(s);

    // setting default strings will adjust the offsets again so we have to
    // do that after we already made adjustments for adding this sub-structure
    //
    s->set_defaults();

    verify_buffer_size();

    return s;
}


void structure::delete_array_item(std::string const & field_name, int idx)
{
    auto f(get_field(field_name));

    std::uint64_t size(0);
    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_ARRAY8:
        f_buffer->pread(&size, sizeof(std::uint8_t), f->offset());
        break;

    case struct_type_t::STRUCT_TYPE_ARRAY16:
        f_buffer->pread(&size, sizeof(std::uint16_t), f->offset());
        break;

    case struct_type_t::STRUCT_TYPE_ARRAY32:
        f_buffer->pread(&size, sizeof(std::uint32_t), f->offset());
        break;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_ARRAY32)
                + "\".");

    }

    structure::vector_t & array(f->sub_structures());
    if(size != array.size())
    {
        throw corrupted_data(
                  "the number of array items is different in our buffer and our field (read "
                + std::to_string(size)
                + " from the buffer, found "
                + std::to_string(array.size())
                + " in our in memory array).");
    }

    if(static_cast<std::size_t>(idx) >= size)
    {
        throw out_of_range(
                  "the index ("
                + std::to_string(idx)
                + ") in delete_array_item() function is out of range (larger or equal to the size: "
                + std::to_string(size)
                + ").");
    }

    reference_t offset(0);
    snapdev::NOT_USED(array[idx]->get_virtual_buffer(offset));
    std::size_t const item_size(array[idx]->get_current_size());
    std::size_t const bytes_erased(f_buffer->perase(item_size, offset));
    if(bytes_erased != item_size)
    {
        // LCOV_EXCL_START
        throw out_of_range(
                  "could not properly delete_array_item() data; deleted "
                + std::to_string(bytes_erased)
                + " bytes; expected "
                + std::to_string(item_size)
                + " instead.");
        // LCOV_EXCL_STOP
    }

    // WARNING: for the adjust_offsets() to work properly we MUST have this
    //          erase() before it; otherwise there would be a mismatch
    //
    f->sub_structures().erase(array.begin() + idx);

    // decrement the array counter and save the new value
    //
    --size;
    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_ARRAY8:
        f_buffer->pwrite(&size, sizeof(uint8_t), f->offset());
        break;

    case struct_type_t::STRUCT_TYPE_ARRAY16:
        f_buffer->pwrite(&size, sizeof(uint16_t), f->offset());
        break;

    case struct_type_t::STRUCT_TYPE_ARRAY32:
        f_buffer->pwrite(&size, sizeof(uint32_t), f->offset());
        break;

    default: // LCOV_EXCL_LINE
        // we already throw above so we should never reach this case
        //
        snapdev::NOT_REACHED(); // LCOV_EXCL_LINE
        break;

    }

    adjust_offsets(offset - 1, -item_size);

    verify_buffer_size();
}


buffer_t structure::get_buffer(std::string const & field_name) const
{
    auto f(get_field(field_name));

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_BUFFER8:
    case struct_type_t::STRUCT_TYPE_BUFFER16:
    case struct_type_t::STRUCT_TYPE_BUFFER32:
        break;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_BUFFER8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BUFFER16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BUFFER32)
                + "\".");

    }

    ssize_t const field_size(f->type_field_size());
    std::uint32_t size(0);
    f_buffer->pread(&size, field_size, f->offset());
    if(size != f->size())
    {
        throw corrupted_data(
                  "the existing buffer size ("
                + std::to_string(size)
                + ") and field size ("
                + std::to_string(f->size())
                + ") do not match.");
    }

    buffer_t result;
    result.resize(size);
    f_buffer->pread(result.data(), size, f->offset() + field_size);
    return result;
}


void structure::set_buffer(std::string const & field_name, buffer_t const & value)
{
    auto f(get_field(field_name));

    switch(f->type())
    {
    case struct_type_t::STRUCT_TYPE_BUFFER8:
    case struct_type_t::STRUCT_TYPE_BUFFER16:
    case struct_type_t::STRUCT_TYPE_BUFFER32:
        break;

    default:
        throw type_mismatch(
                  "this field type is \""
                + to_string(f->type())
                + "\" but we expected one of \""
                + to_string(struct_type_t::STRUCT_TYPE_BUFFER8)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BUFFER16)
                + ", "
                + to_string(struct_type_t::STRUCT_TYPE_BUFFER32)
                + "\".");

    }

    ssize_t const field_size(f->type_field_size());
    std::uint64_t const max(1ULL << (field_size * 8));
    std::uint64_t const size(value.size());
    if(size >= max)
    {
        throw out_of_range(
                  "size of input buffer ("
                + std::to_string(size)
                + ") too large to send it to the buffer; the maximum permitted by this field is "
                + std::to_string(max - 1ULL)
                + ".");
    }

    if(f->size() > size)
    {
        // existing buffer too large, make it the right size (smaller)
        //
        f_buffer->perase(f->size() - size, f->offset() + field_size + size);

        f_buffer->pwrite(&size, field_size, f->offset());
        f_buffer->pwrite(value.data(), size, f->offset() + field_size);

        std::int64_t const adjust(size - f->size());

        f->set_size(size);

        adjust_offsets(f->offset(), adjust);
    }
    else if(f->size() < size)
    {
        // existing buffer too small, enlarge it
        //
        //     |*                   |
        //     | <------>           |
        //     |         <--------->|
        //     ^^   ^        ^
        //     ||   |        |
        //     ||   |        +----- new space (pinsert)
        //     ||   |
        //     ||   +---- existing space (pwrite)
        //     ||
        //     |+------ buffer size
        //     |
        //     +----- f->f_offset
        //
        // Size of each element is:
        //
        //     buffer size -- field_size
        //     existing space -- f->size()
        //     new space -- value.size() - f->size()
        //

        f_buffer->pwrite(&size, field_size, f->offset());

        f_buffer->pwrite(value.data(), f->size(), f->offset() + field_size);

        f_buffer->pinsert(value.data() + f->size(), size - f->size(), f->offset() + field_size + f->size());

        std::int64_t const adjust(size - f->size());

        f->set_size(size);

        adjust_offsets(f->offset(), adjust);
    }
    else
    {
        // same size, just overwrite
        //
        f_buffer->pwrite(value.data(), size, f->offset() + field_size);
    }
}


std::uint64_t structure::parse() const
{
    if(f_fields_by_name.empty())
    {
        f_original_size = parse_descriptions(f_start_offset);
    }

    return f_original_size;
}


std::uint64_t structure::parse_descriptions(std::uint64_t offset) const
{
    field_t::pointer_t previous;
    for(struct_description_t const * def(f_descriptions);
        def->f_type != struct_type_t::STRUCT_TYPE_END;
        ++def)
    {
        std::string field_name(def->f_field_name);

        field_t::pointer_t f(std::make_shared<field_t>(def));
        if(previous != nullptr)
        {
            previous->set_next(f);
            f->set_previous(previous);
        }
        f->set_offset(offset);
        bool has_sub_defs(false);
        std::size_t bit_field(0);
        switch(def->f_type)
        {
        case struct_type_t::STRUCT_TYPE_VOID:
            break;

        case struct_type_t::STRUCT_TYPE_BITS8:
            bit_field = 8;
            [[fallthrough]];
        case struct_type_t::STRUCT_TYPE_INT8:
        case struct_type_t::STRUCT_TYPE_UINT8:
            f->set_size(1);
            offset += 1;
            break;

        case struct_type_t::STRUCT_TYPE_BITS16:
            bit_field = 16;
            [[fallthrough]];
        case struct_type_t::STRUCT_TYPE_INT16:
        case struct_type_t::STRUCT_TYPE_UINT16:
            f->set_size(2);
            offset += 2;
            break;

        case struct_type_t::STRUCT_TYPE_BITS32:
            bit_field = 32;
            [[fallthrough]];
        case struct_type_t::STRUCT_TYPE_INT32:
        case struct_type_t::STRUCT_TYPE_UINT32:
        case struct_type_t::STRUCT_TYPE_FLOAT32:
        case struct_type_t::STRUCT_TYPE_MAGIC:
        case struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION:
        case struct_type_t::STRUCT_TYPE_VERSION:
            f->set_size(4);
            offset += 4;
            break;

        case struct_type_t::STRUCT_TYPE_BITS64:
            bit_field = 64;
            [[fallthrough]];
        case struct_type_t::STRUCT_TYPE_INT64:
        case struct_type_t::STRUCT_TYPE_UINT64:
        case struct_type_t::STRUCT_TYPE_FLOAT64:
        case struct_type_t::STRUCT_TYPE_REFERENCE:
        case struct_type_t::STRUCT_TYPE_OID:
        case struct_type_t::STRUCT_TYPE_TIME:
        case struct_type_t::STRUCT_TYPE_MSTIME:
        case struct_type_t::STRUCT_TYPE_USTIME:
            f->set_size(8);
            offset += 8;
            break;

        case struct_type_t::STRUCT_TYPE_BITS128:
            bit_field = 128;
            [[fallthrough]];
        case struct_type_t::STRUCT_TYPE_INT128:
        case struct_type_t::STRUCT_TYPE_UINT128:
        case struct_type_t::STRUCT_TYPE_FLOAT128:
        case struct_type_t::STRUCT_TYPE_NSTIME:
            f->set_size(16);
            offset += 16;
            break;

        case struct_type_t::STRUCT_TYPE_BITS256:
            bit_field = 256;
            [[fallthrough]];
        case struct_type_t::STRUCT_TYPE_INT256:
        case struct_type_t::STRUCT_TYPE_UINT256:
            f->set_size(32);
            offset += 32;
            break;

        case struct_type_t::STRUCT_TYPE_BITS512:
            bit_field = 512;
            [[fallthrough]];
        case struct_type_t::STRUCT_TYPE_INT512:
        case struct_type_t::STRUCT_TYPE_UINT512:
            f->set_size(64);
            offset += 64;
            break;

        case struct_type_t::STRUCT_TYPE_CHAR:
            {
                std::size_t const size(f->type_field_size());
                f->set_size(size);
                offset += size;
            }
            break;

        case struct_type_t::STRUCT_TYPE_P8STRING:
        case struct_type_t::STRUCT_TYPE_BUFFER8:
            f->add_flags(field_t::FIELD_FLAG_VARIABLE_SIZE);
            if(f_buffer != nullptr
            && f_buffer->count_buffers() != 0)
            {
                std::uint8_t sz;
                f_buffer->pread(&sz, sizeof(sz), offset);
                f->set_size(sz);
                offset += sz;
            }
            offset += 1;
            break;

        case struct_type_t::STRUCT_TYPE_P16STRING:
        case struct_type_t::STRUCT_TYPE_BUFFER16:
            f->add_flags(field_t::FIELD_FLAG_VARIABLE_SIZE);
            if(f_buffer != nullptr
            && f_buffer->count_buffers() != 0)
            {
                std::uint16_t sz;
                f_buffer->pread(&sz, sizeof(sz), offset);
                f->set_size(sz);
                offset += sz;
            }
            offset +=  2;
            break;

        case struct_type_t::STRUCT_TYPE_P32STRING:
        case struct_type_t::STRUCT_TYPE_BUFFER32:
            f->add_flags(field_t::FIELD_FLAG_VARIABLE_SIZE);
            if(f_buffer != nullptr
            && f_buffer->count_buffers() != 0)
            {
                std::uint32_t sz;
                f_buffer->pread(&sz, sizeof(sz), offset);
                f->set_size(sz);
                offset += sz;
            }
            offset += 4;
            break;

        case struct_type_t::STRUCT_TYPE_STRUCTURE:
            // here f_size is a count, not a byte size
            //
            // note that some of the fields within the structure may be
            // of variable size but we can't mark the structure itself
            // as being of variable size
            //
            f->set_size(1);
            has_sub_defs = true;
            break;

        case struct_type_t::STRUCT_TYPE_ARRAY8:
            // here f_size is a count, not a byte size
            //
            f->add_flags(field_t::FIELD_FLAG_VARIABLE_SIZE);
            if(f_buffer != nullptr
            && f_buffer->count_buffers() != 0)
            {
                std::uint8_t sz;
                f_buffer->pread(&sz, sizeof(sz), offset);
                f->set_size(sz);
            }
            offset += 1;
            has_sub_defs = true;
            break;

        case struct_type_t::STRUCT_TYPE_ARRAY16:
            // here f_size is a count, not a byte size
            //
            f->add_flags(field_t::FIELD_FLAG_VARIABLE_SIZE);
            if(f_buffer != nullptr
            && f_buffer->count_buffers() != 0)
            {
                std::uint16_t sz;
                f_buffer->pread(&sz, sizeof(sz), offset);
                f->set_size(sz);
            }
            offset += 2;
            has_sub_defs = true;
            break;

        case struct_type_t::STRUCT_TYPE_ARRAY32:
            // here f_size is a count, not a byte size
            //
            f->add_flags(field_t::FIELD_FLAG_VARIABLE_SIZE);
            if(f_buffer != nullptr
            && f_buffer->count_buffers() != 0)
            {
                std::uint32_t sz;
                f_buffer->pread(&sz, sizeof(sz), offset);
                f->set_size(sz);
            }
            offset += 4;
            has_sub_defs = true;
            break;

        case struct_type_t::STRUCT_TYPE_RENAMED:
            has_sub_defs = true;
            break;

        case struct_type_t::STRUCT_TYPE_END: // <- this one cannot happen here, it helps with the switch() warnings, though -- LCOV_EXCL_LINE
            throw invalid_size("this field does not offer a size which can be queried."); // LCOV_EXCL_LINE

        }

        if(f_buffer != nullptr
        && f_buffer->count_buffers() != 0
        && offset > f_buffer->size())
        {
            throw corrupted_data(
                      "field \""
                    + field_name
                    + "\" is too large for the specified data buffer.");
        }

        if(def->f_sub_description != nullptr)
        {
            if(!has_sub_defs)
            {
                throw logic_error(
                          "field \""
                        + field_name
                        + "\" has its \"f_sub_description\" field set to a pointer when its type does not allow it.");
            }

            if(def->f_type != struct_type_t::STRUCT_TYPE_RENAMED)
            {
                pointer_t me(const_cast<structure *>(this)->shared_from_this());
                f->sub_structures().reserve(f->size());
                for(size_t idx(0); idx < f->size(); ++idx)
                {
                    pointer_t s(std::make_shared<structure>(def->f_sub_description, me));
                    s->set_virtual_buffer(f_buffer, offset);
                    offset = s->parse_descriptions(offset);
                    f->sub_structures().push_back(s);
                }
            }
        }
        else if(has_sub_defs)
        {
            throw logic_error(
                      "field \""
                    + field_name
                    + "\" is expected to have its \"f_sub_description\" field set to a pointer but it is null right now.");
        }
        else if(bit_field > 0)
        {
            std::string::size_type pos(field_name.find('='));
            if(pos == std::string::npos)
            {
                // LCOV_EXCL_START
                throw logic_error(
                          "field \""
                        + field_name
                        + "\" is expected to have a bit field description following its name.");
                // LCOV_EXCL_STOP
            }

            // TODO: add support for 128, 256, and 512 at some point
            //       (if it becomes useful)
            //
            if(bit_field > 64)
            {
                bit_field = 64;
            }

            size_t bit_pos(0);
            std::string::size_type end(pos);
            do
            {
                std::string::size_type start(end + 1);
                if(start >= field_name.size())  // allows for the list to end with a '/'
                {
                    break; // LCOV_EXCL_LINE
                }
                end = field_name.find_first_of(":/", start);
                std::int64_t size(1);
                std::string flag_name;
                if(end == std::string::npos)
                {
                    // no ':' or '/', we found the last flag
                    // and it has a size of 1
                    //
                    flag_name = field_name.substr(start);
                }
                else
                {
                    flag_name = field_name.substr(start, end - start);

                    // name:size separator?
                    //
                    if(field_name[end] == ':')
                    {
                        start = end + 1;

                        std::string size_str;
                        end = field_name.find('/', start);
                        if(end == std::string::npos)
                        {
                            // no '/', we found the last position
                            //
                            size_str = field_name.substr(start);
                        }
                        else
                        {
                            size_str = field_name.substr(start, end - start);
                        }

                        if(!advgetopt::validator_integer::convert_string(size_str, size))
                        {
                            throw invalid_size(
                                  "the size ("
                                + size_str
                                + ") of bit field \""
                                + flag_name
                                + "\" is invalid.");
                        }
                        if(size <= 0)
                        {
                            throw invalid_size(
                                  "the size of a bit field must be positive. \""
                                + flag_name
                                + "\" was given "
                                + std::to_string(size)
                                + " instead.");
                        }
                        if(bit_pos + size > bit_field)
                        {
                            throw invalid_size(
                                  "the total number of bits used by bit field \""
                                + flag_name
                                + "\" overflows the maximum allowed of "
                                + std::to_string(bit_field)
                                + ".");
                        }
                    }
                }
                flag_definition::pointer_t bits(std::make_shared<flag_definition>(field_name.substr(0, pos), flag_name, bit_pos, size));
                f->add_flag_definition(bits);

                bit_pos += size;
            }
            while(end != std::string::npos);

            field_name = field_name.substr(0, pos);
        }
        else if(def->f_type == struct_type_t::STRUCT_TYPE_CHAR)
        {
            std::string::size_type pos(field_name.find('='));
            if(pos == std::string::npos)
            {
                // this is caught in the field_t constructor so it should
                // not happen here
                //
                // LCOV_EXCL_START
                throw logic_error(
                          "field \""
                        + field_name
                        + "\" is expected to have a size following its name.");
                // LCOV_EXCL_STOP
            }
            field_name = field_name.substr(0, pos);
        }

SNAP_LOG_WARNING << "--- saving field named [" << field_name << "]" << SNAP_LOG_SEND;
        const_cast<structure *>(this)->f_fields_by_name[field_name] = f;

        previous = f;
    }

    return offset;
}


void structure::adjust_offsets(reference_t offset_cutoff, std::int64_t diff)
{
    if(diff == 0)
    {
        return;
    }

    // we need to adjust all the offsets after 'offset_cutoff'
    // and to do that we need to start from the very top of the
    // set of structures
    //
    pointer_t s(shared_from_this());
    for(;;)
    {
        pointer_t p(s->f_parent.lock());
        if(p == nullptr)
        {
            break;
        }
        s = p;
    }

    // we cannot use auto and call a lambda function recursively
    //
    typedef std::function<void(pointer_t)> func_t;
    func_t adjust = [&](pointer_t p)
        {
            for(auto const & f : p->f_fields_by_name)
            {
                if(f.second->offset() > offset_cutoff)
                {
                    f.second->adjust_offset(diff);
                }

                for(auto const & sub : f.second->sub_structures())
                {
                    reference_t start_offset(0);
                    virtual_buffer::pointer_t buffer(sub->get_virtual_buffer(start_offset));
                    if(start_offset > offset_cutoff)
                    {
                        sub->f_start_offset += diff;
                    }

                    adjust(sub);
                }
            }
        };

    adjust(s);
}


// LCOV_EXCL_START
void structure::display_offsets() const
{
    pointer_t s(const_cast<structure *>(this)->shared_from_this());

    // we can't use auto and call a lambda function recursively
    //
    typedef std::function<void(pointer_t, int, std::string)> func_t;
    func_t display = [&](pointer_t p, std::string::size_type indent, std::string const & index)
        {
            // we want to show the fields in the order they are defined
            // (i.e. that way the offsets increment as expected and it's
            // easier for the programmer to see possible issues)
            //
            std::string prefix(indent, ' ');
            prefix += index;
            for(struct_description_t const * def(p->f_descriptions);
                    def->f_type != struct_type_t::STRUCT_TYPE_END;
                    ++def)
            {
                auto const f(p->get_field(def->f_field_name));

                std::cout
                    << prefix
                    << f->field_name();
                if(def->f_type == struct_type_t::STRUCT_TYPE_RENAMED)
                {
                    std::cout
                        << " -> "
                        << f->new_name();
                }
                std::cout
                    << ": 0x"
                    << snapdev::int_to_hex(f->offset())
                    << " (size: "
                    << f->size()
                    << ")\n";

                int idx(0);
                for(auto const & sub : f->sub_structures())
                {
reference_t start_offset(0);
virtual_buffer::pointer_t buffer(sub->get_virtual_buffer(start_offset));
std::cout << "--- start offset: 0x" << snapdev::int_to_hex(start_offset)
<< (sub->get_field(sub->f_descriptions->f_field_name)->offset() != start_offset ? " --- DIFFERENT" : "")
<< '\n';
                    std::string sub_index;
                    sub_index += '[';
                    sub_index += std::to_string(idx);
                    sub_index += ']';
                    ++idx;

                    display(sub, indent + 2, sub_index);
                }
            }
        };

    display(s, 0, std::string());
    std::cout << std::flush;
}
// LCOV_EXCL_STOP


void structure::verify_buffer_size()
{
#ifdef _DEBUG
    if(f_buffer != nullptr)
    {
        pointer_t s;
        for(s = shared_from_this(); s->parent() != nullptr; s = s->parent());
        f_verify_offset = cppthread::gettid();
        std::size_t const size(s->get_current_size());
        f_verify_offset = 0;
        if(f_buffer->size() != size)
        {
            // LCOV_EXCL_START
            throw logic_error(
                    "buffer ("
                    + std::to_string(f_buffer->size())
                    + ") and current ("
                    + std::to_string(s->get_current_size())
                    + ") sizes do not match.");
            // LCOV_EXCL_STOP
        }
    }
#endif
}



std::ostream & operator << (std::ostream & out, version_t const & v)
{
    return out << v.to_string();
}



} // namespace prinbee
// vim: ts=4 sw=4 et
