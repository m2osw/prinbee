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
 * \brief Script handling file header.
 *
 * The script feature is used to handle data transformation and filtering
 * for secondary filters (primarily).
 */

// self
//
#include    "prinbee/database/row.h"
#include    "prinbee/data/virtual_buffer.h"



namespace prinbee
{



buffer_t        compile_script(std::string const & script);
buffer_t        execute_script(buffer_t compiled_script, row::pointer_t row);



} // namespace prinbee
// vim: ts=4 sw=4 et
