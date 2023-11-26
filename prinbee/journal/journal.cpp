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


/** \file
 * \brief Journal implementation.
 *
 * When the Client Proxy or the Prinbee Server receive a message through
 * their communicatord, it first saves it to a journal then forward it to
 * the next stage. This implementation handles all the journal support.
 *
 * The message must have a "request_id" field for this to work. Otherwise,
 * the journal refuses the message. The "request_id" is used to find the
 * entry later and mark it as complete (properly acknowledged) or in
 * progress (sent, was not able to forward yet, etc.) or failed (the
 * message was abandonned).
 *
 * The journal makes use of multiple files like so:
 *
 * \li Journal Configuration -- `journal.conf`
 *
 * The journal.conf defines various parameters. When creating a new journal,
 * you can specify those parameters. You may be able to tweak some of these
 * later on.
 *
 * The supported parameters are:
 *
 * 1. Maximum journal file size in bytes (`max_size`)
 * 2. Maximum number of events (`max_events`)
 * 3. Which file we are currently writing to (`active_file`)
 *
 * \li Journal A and B -- `journal-a.events` and `journal-b.events`
 *
 * These two files hold the events received. This is a compressed version
 * of the events (eventdispatcher message structure converted using brs).
 *
 * Another file defines the start and size of each event in both of these
 * files.
 *
 * \li Journal "Index"
 *
 * On load, we read the existing Journal A and B files if they exist and
 * build an in memory "index" which we call locations. This index includes
 * each event request identifier, status, and time. It also has the location
 * where that event can be found in Journal A or B. The allows us to go read
 * the event and return it to the client when necessary.
 *
 * The status is one of:
 *
 * 1. Ready -- we are just writing this to the journal
 * 2. Forwarded -- the message was successfully forwarded to the server/backend
 * 3. Acknowledged -- the message was received by the server
 * 4. Completed -- the message was successfully processed by the server
 * 5. Failed -- the server sent us a failure reply
 *
 * When restarting a process with journal entries that are not yet marked
 * "Completed" or "Failed," that process takes care of these entries as if
 * it hadn't stop. It offers to re-"Forward" the events throught the replay
 * interface and wait for some form of acknowledgement.
 *
 * \note
 * Fast events do not send an ackowledgement. Instead we directly receive
 * an answer. For those, the "Acknowledged" state is skipped. Also, some
 * events may be detected as incompatible or have some other error. Those
 * directly get a "Failed" reply instead.
 *
 * \b File \b Format
 *
 * The journal file format is a small header and then events one after the
 * other. When we reload a journal, we scan the entire file to regenerate
 * the in memory index.
 *
 * \code
 *     // file header and set of events
 *     char        f_magic[4];          // "EVTJ"
 *     uint8_t     f_major_version;     // 1
 *     uint8_t     f_minor_version;     // 0
 *     uint8_t     f_pad2;
 *     uint8_t     f_pad3;
 *     event_t     f_event[n];   // n is 0 to `f_maximum_events - 1`
 *
 *     // where event_t looks like this
 *     uint32_t    f_size;
 *     uint64_t    f_time[2];
 *     uint8_t     f_status;
 *     uint8_t     f_request_id_size;
 *     uint8_t     f_request_id[f_request_id_size];
 *     char        f_event[f_size - 16 - 1 - 1 - f_request_id_size];
 * \endcode
 */

// self
//
#include    "prinbee/journal/journal.h"

#include    "prinbee/exception.h"


// advgetopt
//
#include    <advgetopt/conf_file.h>
#include    <advgetopt/validator_integer.h>


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/stream_fd.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



namespace
{



// maximum discrepancy allowed ahead of current time
//
snapdev::timespec_ex const g_time_epsilon(5, 0);


struct event_journal_header_t
{
    char                f_magic[4] = { 'E', 'V', 'T', 'J' };   // "EVTJ"
    std::uint8_t        f_major_version = 1;
    std::uint8_t        f_minor_version = 0;
    std::uint16_t       f_pad = 0;
    // event_journal_event_t    f_events[n];
};


struct event_journal_event_t
{
    typedef std::uint8_t        file_status_t;
    typedef std::uint8_t        request_id_size_t;

