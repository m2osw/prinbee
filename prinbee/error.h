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
 * \brief Define the error class.
 *
 * Some data errors are reported using an error object. This allows the
 * library to give you an error code to check that error in your program
 * and a description of the error which can be displayed to the end user.
 */

// C++
//
#include    <memory>
#include    <string>



namespace prinbee
{


enum error_code_t
{
    ERROR_CODE_NO_ERROR,
    ERROR_CODE_INVALID_XML,
};


class error
{
public:
    typedef std::shared_ptr<error>  pointer_t;

                                    error(
                                          error_code_t code
                                        , std::string const & message);

    error_code_t                    get_error_code() const;
    std::string                     get_error_message() const;

private:
    error_code_t                    f_error_code = error_code_t::ERROR_CODE_NO_ERROR;
    std::string                     f_message = std::string();
};



} // namespace prinbee
// vim: ts=4 sw=4 et
