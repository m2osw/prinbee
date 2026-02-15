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

// prinbee
//
#include    <prinbee/network/binary_message.h>


// eventdispatcher
//
#include    <eventdispatcher/connection.h>


// cluck
//
#include    <cluck/cluck.h>


// cppthread
//
#include    <cppthread/worker.h>



namespace prinbee_daemon
{



class prinbeed;


struct payload_t
{
    typedef std::shared_ptr<payload_t>      pointer_t;
    typedef std::map<prinbee::message_serial_t, pointer_t>
                                            map_t; // map by message serial number

    ed::connection::pointer_t               f_peer = ed::connection::pointer_t();
    prinbee::binary_message::pointer_t      f_message = prinbee::binary_message::pointer_t();
    std::int_fast8_t                        f_stage = 0;
    cluck::cluck::pointer_t                 f_lock = cluck::cluck::pointer_t();
    prinbee::binary_message::map_t          f_acknowledgment_messages = prinbee::binary_message::map_t();
    cppthread::mutex                        f_mutex = cppthread::mutex();

    void                                    send_message(prinbee::binary_message::pointer_t msg);
    void                                    add_message_to_acknowledge(
                                                  prinbee::message_serial_t serial_number
                                                , prinbee::binary_message::pointer_t msg);
    void                                    set_acknowledged_by(
                                                  prinbee::message_serial_t serial_number
                                                , ed::connection::pointer_t peer
                                                , bool success);
    prinbee::binary_message::pointer_t      get_acknowledged_message();
};


class prinbee_worker
    : public cppthread::worker<payload_t::pointer_t>
{
public:
    typedef std::shared_ptr<prinbee_worker>     pointer_t;

                                prinbee_worker(
                                      std::string const & name
                                    , std::size_t position
                                    , typename cppthread::fifo<payload_t::pointer_t>::pointer_t in
                                    , typename cppthread::fifo<payload_t::pointer_t>::pointer_t out
                                    , prinbeed * p);
                                prinbee_worker(prinbee_worker const &) = delete;
    virtual                     ~prinbee_worker() override;

    prinbee_worker &            operator = (prinbee_worker const &) = delete;

    virtual bool                do_work() override;

private:
    void                        msg_set_context(prinbee::binary_message::pointer_t msg);

    prinbeed *                  f_prinbeed = nullptr;
};



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