    std::uint32_t       f_size;
    file_status_t       f_status;
    request_id_size_t   f_request_id_size;
    std::uint16_t       f_pad; // not currently used
    std::uint64_t       f_time[2];
    // std::uint8_t        f_request_id[f_request_id_size];
    // char                f_event[f_size - 16 - 1 - 1 - f_request_id_size];
};


char const *        g_journal_conf = "journal.conf";



}
// no name namespace




journal::journal(std::string const & path)
    : f_path(path)
{
    if(load_configuration())
    {
        f_valid = true;
    }
    f_next_append.resize(f_maximum_number_of_files);
}


bool journal::is_valid() const
{
    return f_valid;
}


bool journal::set_maximum_number_of_files(std::uint32_t maximum_number_of_files)
{
    if(maximum_number_of_files < 2
    || maximum_number_of_files > 255)
    {
        throw out_of_range(
              "maximum number of files ("
            + std::to_string(maximum_number_of_files)
            + ") is out of range: [2..255]");
    }
    if(maximum_number_of_files < f_maximum_number_of_files)
    {
        // here we are supposed to make sure that if extra files exist,
        // they are empty and if not, either an error occurs or the data
        // can be moved within the files with a smaller index
        //
        SNAP_LOG_TODO
            << "the current version of the journal does not verify that decreasing the maximum number of files is doable at the time it happens."
            << SNAP_LOG_SEND;
    }
    f_maximum_number_of_files = maximum_number_of_files;
    f_next_append.resize(f_maximum_number_of_files);
    return save_configuration();
}


bool journal::set_maximum_size(std::uint32_t maximum_size)
{
    f_maximum_file_size = maximum_size;
    return save_configuration();
}


bool journal::set_maximum_events(std::uint32_t maximum_events)
{
    f_maximum_events = maximum_events;
    return save_configuration();
}


bool journal::set_sync(bool sync)
{
    f_sync = sync;
    return save_configuration();
}


bool journal::set_file_management(file_management_t file_management)
{
    f_file_management = file_management;
    return save_configuration();
}


bool journal::set_replay_order(replay_order_t replay_order)
{
    f_replay_order = replay_order;
    return save_configuration();
}


bool journal::set_compress_when_full(bool compress_when_full)
{
    f_compress_when_full = compress_when_full;
    return save_configuration();
}


bool journal::add_event(
    request_id_t const & request_id,
    in_event_t const & event,
    snapdev::timespec_ex const & event_time)
{
    if(f_event_locations.contains(request_id))
    {
        SNAP_LOG_FATAL
            << "request_id already exists in the list of events, it cannot be re-added."
            << SNAP_LOG_SEND;
        return false;
    }

    if(request_id.empty()
    || request_id.length() > 255)
    {
        SNAP_LOG_FATAL
            << "request_id must be between 1 and 255 characters."
            << SNAP_LOG_SEND;
        return false;
    }

    std::size_t event_size(sizeof(event_journal_event_t));
    event_size += request_id.length();
    event_size += event.f_size;

    bool compress_when_full(f_compress_when_full && f_can_be_compressed);
    for(int attempts(0); attempts < 2; ++attempts)
    {
        for(int count(0); count < f_maximum_number_of_files; ++count)
        {
            if(f_next_append[f_current_file_index] + event_size < f_maximum_file_size)
            {
                event_journal_event_t event_header;
                event_header.f_size = event_size;
                event_header.f_status = static_cast<int>(status_t::STATUS_READY);
                event_header.f_request_id_size = request_id.length();
                event_header.f_pad = 0;
                event_header.f_time[0] = event_time.tv_sec;
                event_header.f_time[1] = event_time.tv_nsec;

                event_file_t file(get_event_file(f_current_file_index));
                file->seekp(f_next_append[f_current_file_index]);
                file->write(reinterpret_cast<char const *>(&event_header), sizeof(event_header));
                file->write(request_id.data(), request_id.length());
                file->write(reinterpret_cast<char const *>(event.f_data), event.f_size);
                if(!file->good())
                {
                    SNAP_LOG_FATAL
                        << "request_id must be between 1 and 255 characters."
                        << SNAP_LOG_SEND;
                    return false;
                }
                sync_if_requested(file);
                return true;
            }

            // too large, try the next file
            //
            ++f_current_file_index;
            if(f_current_file_index > f_maximum_number_of_files)
            {
                f_current_file_index = 0;
            }
        }

        if(!compress_when_full)
        {
            break;
        }

        compress_when_full = false;
        load_event_locations(true);
    }

    SNAP_LOG_FATAL
        << "not enough space in any journal file to save this event."
        << SNAP_LOG_SEND;

    return false;
}


