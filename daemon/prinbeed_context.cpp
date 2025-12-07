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

/** \file
 * \brief Daemon managing prinbee: context related functions.
 *
 * This file implements the context related functions. These allow the user
 * to create, drop, alter, and connect to a context.
 *
 * We support the following messages:
 *
 * \li GCTX -- get context, allows the user to connect to a context by
 *             retrieving the context metadata (CONNECT)
 * \li SCTX -- set context, this is used by the CREATE CONTEXT and
 *             ALTER CONTEXT pbql commands
 * \li LCTX -- list contexts, this command allows the user to see a list
 *             of existing contexts (SHOW CONTEXTS)
 * \li SYNC -- synchronize one context and its complex types and tables
 *
 * The update of a context is done using the SCTX between nodes. When the
 * client initiated the command, the computer that receives the SCTX message
 * is in charge of duplicating the change on all the nodes. When a node is
 * first started, it wants to make sure it is up to date before starting
 * to accept client messages. This is done with the SYNC message. The
 * synchronizer node is the one that is expected to send that message to
 * all the other computers.
 */


// self
//
#include    "prinbeed.h"

//#include    "connection_reference.h"
//#include    "node_client.h"
//#include    "node_listener.h"
//#include    "proxy_listener.h"
//#include    "direct_listener.h"



// prinbee
//
//#include    <prinbee/exception.h>
//#include    <prinbee/names.h>
//#include    <prinbee/version.h>


// communicatord
//
//#include    <communicatord/flags.h>
//#include    <communicatord/names.h>


// cluck
//
//#include    <cluck/cluck_status.h>


// cppprocess
//
//#include    <cppprocess/io_capture_pipe.h>
//#include    <cppprocess/process.h>


//// eventdispatcher
////
//#include    <eventdispatcher/names.h>


// libaddr
//
//#include    <libaddr/addr_parser.h>


// snapdev
//
//#include    <snapdev/gethostname.h>
//#include    <snapdev/stringize.h>
//#include    <snapdev/tokenize_string.h>
//#include    <snapdev/to_lower.h>


// snaplogger
//
//#include    <snaplogger/logger.h>
//#include    <snaplogger/options.h>
//#include    <snaplogger/severity.h>


// advgetopt
//
//#include    <advgetopt/advgetopt.h>
//#include    <advgetopt/exception.h>


//// C++
////
//#include    <algorithm>
//#include    <iostream>
//#include    <sstream>
//
//
//// openssl
////
//#include    <openssl/rand.h>


// last include
//
#include    <snapdev/poison.h>






