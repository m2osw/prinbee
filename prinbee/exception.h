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
 * \brief Prinbee Database exceptions.
 *
 * This files declares a few exceptions that the database uses when a
 * parameter is wrong or something goes wrong (can't open a file, can't
 * create a lock for the context, etc.)
 *
 * The prinbee also makes use of the snaplogger so it will emit
 * a corresponding error to the log before throwing an exception.
 */


// libexcept
//
#include    <libexcept/exception.h>


namespace prinbee
{



DECLARE_LOGIC_ERROR(logic_error);
DECLARE_LOGIC_ERROR(not_yet_implemented);

DECLARE_OUT_OF_RANGE(out_of_range);

DECLARE_MAIN_EXCEPTION(fatal_error);
DECLARE_MAIN_EXCEPTION(prinbee_exception);

// uncomment as we use these
DECLARE_EXCEPTION(prinbee_exception, block_not_found);
DECLARE_EXCEPTION(prinbee_exception, column_not_found);
DECLARE_EXCEPTION(prinbee_exception, conversion_unavailable);
DECLARE_EXCEPTION(prinbee_exception, corrupted_data);
DECLARE_EXCEPTION(prinbee_exception, defined_twice);
DECLARE_EXCEPTION(prinbee_exception, exclusive_fields);
DECLARE_EXCEPTION(prinbee_exception, field_not_found);
DECLARE_EXCEPTION(prinbee_exception, file_not_found);
DECLARE_EXCEPTION(prinbee_exception, file_not_opened);
DECLARE_EXCEPTION(prinbee_exception, file_still_in_use);
DECLARE_EXCEPTION(prinbee_exception, full);
DECLARE_EXCEPTION(prinbee_exception, id_already_assigned);
DECLARE_EXCEPTION(prinbee_exception, id_missing);
DECLARE_EXCEPTION(prinbee_exception, invalid_entity);
DECLARE_EXCEPTION(prinbee_exception, invalid_name);
DECLARE_EXCEPTION(prinbee_exception, invalid_number);
DECLARE_EXCEPTION(prinbee_exception, invalid_parameter);
DECLARE_EXCEPTION(prinbee_exception, invalid_size);
DECLARE_EXCEPTION(prinbee_exception, invalid_token);
DECLARE_EXCEPTION(prinbee_exception, invalid_type);
//DECLARE_EXCEPTION(prinbee_exception, invalid_xml);
DECLARE_EXCEPTION(prinbee_exception, io_error);
DECLARE_EXCEPTION(prinbee_exception, missing_parameter);
DECLARE_EXCEPTION(prinbee_exception, node_already_in_tree);
DECLARE_EXCEPTION(prinbee_exception, not_ready);
DECLARE_EXCEPTION(prinbee_exception, out_of_bounds);
DECLARE_EXCEPTION(prinbee_exception, page_not_found);
DECLARE_EXCEPTION(prinbee_exception, row_already_exists);
DECLARE_EXCEPTION(prinbee_exception, row_not_found);
DECLARE_EXCEPTION(prinbee_exception, schema_not_found);
DECLARE_EXCEPTION(prinbee_exception, string_not_terminated);
DECLARE_EXCEPTION(prinbee_exception, type_mismatch);
DECLARE_EXCEPTION(prinbee_exception, type_not_found);
DECLARE_EXCEPTION(prinbee_exception, unexpected_eof);
DECLARE_EXCEPTION(prinbee_exception, unexpected_token);
DECLARE_EXCEPTION(prinbee_exception, unknown_parameter);



} // namespace prinbee
// vim: ts=4 sw=4 et