bool journal::event_forwarded(request_id_t const & request_id)
{
    return update_event_status(request_id, status_t::STATUS_FORWARDED);
}


bool journal::event_acknowledged(request_id_t const & request_id)
{
    return update_event_status(request_id, status_t::STATUS_ACKNOWLEDGED);
}


bool journal::event_completed(request_id_t const & request_id)
{
    return update_event_status(request_id, status_t::STATUS_COMPLETED);
}


bool journal::empty() const
{
    return f_event_locations.empty();
}


std::size_t journal::size() const
{
    return f_event_locations.size();
}


void journal::rewind()
{
    f_event_locations_iterator = f_event_locations.begin();
    f_timebased_replay_iterator = f_timebased_replay.begin();
}


bool journal::next_event(out_event_t & event, bool by_time)
{
    location::pointer_t l;
    if(by_time)
    {
        if(f_timebased_replay_iterator == f_timebased_replay.end())
        {
            return false;
        }
        l = f_timebased_replay_iterator->second;
        ++f_timebased_replay_iterator;
    }
    else
    {
        if(f_event_locations_iterator == f_event_locations.end())
        {
            return false;
        }
        l = f_event_locations_iterator->second;
        ++f_event_locations_iterator;
    }

    event_file_t file(get_event_file(l->f_file_index));
    if(file == nullptr)
    {
        SNAP_LOG_FATAL
            << "file for request_id "
            << l->f_request_id
            << " named \""
            << get_filename(l->f_file_index)
            << "\" not found."
            << SNAP_LOG_SEND;
        return false;
    }

    event.f_request_id = l->f_request_id;
    event.f_size = l->f_size;
    event.f_data.resize(event.f_size);
    std::uint32_t const offset(l->f_offset + sizeof(event_journal_event_t) + l->f_request_id.length());
    file->seekg(offset);
    file->read(reinterpret_cast<char *>(event.f_data.data()), event.f_size);

    if(!file->good())
    {
        SNAP_LOG_FATAL
            << "found an invalid status at "
            << offset
            << " in \""
            << f_path
            << "/journal-"
            << l->f_file_index
            << ".events\"."
            << SNAP_LOG_SEND;
        return false;
    }

    return true;
}


std::string journal::get_configuration_filename() const
{
    return f_path + '/' + g_journal_conf;
}


