// Copyright (c) 2016-2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Binary message definition.
 *
 * Prinbee primarily communicates using binary messages. This is much more
 * efficient than using the communicator daemon. This file implements the
 * binary message header used by the low level binary interface.
 */


// self
//
#include    "prinbee/network/binary_message.h"

//#include    "prinbeed.h"
//#include    "prinbee/network/binary_server_client.h"


// snaplogger
//
#include    <snaplogger/message.h>


// eventdispatcher
//
//#include    <eventdispatcher/communicator.h>


//// communicatord
////
//#include    <communicatord/names.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{


namespace
{



struct_description_t g_binary_message_header[] =
{
    define_description( // the magic
          FieldName("magic=2")
        , FieldType(struct_type_t::STRUCT_TYPE_CHAR)
        , FieldDefaultValue("bm")
    ),
    define_description( // the message name (on 4 bytes, a bit a la IFF)
          FieldName("message=4")
        , FieldType(struct_type_t::STRUCT_TYPE_CHAR)
    ),
    define_description( // size of the following buffer
          FieldName(g_name_prinbee_fld_length)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT32)
    ),
    define_description( // for: ( <expression> ) [column_id is 0 when this is defined and vice versa]
          FieldName(g_name_prinbee_fld_key_script)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    end_descriptions()
};



} // no name namespace


/** \class binary_message
 * \brief Handle the binary message header.
 *
 * This class is an implementation of the structure used to define the
 * message header used by the binary communication interface.
 */



/** \brief A binary_message to interpret a message on a binary connection.
 *
 *
 * \param[in] a  The address to listen on, it can be ANY.
 * \param[in] opts  The options received from the command line.
 */
binary_message::binary_message()
{
}


binary_server::~binary_server()
{
}


void binary_server::process_accept()
{
    // a new client just connected, create a new service_connection
    // object and add it to the ed::communicator object.
    //
    ed::tcp_bio_client::pointer_t const new_client(accept());
    if(new_client == nullptr)
    {
        // an error occurred, report in the logs
        //
        int const e(errno);
        SNAP_LOG_ERROR
            << "somehow accept() of a binary connection failed with errno: "
            << e
            << " -- "
            << strerror(e)
            << SNAP_LOG_SEND;
        return;
    }

    binary_server_client::pointer_t service(std::make_shared<binary_server_client>(new_client));
    addr::addr const remote_addr(service->get_remote_address());
    service->set_name(
              "in: "
            + remote_addr.to_ipv4or6_string(addr::STRING_IP_BRACKET_ADDRESS | addr::STRING_IP_PORT));

    if(!ed::communicator::instance()->add_connection(service))
    {
        // this should never happen here since each new creates a
        // new pointer
        //
        SNAP_LOG_ERROR
            << "new client \""
            << service->get_name()
            << "\" connection could not be added to the ed::communicator list of connections."
            << SNAP_LOG_SEND;
    }
}



} // namespace prinbee
// vim: ts=4 sw=4 et