namespace prinbee_daemon
{



bool prinbeed::list_contexts(payload_t::pointer_t payload)
{
    // there is nothing for us to deserialize
    //prinbee::msg_list_context_t l;
    //if(!payload.f_message->deserialize_list_context_message(l))
    //{
    //    return false; // LCOV_EXCL_LINE
    //}

    advgetopt::string_list_t const list(f_context_manager->get_context_list());
    payload->f_message->create_list_contexts_message(list);
    payload->send_message(payload->f_message);

    return false;
}


bool prinbeed::get_context(payload_t::pointer_t payload)
{
    prinbee::msg_context_t c;
    if(!payload->f_message->deserialize_context_message(c))
    {
        return false; // LCOV_EXCL_LINE
    }

    prinbee::context::pointer_t ctx(f_context_manager->get_context(c.f_context_name));
    if(ctx == nullptr)
    {
        prinbee::binary_message::pointer_t error_msg(std::make_shared<prinbee::binary_message>());
        error_msg->create_error_message(
              payload->f_message
            , prinbee::err_code_t::ERR_CODE_INVALID_PARAMETERS
            , std::string("context \"")
            + c.f_context_name
            + "\" not found.");
        payload->send_message(error_msg);
        return true;
    }

    //c.f_context_name = ... already set
    c.f_description = ctx->get_description();
    c.f_schema_version = ctx->get_schema_version();
    c.f_context_id = ctx->get_id();
    c.f_created_on = ctx->get_created_on();
    c.f_last_updated_on = ctx->get_last_updated_on();
    c.f_table_count = ctx->list_tables().size();

    prinbee::binary_message::pointer_t get_context_msg(std::make_shared<prinbee::binary_message>());
    get_context_msg->create_context_message(c);
    payload->send_message(get_context_msg);

    return false;
}


bool prinbeed::set_context(payload_t::pointer_t payload)
{
    prinbee::msg_context_t c;
    if(!payload->f_message->deserialize_context_message(c))
    {
        return false; // LCOV_EXCL_LINE
    }

    // the context name may have up to 3 pre-segments; we need to
    // extract the name itself
    //
    prinbee::context_setup cs;
    try
    {
        cs.set_name(c.f_context_name);
    }
    catch(prinbee::invalid_parameter const & e)
    {
        prinbee::binary_message::pointer_t error_msg(std::make_shared<prinbee::binary_message>());
        error_msg->create_error_message(
              payload->f_message
            , prinbee::err_code_t::ERR_CODE_INVALID_PARAMETERS
            , std::string("invalid context name: ")
            + e.what());
        payload->send_message(error_msg);
        return true;
    }

    // get the canonicalized name
    //
    c.f_context_name = cs.get_name();

    switch(payload->f_stage)
    {
    case 0:
        send_acknowledgment(payload, prinbee::PHASE_CONTEXT_RECEIVED);

        // we need to obtain an exclusive lock first
        {
            payload->f_stage = 1;
            std::string lock_name("context::");
            lock_name += c.f_context_name;
            obtain_cluster_lock(payload, lock_name);
        }
        break;

    case 1:
        {
            // we obtained the "context::<name>" lock, go ahead and handle the changes
            //
            prinbee::context::pointer_t context(f_context_manager->get_context(c.f_context_name));
            if(context != nullptr)
            {
                // context exists, do an update if the serial number is correct
                // (i.e. changes are accepted only if the existing context
                // version + 1 is equal to the version in 'c'; if not, then
                // two set_context() happened "simultaneously" and we ignore
                // the second one with an error or the version is 1 meaning that
                // the IF NOT EXISTS was used.)
                //
                if(context->get_schema_version() + 1 != c.f_schema_version)
                {
                    prinbee::binary_message::pointer_t error_msg(std::make_shared<prinbee::binary_message>());
                    error_msg->create_error_message(
                          payload->f_message
                        , prinbee::err_code_t::ERR_CODE_UNEXPECTED_VERSION
                        , std::string("expected context version ")
                        + std::to_string(context->get_schema_version() + 1)
                        + ", got "
                        + std::to_string(c.f_schema_version)
                        + " instead.");
                    payload->send_message(error_msg);
                    return true;
                }
                prinbee::context_update new_info;
                new_info.set_schema_version(c.f_schema_version);
                //new_info.set_name("test_context"); <-- TODO we need a "new name" field... and also implement it properly
                new_info.set_description(c.f_description);
                context->update(new_info);
            }
            else
            {
                // the context does not exist, create it now
                //
                context = f_context_manager->create_context(
                      c.f_context_name
                    , c.f_description
                    , true);
            }

            send_acknowledgment(payload, prinbee::PHASE_CONTEXT_SAVED);

            // next make sure that all the other nodes are also updated
            // with the same changes (synchronization)
            //
            payload->f_stage = 2;
            push_payload(payload);
        }
        break;

    case 2:
        // when we receive ACK messages, we need to be on stage 3
        //
        payload->f_stage = 3;

        for(auto const & ref : f_connection_references)
        {
            //ed::connection::pointer_t connection(ref.second->get_connection());
            connection_type_t const type(ref.second->get_connection_type());
            if(type == connection_type_t::CONNECTION_TYPE_NODE)
            {
                prinbee::binary_message::pointer_t set_context_msg(std::make_shared<prinbee::binary_message>());
                set_context_msg->create_context_message(c);
                expect_acknowledgment(payload, set_context_msg); // you must call this before the send_message()
                send_message(ref.second->get_connection(), set_context_msg);
            }
        }

        //push_payload(payload); -- no pushing since the ACK will do that
        break;

    case 3:
        {
            prinbee::binary_message::pointer_t msg(payload->get_acknowledged_message());
            ed::connection::pointer_t peer(msg->get_acknowledged_by());
            connection_reference::pointer_t ref(find_connection_reference(peer));

            // got all the ACK messages, move on to stage 4
            //
            payload->f_stage = 4;
        }
        break;

    case 4:
        // done with the lock now
        //
        release_cluster_lock(payload);
        break;

    }

    return false;
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