bool journal::load_configuration()
{
    advgetopt::conf_file_setup setup(get_configuration_filename());
    advgetopt::conf_file::pointer_t config(advgetopt::conf_file::get_conf_file(setup));

    if(config->has_parameter("sync"))
    {
        f_sync = advgetopt::is_true(config->get_parameter("sync"));
    }

    if(config->has_parameter("compress_when_full"))
    {
        f_compress_when_full = advgetopt::is_true(config->get_parameter("compress_when_full"));
    }

    if(config->has_parameter("file_management"))
    {
        std::string const file_management(config->get_parameter("file_management"));
        if(file_management == "keep")
        {
            f_file_management = file_management_t::FILE_MANAGEMENT_KEEP;
        }
        else if(file_management == "truncate")
        {
            f_file_management = file_management_t::FILE_MANAGEMENT_TRUNCATE;
        }
        else if(file_management == "delete")
        {
            f_file_management = file_management_t::FILE_MANAGEMENT_DELETE;
        }
        else
        {
            SNAP_LOG_WARNING
                << "unknown file management type \""
                << file_management
                << "\"."
                << SNAP_LOG_SEND;
        }
    }

    if(config->has_parameter("replay_order"))
    {
        std::string const replay_order(config->get_parameter("replay_order"));
        if(replay_order == "request-id")
        {
            f_replay_order = replay_order_t::REPLAY_ORDER_REQUEST_ID;
        }
        else if(replay_order == "event-time")
        {
            f_replay_order = replay_order_t::REPLAY_ORDER_EVENT_TIME;
        }
        else
        {
            SNAP_LOG_WARNING
                << "unknown replay order type \""
                << replay_order
                << "\"."
                << SNAP_LOG_SEND;
        }
    }

    if(config->has_parameter("maximum_number_of_files"))
    {
        std::string const maximum_number_of_files(config->get_parameter("maximum_number_of_files"));
        std::int64_t max(0);
        advgetopt::validator_integer::convert_string(maximum_number_of_files, max);
        if(max < 2)
        {
            max = 2;
        }
        else if(max > 255)
        {
            max = 255;
        }
        f_maximum_number_of_files = max;
        f_next_append.resize(max);
    }

    if(config->has_parameter("maximum_file_size"))
    {
        std::string const maximum_file_size(config->get_parameter("maximum_file_size"));
        std::int64_t max(0);
        advgetopt::validator_integer::convert_string(maximum_file_size, max);
        if(max < 64 * 1024)
        {
            max = 64 * 1024;
        }
        else if(max > 128 * 1024 * 1024)
        {
            max = 128 * 1024 * 1024;
        }
        f_maximum_file_size = max;
    }

    if(config->has_parameter("maximum_events"))
    {
        std::string const maximum_events(config->get_parameter("maximum_events"));
        std::int64_t max(0);
        advgetopt::validator_integer::convert_string(maximum_events, max);
        if(max < 100)
        {
            max = 100;
        }
        else if(max > 100'000)
        {
            max = 100'000;
        }
        f_maximum_events = max;
    }

    return true;
}


bool journal::save_configuration()
{
    advgetopt::conf_file_setup setup(get_configuration_filename());
    advgetopt::conf_file::pointer_t config(advgetopt::conf_file::get_conf_file(setup));

    config->set_parameter(
        std::string(),
        "sync",
        f_sync ? "true" : "false");

    config->set_parameter(
        std::string(),
        "compress_when_full",
        f_compress_when_full ? "true" : "false");

    std::string file_management;
    switch(f_file_management)
    {
    case file_management_t::FILE_MANAGEMENT_TRUNCATE:
        file_management = "truncate";
        break;

    case file_management_t::FILE_MANAGEMENT_DELETE:
        file_management = "delete";
        break;

    //case file_management_t::FILE_MANAGEMENT_KEEP:
    default:
        file_management = "keep";
        break;

    }
    config->set_parameter(
        std::string(),
        "file_management",
        file_management);

    std::string replay_order;
    switch(f_replay_order)
    {
    case replay_order_t::REPLAY_ORDER_EVENT_TIME:    
        replay_order = "event-time";
        break;

    //case replay_order_t::REPLAY_ORDER_REQUEST_ID:
    default:
        replay_order = "request-id";
        break;

    }
    config->set_parameter(
        std::string(),
        "replay_order",
        replay_order);

    config->set_parameter(
        std::string(),
        "maximum_number_of_files",
        std::to_string(static_cast<int>(f_maximum_number_of_files)));

    config->set_parameter(
        std::string(),
        "maximum_file_size",
        std::to_string(static_cast<int>(f_maximum_file_size)));

    config->set_parameter(
        std::string(),
        "maximum_events",
        std::to_string(static_cast<int>(f_maximum_events)));

    config->save_configuration(".bak", true);

    return true;
}


bool journal::load_event_locations(bool compress)
{
    std::vector<char> buffer;
    f_can_be_compressed = false;
    f_event_locations.clear();
    for(std::uint32_t index(0); index < f_maximum_number_of_files; ++index)
    {
        event_file_t file(get_event_file(index));
        if(file == nullptr)
        {
            continue;
        }
        std::size_t const file_size(file->tellg());
        file->seekg(0);
        event_journal_header_t journal_header;
        file->read(reinterpret_cast<char *>(&journal_header), sizeof(journal_header));
        if(!file->good())
        {
            continue;
        }
        if(journal_header.f_magic[0] != 'E'
        || journal_header.f_magic[1] != 'V'
        || journal_header.f_magic[2] != 'T'
        || journal_header.f_magic[3] != 'J'
        || journal_header.f_major_version != 1
        || journal_header.f_minor_version != 0)
        {
            continue;
        }

        std::uint32_t compress_offset(0);
        bool good(true);
        while(good)
        {
            std::ios::pos_type const offset(file->tellg());
            event_journal_event_t event_header;
            file->read(reinterpret_cast<char *>(&event_header), sizeof(event_header));
            if(!file->good())
            {
                break;
            }

            // validate all the data from the header
            //
            switch(static_cast<status_t>(event_header.f_status))
            {
            case status_t::STATUS_READY:
            case status_t::STATUS_FORWARDED:
            case status_t::STATUS_ACKNOWLEDGED:
            case status_t::STATUS_COMPLETED:
            case status_t::STATUS_FAILED:
                break;

            default:
                good = false;
                SNAP_LOG_FATAL
                    << "found an invalid status at "
                    << offset
                    << " in \""
                    << get_filename(index)
                    << '"'
                    << SNAP_LOG_SEND;
                continue;

            }
            ssize_t const data_size(event_header.f_size - sizeof(event_header) - event_header.f_request_id_size);
            if(event_header.f_request_id_size == 0
            || static_cast<std::size_t>(event_header.f_size + offset) > file_size
            || data_size <= 0)
            {
                good = false;
                SNAP_LOG_FATAL
                    << "found an invalid size at "
                    << offset
                    << " in \""
                    << get_filename(index)
                    << '"'
                    << SNAP_LOG_SEND;
                continue;
            }
            snapdev::timespec_ex const event_time(event_header.f_time[0], event_header.f_time[1]);
            if(event_time.is_in_the_future(g_time_epsilon))
            {
                good = false;
                SNAP_LOG_FATAL
                    << "found an invalid date and time (a.k.a. in the future) at "
                    << offset
                    << " in \""
                    << get_filename(index)
                    << '"'
                    << SNAP_LOG_SEND;
                continue;
            }

            // if event has a status other than a "still working on that
            // event", then skip it, it's not part of our index (it can
            // actually be dropped from the file)
            //
            if(static_cast<status_t>(event_header.f_status) != status_t::STATUS_READY
            && static_cast<status_t>(event_header.f_status) != status_t::STATUS_FORWARDED
            && static_cast<status_t>(event_header.f_status) != status_t::STATUS_ACKNOWLEDGED)
            {
                f_can_be_compressed = true;
                file->seekg(event_header.f_size - sizeof(event_header), std::ios::cur);
                if(file->tellp() == 0)
                {
                    file->seekp(offset);
                }
                continue;
            }

            location::pointer_t l(std::make_shared<location>());
            l->f_event_time = event_time;
            l->f_status = static_cast<status_t>(event_header.f_status);
            l->f_file_index = index;
            l->f_offset = offset;
            l->f_size = data_size;

            l->f_request_id.resize(event_header.f_request_id_size);
            file->read(l->f_request_id.data(), event_header.f_request_id_size);
            if(!file->good())
            {
                SNAP_LOG_FATAL
                    << "could not read request identifier at "
                    << offset
                    << " in \""
                    << get_filename(index)
                    << '"'
                    << SNAP_LOG_SEND;
                break;
            }
            f_event_locations[l->f_request_id] = l;
            f_timebased_replay[event_time] = l;

            std::uint32_t next_append(0);
            if(compress_offset != 0
            && compress)
            {
                // we are in compression mode and this event can be moved
                // "up" (lower offset), do so

                // save the header
                //
                l->f_offset = file->tellp();
                file->write(reinterpret_cast<char const *>(&event_header), sizeof(event_header));

                // save the request id and string
                //
                event_journal_event_t::request_id_size_t request_id_size(l->f_request_id.length());
                file->write(reinterpret_cast<char const *>(&request_id_size), sizeof(request_id_size));
                file->write(l->f_request_id.data(), request_id_size);

                // now copy the data, one block at a time
                //
                if(buffer.size() == 0)
                {
                    // copy up to 64Kb at a time
                    //
                    buffer.resize(64 * 1024);
                }
                std::size_t remaining_size(data_size);
                while(remaining_size > 0)
                {
                    ssize_t const s(std::min(remaining_size, buffer.size()));
                    file->read(buffer.data(), s);
                    file->write(buffer.data(), s);
                    remaining_size -= s;
                }

                next_append = file->tellp();
            }
            else
            {
                // skip the data, we don't need it for our index
                //
                file->seekg(data_size, std::ios::cur);

                next_append = event_header.f_size + offset;
            }
            if(f_next_append.size() <= index)
            {
                f_next_append.resize(index + 1);
            }
            f_next_append[index] = next_append;
        }
    }

    rewind();

    return true;
}


journal::event_file_t journal::get_event_file(std::uint8_t index, bool create)
{
    event_file_t f;
    if(index < f_event_files.size())
    {
        f = f_event_files[index];
        if(f != nullptr)
        {
            return f;
        }
    }

    if(!create)
    {
        return f;
    }

    // create new file
    //
    std::string filename(get_filename(index));

    f = std::make_shared<event_file_t::element_type>(filename, std::ios::in | std::ios::out | std::ios::binary);
    if(f->good())
    {
        f_event_files[index] = f;
    }
    else
    {
        f = event_file_t();
    }

    return f;
}


std::string journal::get_filename(std::uint8_t index)
{
    std::string filename(f_path);
    filename += "/journal-";
    filename += std::to_string(static_cast<int>(index));
    filename += ".events";
    return filename;
}


bool journal::update_event_status(request_id_t const & request_id, status_t status)
{
    switch(status)
    {
    case status_t::STATUS_READY:
    case status_t::STATUS_FORWARDED:
    case status_t::STATUS_ACKNOWLEDGED:
    case status_t::STATUS_COMPLETED:
    case status_t::STATUS_FAILED:
        break;

    default:
        throw invalid_parameter("the status in `update_event_status` is not valid");

    }

    auto const it(f_event_locations.find(request_id));
    if(it == f_event_locations.end())
    {
        SNAP_LOG_MAJOR
            << "location with request identifier \""
            << request_id
            << "\" not found while attempting to update its status."
            << SNAP_LOG_SEND;
        return false;
    }

    constexpr auto merge_status = [](status_t a, status_t b)
    {
        return (static_cast<int>(a) << 8) | static_cast<int>(b);
    };

    switch(merge_status(it->second->f_status, status))
    {
    case merge_status(status_t::STATUS_READY, status_t::STATUS_FORWARDED):
    case merge_status(status_t::STATUS_READY, status_t::STATUS_ACKNOWLEDGED):
    case merge_status(status_t::STATUS_READY, status_t::STATUS_COMPLETED):
    case merge_status(status_t::STATUS_READY, status_t::STATUS_FAILED):
    case merge_status(status_t::STATUS_FORWARDED, status_t::STATUS_ACKNOWLEDGED):
    case merge_status(status_t::STATUS_FORWARDED, status_t::STATUS_COMPLETED):
    case merge_status(status_t::STATUS_FORWARDED, status_t::STATUS_FAILED):
    case merge_status(status_t::STATUS_ACKNOWLEDGED, status_t::STATUS_COMPLETED):
    case merge_status(status_t::STATUS_ACKNOWLEDGED, status_t::STATUS_FAILED):
        break;

    default:
        SNAP_LOG_MAJOR
            << "location already has status "
            << static_cast<int>(it->second->f_status)
            << ", it cannot be changed to "
            << static_cast<int>(status)
            << '.'
            << SNAP_LOG_SEND;
        return false;

    }

    event_file_t file(get_event_file(it->second->f_file_index));
    if(file == nullptr)
    {
        SNAP_LOG_MAJOR
            << "location file for request identifier \""
            << request_id
            << "\" not found while attempting to update its status."
            << SNAP_LOG_SEND;
        return false;
    }

    // TODO: a seekp() doesn't fail; instead, it may move the file pointer
    //       at the end of the file and then write there even if that's
    //       the wrong location
    //
    file->seekp(it->second->f_offset + offsetof(event_journal_event_t, f_status));

    // type comes from the event_journal_event_t structure
    //
    event_journal_event_t::file_status_t const s(static_cast<std::uint8_t>(status));
    file->write(reinterpret_cast<char const *>(&s), sizeof(s));
    sync_if_requested(file);

    switch(status)
    {
    case status_t::STATUS_COMPLETED:
    case status_t::STATUS_FAILED:
        f_can_be_compressed = true;
        break;

    default:
        break;

    }

    return file->good();
}


void journal::sync_if_requested(event_file_t file)
{
    if(!f_sync)
    {
        return;
    }

    int const fd(snapdev::stream_fd(*file));
    if(fd != -1)
    {
        ::fsync(fd);
    }
}



} // namespace prinbee
// vim: ts=4 sw=4 et
