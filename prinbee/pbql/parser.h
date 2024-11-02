// Copyright (c) 2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Lexer of the Prinbee Query Language.
 *
 * The Pribee Query Language (PBQL) is an SQL-like language. This file
 * defines the classes supported by the lexer.
 */

// self
//
#include    <prinbee/pbql/lexer.h>


//// advgetopt
////
//#include    <advgetopt/conf_file.h>
//#include    <advgetopt/validator_integer.h>
//
//
//// snaplogger
////
//#include    <snaplogger/message.h>
//
//
//// snapdev
////
//#include    <snapdev/hexadecimal_string.h>
//#include    <snapdev/mkdir_p.h>
//#include    <snapdev/pathinfo.h>
//#include    <snapdev/stream_fd.h>
//#include    <snapdev/unique_number.h>
//
//
//// C
////
//#include    <linux/fs.h>
//#include    <sys/ioctl.h>



namespace prinbee
{
namespace pbql
{



class parser
{
public:
                        parser(lexer::pointer_t l);

    void                run();

private:
    void                expect_semi_colon(std::string const & command);

    void                parse_alter_index();
    void                parse_alter_table();

    lexer::pointer_t    f_lexer = lexer::pointer_t();
};



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
