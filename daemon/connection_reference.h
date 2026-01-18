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


// libaddr
//
#include    <libaddr/addr.h>


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



enum class connection_type_t
{
    CONNECTION_TYPE_UNKNOWN,
    CONNECTION_TYPE_NODE,
    CONNECTION_TYPE_PROXY,
    CONNECTION_TYPE_DIRECT,
};


class connection_reference
{
public:
    typedef std::shared_ptr<connection_reference>   pointer_t;
    typedef std::map<ed::connection *, pointer_t>   map_t;
    //typedef std::vector<pointer_t>                  vector_t;
    //typedef std::map<std::string, pointer_t>        map_t;

                                connection_reference(ed::connection::pointer_t c, connection_type_t t);
                                connection_reference(connection_reference const & rhs) = delete;
    virtual                     ~connection_reference();

    connection_reference &      operator = (connection_reference const & rhs) = delete;

    //prinbeed *                  get_server() const;
    //void                        set_connection_type(connection_type_t type);
    connection_type_t           get_connection_type() const;
    //void                        set_name(std::string const & name);
    //std::string const &         get_name() const;
    //void                        set_connection(ed::connection::pointer_t connection);
    ed::connection::pointer_t   get_connection() const;
    addr::addr                  get_remote_address() const;
    void                        set_protocol(versiontheca::versiontheca::pointer_t protocol);
    versiontheca::versiontheca::pointer_t
                                get_protocol() const;

    void                        set_expected_ping(std::uint32_t serial_number);
    std::uint32_t               get_expected_ping();
    bool                        has_expected_ping(std::uint32_t serial_number);
    std::uint32_t               increment_no_pong_answer();

private:
    //prinbeed *                  f_prinbeed = nullptr;
    connection_type_t           f_connection_type = connection_type_t::CONNECTION_TYPE_UNKNOWN;
    //std::string                 f_name = std::string();
    ed::connection::pointer_t   f_connection = ed::connection::pointer_t();
    versiontheca::versiontheca::pointer_t
                                f_protocol = versiontheca::versiontheca::pointer_t();
    std::uint32_t               f_ping_serial_number = 0;
    std::uint32_t               f_no_pong_answer = 0;
};



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
