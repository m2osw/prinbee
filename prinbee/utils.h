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
#pragma once


/** \file
 * \brief Utility functions.
 *
 * A few utility functions used within prinbee.
 *
 * \note
 * Functions defined here may be moved around or removed. You may want
 * to avoid using them in your code.
 */

// prinbee
//
#include    <prinbee/exception.h>



// C++
//
#include    <cstdint>
#include    <string>


// C
//
#include    <string.h>



namespace prinbee
{



constexpr std::uint64_t round_down(std::uint64_t value, std::uint64_t multiple)
{
    return value - value % multiple;
}


constexpr std::uint64_t round_up(std::uint64_t value, std::uint64_t multiple)
{
    std::uint64_t const adjusted(value + multiple - 1);
    return round_down(adjusted, multiple);
}


constexpr std::uint64_t divide_rounded_up(std::uint64_t value, std::uint64_t multiple)
{
    return (value + multiple - 1) / multiple;
}


char const *    get_default_context_path();
char const *    get_prinbee_group();
char const *    get_prinbee_user();


/** \brief Validate a field, table, or column name.
 *
 * This function checks all the characters of the specified \p name.
 *
 * If the field is a bit field, make sure to use the validate_bit_field_name()
 * function instead. It allows for the `'='` sign and flag name and size
 * definitions.
 *
 * Similary, if the field is a CHAR field, make sure to use the
 * validate_char_field_name() function, which checks for the `'='` sign
 * and a size.
 *
 * \param[in] name  The name of the field, table, column being checked.
 * \param[in] max_length  The maximum length of this name.
 *
 * \return true if the name is considered valid.
 */
constexpr bool validate_name(
      char const * const name
    , std::size_t const max_length = 255)
{
    if(max_length == 0)
    {
        throw logic_error("max_length parameter cannot be zero in validate_name().");
    }

    std::size_t const max(name == nullptr ? 0 : strlen(name));
    if(max == 0)
    {
        return false;
    }
    if(max > max_length)
    {
        return false;
    }

    char c(name[0]);
    if((c < 'a' || c > 'z')
    && (c < 'A' || c > 'Z')
    && c != '_')
    {
        return false;
    }

    for(std::size_t idx(1); idx < max; ++idx)
    {
        c = name[idx];
        if((c < 'a' || c > 'z')
        && (c < 'A' || c > 'Z')
        && (c < '0' || c > '9')
        && c != '_')
        {
            return false;
        }
    }

    return true;
}


/** \brief Validate a CHAR field name.
 *
 * This function checks all the characters of the specified \p name.
 *
 * A CHAR field must include an equal (`'='`) character followed by a
 * decimal number representing the size of the field. This is useful if
 * you want to create a sequential table (where all rows have the exact
 * same size allowing for O(1) updates whatever the row).
 *
 * This function checks that \p name matches the following regex:
 *
 * \code
 * ^[A-Za-z_][A-Za-z_0-9]*=[0-9]+$
 * \endcode
 *
 * \note
 * This function does not verify that the size is valid except that it
 * is only composed for digits.
 *
 * \param[in] name  The name of the field, table, column being checked.
 * \param[in] max_length  The maximum length of this name.
 *
 * \return true if the name is considered valid for a CHAR field.
 */
constexpr bool validate_char_field_name(
      char const * const name
    , std::size_t const max_length = 255)
{
    if(max_length == 0)
    {
        throw logic_error("max_length parameter cannot be zero in validate_char_field_name().");
    }

    std::size_t const max(name == nullptr ? 0 : strlen(name));
    if(max == 0)
    {
        return false;
    }

    char c(name[0]);
    if((c < 'a' || c > 'z')
    && (c < 'A' || c > 'Z')
    && c != '_')
    {
        return false;
    }

    std::size_t idx(1);
    for(; idx < max; ++idx)
    {
        c = name[idx];
        if((c < 'a' || c > 'z')
        && (c < 'A' || c > 'Z')
        && (c < '0' || c > '9')
        && c != '_')
        {
            break;
        }
    }

    if(c != '='
    || idx > max_length
    || idx + 1 == max)
    {
        // we need the size to be defined
        // and the name of the field cannot be longer than max_length
        //
        return false;
    }

    for(++idx; idx < max; ++idx)
    {
        c = name[idx];
        if(c < '0' || c > '9')
        {
            return false;
        }
    }

    return true;
}


/** \brief Validate the name and definition of a bit field.
 *
 * This function checks all the characters of the specified \p name assuming
 * the name is used as a bit field definition.
 *
 * The bit field definition looks like this:
 *
 * \code
 * bit_field_name=<name>[:<size>][/<name>[:<size>]/...]
 * \endcode
 *
 * Each \<name> is the name of a field. It must be at least one character.
 *
 * The optional \<size> is the number of bits in that bit field. This
 * function does not verify the validity of the size, but it makes sure
 * it is defined as a decimal number (checks that it's all digits `[0-9]+`).
 *
 * \param[in] name  The name of the field, table, column being checked.
 * \param[in] max_length  The maximum length of any of the names.
 *
 * \return true if the name is considered valid.
 */
constexpr bool validate_bit_field_name(
      char const * const name
    , std::size_t const max_length = 255)
{
    if(max_length == 0)
    {
        throw logic_error("max_length parameter cannot be zero in validate_char_field_name().");
    }

    std::size_t const max(name == nullptr ? 0 : strlen(name));
    if(max == 0)
    {
        return false;
    }

    char c(name[0]);
    if((c < 'a' || c > 'z')
    && (c < 'A' || c > 'Z')
    && c != '_')
    {
        return false;
    }

    std::size_t idx(1);
    for(; idx < max; ++idx)
    {
        c = name[idx];
        if((c < 'a' || c > 'z')
        && (c < 'A' || c > 'Z')
        && (c < '0' || c > '9')
        && c != '_')
        {
            break;
        }
    }

    if(c != '='
    || idx > max_length)
    {
        // we need at least one flag to be defined
        // and the name of the bit field cannot be longer than max_length
        //
        return false;
    }

    // state=0, reading name character 0
    // state=1, reading name character 1+
    // state=2, reading size character 0
    // state=3, reading size character 1+
    //
    int state(0);

    std::size_t name_start(0);
    for(++idx; idx < max; ++idx)
    {
        c = name[idx];
        switch(state)
        {
        case 0: // name character 0
            if((c < 'a' || c > 'z')
            && (c < 'A' || c > 'Z')
            && c != '_')
            {
                return false;
            }
            name_start = idx;
            state = 1;
            break;

        case 1:
            if(c == ':')
            {
                state = 2;
            }
            else if(c == '/')
            {
                state = 0;
            }
            else
            {
                if((c < 'a' || c > 'z')
                && (c < 'A' || c > 'Z')
                && (c < '0' || c > '9')
                && c != '_')
                {
                    return false;
                }
            }
            if(state != 1
            && idx - name_start > max_length)
            {
                return false;
            }
            break;

        case 2:
        case 3:
            if(c == '/')
            {
                if(state == 2)
                {
                    // a slash must be followed by a valid decimal number with
                    // at least one digit
                    //
                    return false;
                }
                state = 0;
            }
            else
            {
                if(c < '0' || c > '9')
                {
                    return false;
                }
                state = 3;
            }
            break;

        // LCOV_EXCL_START
        default:
            throw logic_error("unknown bit field validation state.");
        // LCOV_EXCL_STOP

        }
    }
    if(state == 0
    || state == 2
    || (state != 1 && idx - name_start > max_length))
    {
        // we expect at least one character in a name
        //
        // also a slash must be followed by a valid decimal number with
        // at least one digit
        //
        // or the last name is not followed by a size and is too long
        //
        return false;
    }

    return true;
}



} // namespace prinbee
// vim: ts=4 sw=4 et
