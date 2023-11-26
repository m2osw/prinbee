// Copyright (c) 2019-2022  Made to Order Software Corp.  All Rights Reserved
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
 * \brief This file defines a journal.
 *
 * A journal is a basic system that saves messages in a journal (a file)
 * before forwarding it to the next systems.
 *
 * This implementation is specific to the eventdispatcher in that is knows
 * about the `ed::message` structure which is serialize to save it in the
 * journal.
 *
 * Additional information can be found in the implementation file (.cpp).
 */

// C++
//
#include    <snapdev/timespec_ex.h>


// C++
//
#include    <fstream>
#include    <memory>



namespace prinbee
{



typedef std::string     request_id_t;


struct in_event_t
{
    request_id_t        f_request_id = std::string();
    std::size_t         f_size = 0;
    void *              f_data = nullptr;
};


struct out_event_t
{
    request_id_t        f_request_id = std::string();
    std::size_t         f_size = 0;
    std::vector<std::uint8_t>
                        f_data = std::vector<std::uint8_t>();
};


enum class status_t : std::uint8_t
{
    STATUS_UNKNOWN,       // equivalent to a "null"

    STATUS_READY,
    STATUS_FORWARDED,
    STATUS_ACKNOWLEDGED,
    STATUS_COMPLETED,

    STATUS_FAILED,        // TBD: maybe have a clearer reason for failure since we have another 250 numbers available?
};


enum class file_management_t : std::uint8_t
{
    FILE_MANAGEMENT_KEEP,
    FILE_MANAGEMENT_TRUNCATE,
    FILE_MANAGEMENT_DELETE,
};


enum class replay_order_t : std::uint8_t
{
    REPLAY_ORDER_REQUEST_ID,
    REPLAY_ORDER_EVENT_TIME,
};


class journal
{
public:
    typedef std::shared_ptr<journal>
                                pointer_t;

                                journal(std::string const & path);

    bool                        is_valid() const;
    bool                        sync();

    // options
    //
    bool                        set_maximum_number_of_files(std::uint32_t maximum_number_of_files);
    bool                        set_maximum_size(std::uint32_t maximum_size);
    bool                        set_maximum_events(std::uint32_t maximum_events);
    bool                        set_sync(bool sync);
    bool                        set_file_management(file_management_t file_management);
    bool                        set_replay_order(replay_order_t replay_order);
    bool                        set_compress_when_full(bool compress_when_full);

    // events status
    //
    bool                        add_event(
                                    request_id_t const & request_id,
                                    in_event_t const & event,
                                    snapdev::timespec_ex const & event_time);
    bool                        event_forwarded(request_id_t const & request_id);
    bool                        event_acknowledged(request_id_t const & request_id);
    bool                        event_completed(request_id_t const & request_id);

    // events replay
    //
    bool                        empty() const;
    std::size_t                 size() const;
    void                        rewind();
    bool                        next_event(out_event_t & event, bool by_time = false);

private:
    typedef std::shared_ptr<std::fstream>   event_file_t;

    struct location
    {
        typedef std::shared_ptr<location>
                                    pointer_t;
        typedef std::map<request_id_t, pointer_t>
                                    location_map_t;
        typedef std::map<snapdev::timespec_ex, pointer_t>
                                    timebased_replay_map_t;

        request_id_t                f_request_id = request_id_t();
        snapdev::timespec_ex        f_event_time = snapdev::timespec_ex();
        status_t                    f_status = status_t::STATUS_UNKNOWN;
        std::uint8_t                f_file_index = 0;
        std::uint32_t               f_offset = 0;
        std::uint32_t               f_size = 0;
    };

    bool                        load_configuration();
    bool                        save_configuration();
    bool                        load_event_locations(bool compress = false);
    bool                        append_new_event();
    bool                        update_event_status(request_id_t const & request_id, status_t status);
    event_file_t                get_event_file(std::uint8_t index, bool create = false);
    std::string                 get_filename(std::uint8_t index);
    void                        sync_if_requested(event_file_t file);

    std::string                 f_path = std::string();
    bool                        f_valid = false;
    bool                        f_can_be_compressed = false;

    // options (from .conf file)
    //
    bool                        f_sync = false;
    bool                        f_using_alternate_file = false;
    bool                        f_compress_when_full = false;
    file_management_t           f_file_management = file_management_t::FILE_MANAGEMENT_KEEP;
    replay_order_t              f_replay_order = replay_order_t::REPLAY_ORDER_REQUEST_ID;
    std::uint8_t                f_maximum_number_of_files = 2;
    std::uint32_t               f_maximum_file_size = 1024 * 1024;
    std::uint32_t               f_maximum_events = 4096;

    // the actual journal data
    //
    std::uint8_t                f_current_file_index = 0;
    std::vector<event_file_t>   f_event_files = std::vector<event_file_t>();
    location::location_map_t    f_event_locations = location::location_map_t();
    location::timebased_replay_map_t
                                f_timebased_replay = location::timebased_replay_map_t();
    location::location_map_t::iterator 
                                f_event_locations_iterator = location::location_map_t::iterator();
    location::timebased_replay_map_t::iterator
                                f_timebased_replay_iterator = location::timebased_replay_map_t::iterator();
    std::vector<std::uint32_t>  f_next_append = std::vector<std::uint32_t>();
};



} // namespace prinbee
// vim: ts=4 sw=4 et
