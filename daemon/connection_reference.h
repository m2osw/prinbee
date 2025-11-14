// Copyright (c) 2016-2025  Made to Order Software Corp.  All Rights Reserved
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

// eventdispatcher
//
#include    <eventdispatcher/connection.h>


// versiontheca
//
#include    <versiontheca/versiontheca.h>
#include    <versiontheca/decimal.h>


// C++
//
#include    <map>
#include    <memory>


// C
//
//#include    <signal.h>



namespace prinbee_daemon
{



//class prinbeed;



class connection_reference
{
public:
    typedef std::shared_ptr<connection_reference>   pointer_t;
    //typedef std::vector<pointer_t>                  vector_t;
    typedef std::map<std::string, pointer_t>        map_t;

                                connection_reference();
                                connection_reference(connection_reference const & rhs) = delete;
    virtual                     ~connection_reference();

    connection_reference &      operator = (connection_reference const & rhs) = delete;

    //prinbeed *                  get_server() const;
    void                        set_name(std::string const & name);
    std::string const &         get_name() const;
    void                        set_connection(ed::connection::pointer_t connection);
    ed::connection::pointer_t   get_connection() const;
    void                        set_protocol(versiontheca::versiontheca::pointer_t protocol);
    versiontheca::versiontheca::pointer_t
                                get_protocol() const;

private:
    //prinbeed *                  f_prinbeed = nullptr;
    std::string                 f_name = std::string();
    ed::connection::pointer_t   f_connection = ed::connection::pointer_t();
    versiontheca::versiontheca::pointer_t
                                f_protocol = versiontheca::versiontheca::pointer_t();
};



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
