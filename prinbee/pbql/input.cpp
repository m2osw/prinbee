// Copyright (c) 2024-2025  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Location in the input.
 *
 * This object manages the current location in the input file or string.
 * (The CLI uses a string.)
 *
 * The object generates a string with the location which is often used
 * to display an error message if necessary. The location can also be
 * copied in each node so if the error is found at a later time (in the
 * parser or when executing the results) then it can be used with the
 * correct information.
 */

// self
//
#include    "prinbee/pbql/input.h"

#include    "prinbee/exception.h"


// snapdev
//
#include    <snapdev/file_contents.h>


// snaplogger
//
#include    <snaplogger/message.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace pbql
{



input::input(std::string const & script, std::string const & filename)
    : f_script(script)
    , f_input(f_script)
{
    f_location.set_filename(filename);
}


char32_t input::getc()
{
    if(f_ungetc_pos > 0)
    {
        --f_ungetc_pos;
        return f_ungetc[f_ungetc_pos];
    }
    char32_t c(*f_input);
    if(c != libutf8::EOS)
    {
        f_location.next_column();
    }
    ++f_input;
    if(c == '\r')
    {
        c = *f_input;
        if(c == '\n')
        {
            ++f_input;
        }
        else
        {
            c = '\n';
        }
    }
    if(c == '\n')
    {
        f_location.next_line();
    }

    return c;
}


void input::ungetc(char32_t c)
{
    if(c == libutf8::EOS)
    {
        return;
    }
    if(f_ungetc_pos >= std::size(f_ungetc))
    {
        throw out_of_range("ungetc() called too many times.");
    }
    f_ungetc[f_ungetc_pos] = c;
    ++f_ungetc_pos;
}


location const & input::get_location() const
{
    return f_location;
}


input::pointer_t create_input(std::string const & filename)
{
    snapdev::file_contents file(filename);
    if(!file.read_all())
    {
        // "not found" may be wrong (i.e. it could be a permission error)
        //
        throw file_not_found("could not read \"" + filename + "\".");
    }
    return std::make_shared<input>(file.contents(), filename);
}



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
