// Copyright (c) 2023  Made to Order Software Corp.  All Rights Reserved
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
#include    <snapdev/hexadecimal_string.h>
#include    <snapdev/mkdir_p.h>
#include    <snapdev/not_reached.h>
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


// when compressing the data, we may need an end marker
// this is used to clear any existing marker
//
constexpr std::uint8_t const g_end_marker[2] = { 0, 0 };


struct event_journal_header_t
{
    std::uint8_t        f_magic[4] = { 'E', 'V', 'T', 'J' };   // "EVTJ"
    std::uint8_t        f_major_version = 1;
    std::uint8_t        f_minor_version = 0;
    std::uint16_t       f_pad = 0;
    // event_journal_event_t    f_events[n];
};


struct event_journal_event_t
{
    typedef std::uint8_t        file_status_t;
    typedef std::uint8_t        request_id_size_t;

    std::uint8_t        f_magic[2]; // "ev"
    file_status_t       f_status;
    request_id_size_t   f_request_id_size;
    std::uint32_t       f_size;
    std::uint64_t       f_time[2];
    // std::uint8_t        f_request_id[f_request_id_size];
    // char                f_event[f_size - 24 - f_request_id_size];
};


char const *        g_journal_conf = "journal.conf";



}
// no name namespace





journal::file::file(journal * j, std::string const & filename, bool create)
    : f_filename(filename)
    , f_journal(j)
{
    f_event_file = std::make_shared<std::fstream>(filename, std::ios::in | std::ios::out | std::ios::binary);
    if(!f_event_file->good()
    && create)
    {
        // it may not exist yet, create and then try reopening
        {
            std::fstream new_file(filename, std::ios::out | std::ios::binary);
        }
        f_event_file = std::make_shared<std::fstream>(filename, std::ios::in | std::ios::out | std::ios::binary);
    }

    if(!f_event_file->good())
    {
        f_event_file.reset();
        return;
    }
}


journal::file::~file()
{
    truncate();
}


std::string const & journal::file::filename() const
{
    return f_filename;
}


bool journal::file::good() const
{
    if(f_event_file == nullptr)
    {
        return false;
    }

    return f_event_file->good();
}


bool journal::file::fail() const
{
    if(f_event_file == nullptr)
    {
        return true;
    }

    return f_event_file->fail();
}


void journal::file::clear()
{
    if(f_event_file != nullptr)
    {
        f_event_file->clear();
    }
}


void journal::file::seekg(std::ios::pos_type offset, std::ios::seekdir dir)
{
    switch(dir)
    {
    case std::ios::beg:
        f_pos_read = offset;
        break;

    case std::ios::cur:
        f_pos_read += offset;
        break;

    case std::ios::end:
        f_pos_read = size();
        break;

    }
}


void journal::file::seekp(std::ios::pos_type offset, std::ios::seekdir dir)
{
    switch(dir)
    {
    case std::ios::beg:
        f_pos_write = offset;
        break;

    case std::ios::cur:
        f_pos_write += offset;
        break;

    case std::ios::end:
        f_pos_write = size();
        break;

    }
}


/** \brief This function returns the next read position.
 *
 * We manage two offsets, a read and a write, to know where to read and/or
 * write next in the journal files. This function returns the next read
 * position. It can be updated using the seekp() function.
 *
 * If you want to get the actual position of the OS file pointer, use
 * the tell() function instead.
 *
 * \return The offset for the next write() call.
 */
std::ios::pos_type journal::file::tellg() const
{
    return f_pos_read;
}


/** \brief This function returns the next write position.
 *
 * We manage two offsets, a read and a write, to know where to read and/or
 * write next in the journal files. This function returns the next write
 * position. It can be updated using the seekp() function.
 *
 * If you want to get the actual position of the OS file pointer, use
 * the tell() function instead.
 *
 * \return The offset for the next write() call.
 */
std::ios::pos_type journal::file::tellp() const
{
    return f_pos_write;
}


/** \brief This function returns the current position of the file itself.
 *
 * This function actually calls tell() on the system file and returns that
 * position. This is useful just after a read() or a write() to get that
 * offset.
 *
 * \return The current file position.
 */
std::ios::pos_type journal::file::tell() const
{
    if(f_event_file != nullptr)
    {
        return f_event_file->tellg();
    }

    return 0;
}


std::ios::pos_type journal::file::size() const
{
    if(f_event_file != nullptr)
    {
        // note: all the read() and write() calls will do a seek before the
        //       actual system call so there is not need to save the
        //       current position here
        //
        f_event_file->seekg(0, std::ios::end);
        return f_event_file->tellg();
    }

    return 0;
}


void journal::file::write(void const * data, std::size_t size)
{
    if(f_event_file != nullptr)
    {
        f_event_file->seekp(f_pos_write, std::ios::beg);
        f_event_file->write(reinterpret_cast<char const *>(data), size);
        f_pos_write += size;
    }
}


void journal::file::read(void * data, std::size_t size)
{
    if(f_event_file != nullptr)
    {
        f_event_file->seekp(f_pos_read, std::ios::beg);
        f_event_file->read(reinterpret_cast<char *>(data), size);
        f_pos_read += size;
    }
}


void journal::file::truncate()
{
    if(f_event_file == nullptr)
    {
        return;
    }

    int const fd(snapdev::stream_fd(*f_event_file));
    if(fd == -1)
    {
        return;
    }

    file_management_t const file_management(f_journal->get_file_management());
    switch(file_management)
    {
    case file_management_t::FILE_MANAGEMENT_KEEP:
        {
            // in this case we keep all the content
            //
            std::size_t const file_size(size());
            if(file_size > static_cast<std::size_t>(f_next_append))
            {
                seekp(f_next_append);
                write(
                    reinterpret_cast<char const *>(&g_end_marker),
                    std::min(sizeof(g_end_marker), file_size - f_next_append));
            }
        }
        break;

    case file_management_t::FILE_MANAGEMENT_TRUNCATE:
    case file_management_t::FILE_MANAGEMENT_DELETE:
        {
            // make sure all previous write()'s where applied before
            // truncating or we can end up with spurious data (it's also
            // a good idea to do that if you are to delete the file so it
            // stay deleted)
            //
            f_event_file->flush();

            std::size_t const size(std::max(sizeof(event_journal_header_t), static_cast<std::size_t>(f_next_append)));
            if(size == sizeof(event_journal_header_t)
            && file_management == file_management_t::FILE_MANAGEMENT_DELETE)
            {
                ::unlink(f_filename.c_str());
                f_next_append = 0;
            }
            else
            {
                ::ftruncate(fd, size);
            }

            // those should not be necessary, but I think it makes sense to
            // fix the position if out of scope
            //
            if(f_pos_read > f_next_append)
            {
                f_pos_read = f_next_append;
            }
            if(f_pos_write > f_next_append)
            {
                f_pos_write = f_next_append;
            }
        }
        break;

    }
}


void journal::file::flush()
{
    if(f_event_file != nullptr)
    {
        f_event_file->flush();
    }
}


void journal::file::fsync()
{
    if(f_event_file != nullptr)
    {
        int const fd(snapdev::stream_fd(*f_event_file));
        if(fd != -1)
        {
            ::fsync(fd);
        }
    }
}


void journal::file::reset_event_count()
{
    f_event_count = 0;
}


void journal::file::increase_event_count()
{
    ++f_event_count;
}


std::uint32_t journal::file::get_event_count() const
{
    return f_event_count;
}


void journal::file::set_next_append(std::uint32_t offset)
{
    f_next_append = offset;
}


std::uint32_t journal::file::get_next_append() const
{
    return f_next_append;
}











journal::location::location(file::pointer_t f)
    : f_file(f)
{
}


bool journal::location::read_data(out_event_t & event, bool debug)
{
    event.f_request_id = f_request_id;
    event.f_status = f_status;
    event.f_event_time = f_event_time;
    event.f_data.resize(f_size);
    if(debug)
    {
        event.f_debug_filename = f_file->filename();
        event.f_debug_offset = f_offset;
    }
    std::size_t const header_size(sizeof(event_journal_event_t) + f_request_id.length());
    std::uint32_t const offset(f_offset + header_size);
    f_file->seekg(offset);
    f_file->read(event.f_data.data(), f_size);

    if(f_file->fail())
    {
        SNAP_LOG_CRITICAL
            << "could not read event "
            << f_request_id
            << " at "
            << offset
            << " in \""
            << f_file->filename()
            << "\"."
            << SNAP_LOG_SEND;
        return false;
    }

    return true;
}


journal::file::pointer_t journal::location::get_file() const
{
    return f_file;
}


void journal::location::set_request_id(std::string const & request_id)
{
    f_request_id = request_id;
}


snapdev::timespec_ex journal::location::get_event_time() const
{
    return f_event_time;
}


void journal::location::set_event_time(snapdev::timespec_ex const & event_time)
{
    f_event_time = event_time;
}


status_t journal::location::get_status() const
{
    return f_status;
}


void journal::location::set_status(status_t status)
{
    f_status = status;
}


void journal::location::set_file_index(std::uint8_t file_index)
{
    f_file_index = file_index;
}


std::uint32_t journal::location::get_offset() const
{
    return f_offset;
}


void journal::location::set_offset(std::uint32_t offset)
{
    f_offset = offset;
}


void journal::location::set_size(std::uint32_t size)
{
    f_size = size;
}


bool journal::location::write_new_event(in_event_t const & event)
{
    if(f_file->get_next_append() == 0)
    {
        event_journal_header_t journal_header;
        f_file->seekp(0);
        f_file->write(&journal_header, sizeof(journal_header));
        f_file->set_next_append(sizeof(journal_header));
    }

    f_request_id = event.f_request_id;
    f_status = status_t::STATUS_READY;
    f_offset = f_file->get_next_append();
    f_size = event.f_size;

    event_journal_event_t event_header;
    event_header.f_magic[0] = 'e';
    event_header.f_magic[1] = 'v';
    event_header.f_status = static_cast<event_journal_event_t::file_status_t>(f_status);
    event_header.f_request_id_size = f_request_id.length();
    event_header.f_size = sizeof(event_journal_event_t)
                        + f_request_id.length()
                        + event.f_size;
    event_header.f_time[0] = f_event_time.tv_sec;
    event_header.f_time[1] = f_event_time.tv_nsec;

    f_file->seekp(f_offset);
    f_file->write(reinterpret_cast<char const *>(&event_header), sizeof(event_header));
    f_file->write(f_request_id.data(), f_request_id.length());
    f_file->write(reinterpret_cast<char const *>(event.f_data), f_size);
    if(f_file->fail())
    {
        // LCOV_EXCL_START
        SNAP_LOG_FATAL
            << "failed write_new_event() while writing."
            << SNAP_LOG_SEND;
        return false;
        // LCOV_EXCL_STOP
    }

    f_file->set_next_append(f_file->tell());
    f_file->increase_event_count();

    return true;
}












journal::journal(std::string const & path)
    : f_path(path)
{
    if(snapdev::mkdir_p(path) == 0)
    {
        if(load_configuration())
        {
            f_valid = true;
        }

        load_event_locations(false);
    }
}


bool journal::is_valid() const
{
    return f_valid;
}


bool journal::set_maximum_number_of_files(std::uint32_t maximum_number_of_files)
{
    if(maximum_number_of_files < JOURNAL_MINIMUM_NUMBER_OF_FILES
    || maximum_number_of_files > JOURNAL_MAXIMUM_NUMBER_OF_FILES)
    {
        throw out_of_range(
              "maximum number of files ("
            + std::to_string(maximum_number_of_files)
            + ") is out of range: ["
            + std::to_string(JOURNAL_MINIMUM_NUMBER_OF_FILES)
            + ".."
            + std::to_string(JOURNAL_MAXIMUM_NUMBER_OF_FILES)
            + "]");
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
    f_event_files.resize(f_maximum_number_of_files);

    return save_configuration();
}


bool journal::set_maximum_file_size(std::uint32_t maximum_file_size)
{
    if(maximum_file_size < JOURNAL_MINIMUM_FILE_SIZE)
    {
        f_maximum_file_size = JOURNAL_MINIMUM_FILE_SIZE;
    }
    else if(maximum_file_size > JOURNAL_MAXIMUM_FILE_SIZE)
    {
        f_maximum_file_size = JOURNAL_MAXIMUM_FILE_SIZE;
    }
    else
    {
        f_maximum_file_size = maximum_file_size;
    }
    return save_configuration();
}


bool journal::set_maximum_events(std::uint32_t maximum_events)
{
    if(maximum_events < JOURNAL_MINIMUM_EVENTS)
    {
        f_maximum_events = JOURNAL_MINIMUM_EVENTS;
    }
    else if(maximum_events > JOURNAL_MAXIMUM_EVENTS)
    {
        f_maximum_events = JOURNAL_MAXIMUM_EVENTS;
    }
    else
    {
        f_maximum_events = maximum_events;
    }
    return save_configuration();
}


bool journal::set_sync(sync_t sync)
{
    f_sync = sync;
    return save_configuration();
}


file_management_t journal::get_file_management() const
{
    return f_file_management;
}


bool journal::set_file_management(file_management_t file_management)
{
    switch(file_management)
    {
    case file_management_t::FILE_MANAGEMENT_KEEP:
    case file_management_t::FILE_MANAGEMENT_TRUNCATE:
    case file_management_t::FILE_MANAGEMENT_DELETE:
        break;

    default:
        throw invalid_parameter("unsupported file management number");

    }
    f_file_management = file_management;
    return save_configuration();
}


bool journal::set_replay_order(replay_order_t replay_order)
{
    switch(replay_order)
    {
    case replay_order_t::REPLAY_ORDER_REQUEST_ID:
    case replay_order_t::REPLAY_ORDER_EVENT_TIME:
        break;

    default:
        throw invalid_parameter("unsupported replay order number");

    }
    f_replay_order = replay_order;
    return save_configuration();
}


bool journal::set_compress_when_full(bool compress_when_full)
{
    f_compress_when_full = compress_when_full;
    return save_configuration();
}


/** \brief Add \p event to the journal.
 *
 * This function adds the \p event to the journal and saves it to disk.
 * If you asked for synchronized I/O, the function only returns after the
 * data was commited to disk.
 *
 * The \p event_time is expected to be set to snapdev::now(). If another
 * event happened at exactly the same time, the second one being added gets
 * its time updated (+1) so both events can also be distinguished by time.
 * The change gets returned in your \p event_time variable.
 *
 * \warning
 * If that event (as defined by the event request identifier) already exists,
 * then the function ignores the request and returns false.
 *
 * \param[in] event  The event data and metadata.
 * \param[in] event_time  The time when the event occurred. May be updated
 * by this function if an ealier event already used this time.
 *
 * \return true if the event was added as to the journal.
 */
bool journal::add_event(
    in_event_t const & event,
    snapdev::timespec_ex & event_time)
{
    if(f_event_locations.contains(event.f_request_id))
    {
        SNAP_LOG_FATAL
            << "request_id already exists in the list of events, it cannot be re-added."
            << SNAP_LOG_SEND;
        return false;
    }

    if(event.f_request_id.empty()
    || event.f_request_id.length() > 255)
    {
        SNAP_LOG_FATAL
            << "request_id must be between 1 and 255 characters."
            << SNAP_LOG_SEND;
        return false;
    }

    std::size_t event_size(sizeof(event_journal_event_t));
    event_size += event.f_request_id.length();
    event_size += event.f_size;

    while(f_timebased_replay.contains(event_time))
    {
        ++event_time;
    }

    // if the file can be compessed, we need up to two attempts, hence
    // the extra loop
    //
    bool compress_when_full(f_compress_when_full && f_can_be_compressed);
    for(int attempts(0); attempts < 2; ++attempts)
    {
        for(int count(0); count < f_maximum_number_of_files; ++count)
        {
            file::pointer_t f(get_event_file(f_current_file_index, true));
            if(f == nullptr)
            {
                SNAP_LOG_FATAL
                    << "could not retrieve/create event file."
                    << SNAP_LOG_SEND;
                return false;
            }

            if(f->get_next_append() + event_size < f_maximum_file_size
            && f->get_event_count() < f_maximum_events)
            {
                // if file is still empty, it was not yet created and thus
                // it requires a EVTJ header first
                //
                location::pointer_t l(std::make_shared<location>(f));
                l->set_event_time(event_time);
                l->set_file_index(f_current_file_index);

                l->write_new_event(event);

                sync_if_requested(f);

                f_event_locations[event.f_request_id] = l;
                f_timebased_replay[event_time] = l;
                return true;
            }

            // too large, try the next file
            //
            ++f_current_file_index;
            if(f_current_file_index >= f_maximum_number_of_files)
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


bool journal::event_failed(request_id_t const & request_id)
{
    return update_event_status(request_id, status_t::STATUS_FAILED);
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


bool journal::next_event(out_event_t & event, bool by_time, bool debug)
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

    return l->read_data(event, debug);
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
        std::string const sync(config->get_parameter("sync"));
        if(sync == "none")
        {
            f_sync = sync_t::SYNC_NONE;
        }
        else if(sync == "flush")
        {
            f_sync = sync_t::SYNC_FLUSH;
        }
        else if(sync == "full")
        {
            f_sync = sync_t::SYNC_FULL;
        }
        else
        {
            SNAP_LOG_WARNING
                << "unknown sync type \""
                << sync
                << "\"."
                << SNAP_LOG_SEND;
        }
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
        if(max < JOURNAL_MINIMUM_NUMBER_OF_FILES)
        {
            max = JOURNAL_MINIMUM_NUMBER_OF_FILES;
        }
        else if(max > JOURNAL_MAXIMUM_NUMBER_OF_FILES)
        {
            max = JOURNAL_MAXIMUM_NUMBER_OF_FILES;
        }
        f_maximum_number_of_files = max;
    }
    f_event_files.resize(f_maximum_number_of_files);

    if(config->has_parameter("maximum_file_size"))
    {
        std::string const maximum_file_size(config->get_parameter("maximum_file_size"));
        std::int64_t max(0);
        advgetopt::validator_integer::convert_string(maximum_file_size, max);
        if(max < JOURNAL_MINIMUM_FILE_SIZE)
        {
            max = JOURNAL_MINIMUM_FILE_SIZE;
        }
        else if(max > JOURNAL_MAXIMUM_FILE_SIZE)
        {
            max = JOURNAL_MAXIMUM_FILE_SIZE;
        }
        f_maximum_file_size = max;
    }

    if(config->has_parameter("maximum_events"))
    {
        std::string const maximum_events(config->get_parameter("maximum_events"));
        std::int64_t max(0);
        advgetopt::validator_integer::convert_string(maximum_events, max);
        if(max < JOURNAL_MINIMUM_EVENTS)
        {
            max = JOURNAL_MINIMUM_EVENTS;
        }
        else if(max > JOURNAL_MAXIMUM_EVENTS)
        {
            max = JOURNAL_MAXIMUM_EVENTS;
        }
        f_maximum_events = max;
    }

    return true;
}


bool journal::save_configuration()
{
    advgetopt::conf_file_setup setup(get_configuration_filename());
    advgetopt::conf_file::pointer_t config(advgetopt::conf_file::get_conf_file(setup));

    std::string sync;
    switch(f_sync)
    {
    case sync_t::SYNC_NONE:
        sync = "none";
        break;

    case sync_t::SYNC_FLUSH:
        sync = "flush";
        break;

    case sync_t::SYNC_FULL:
        sync = "full";
        break;

    }
    config->set_parameter(
        std::string(),
        "sync",
        sync);

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
        file::pointer_t f(get_event_file(index));
        if(f == nullptr)
        {
            continue;
        }
        f->reset_event_count();
        std::size_t const file_size(f->size());
        f->seekg(0);
        event_journal_header_t journal_header;
        f->read(reinterpret_cast<char *>(&journal_header), sizeof(journal_header));
        if(f->fail())
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

        bool found_compress_offset(false);
        bool good(true);
        while(good)
        {
            std::ios::pos_type const offset(f->tell());
            event_journal_event_t event_header;
            f->read(&event_header, sizeof(event_header));
            if(!f->good())
            {
                // in this case we need to clear because trying to read more
                // data than available sets the fail bit and that happens
                // here
                //
                f->clear();
                break;
            }

            // validate all the data from the header
            //
            if(event_header.f_magic[0] != 'e'
            || event_header.f_magic[1] != 'v')
            {
                // this happens when we compress a file and it is not marked
                // to be truncated (i.e. the end is marked with "\0\0"
                // instead of "ev")
                //
                if(event_header.f_magic[0] != g_end_marker[0]
                || event_header.f_magic[1] != g_end_marker[1])
                {
                    auto ascii = [](char c)
                    {
                        if(c < 0x20)
                        {
                            char buf[2] = {
                                '^',
                                static_cast<char>(c + 0x20),
                            };
                            return std::string(buf, 2);
                        }
                        else if(c > 0x7E)
                        {
                            char buf[4] = {
                                '0',
                                'x',
                                snapdev::to_hex((c >> 4) & 15),
                                snapdev::to_hex(c & 15),
                            };
                            return std::string(buf, 4);
                        }
                        else {
                            return std::string(1, c);
                        }
                    };

                    SNAP_LOG_MAJOR
                        << "found an invalid event magic ("
                        << ascii(event_header.f_magic[0])
                        << ascii(event_header.f_magic[1])
                        << ") at "
                        << offset
                        << " in \""
                        << f->filename()
                        << '"'
                        << SNAP_LOG_SEND;
                }
                break;
            }

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
                    << "found an invalid status ("
                    << static_cast<int>(event_header.f_status)
                    << ") at "
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
                SNAP_LOG_FATAL
                    << "found an invalid size ("
                    << event_header.f_size
                    << " + "
                    << offset
                    << " > "
                    << file_size
                    << ") at "
                    << offset
                    << " in \""
                    << get_filename(index)
                    << '"'
                    << SNAP_LOG_SEND;
                break;
            }
            snapdev::timespec_ex const event_time(event_header.f_time[0], event_header.f_time[1]);
            if(event_time.is_in_the_future(g_time_epsilon))
            {
                SNAP_LOG_FATAL
                    << "found an invalid date and time (a.k.a. in the future) at "
                    << offset
                    << " in \""
                    << get_filename(index)
                    << '"'
                    << SNAP_LOG_SEND;
                break;
            }

            f->increase_event_count();

            // if event has a status other than a "still working on that
            // event", then skip it, it's not part of our index (it can
            // actually be dropped from the file if `compress` is true)
            //
            if(static_cast<status_t>(event_header.f_status) != status_t::STATUS_READY
            && static_cast<status_t>(event_header.f_status) != status_t::STATUS_FORWARDED
            && static_cast<status_t>(event_header.f_status) != status_t::STATUS_ACKNOWLEDGED)
            {
                f->seekg(event_header.f_size - sizeof(event_header), std::ios::cur);
                if(!found_compress_offset)
                {
                    found_compress_offset = true;
                    f->seekp(offset);
                }
                f_can_be_compressed = true;
                continue;
            }

            std::string request_id(event_header.f_request_id_size, ' ');
            f->read(request_id.data(), event_header.f_request_id_size);
            if(!f->good())
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

            location::pointer_t l(std::make_shared<location>(f));
            l->set_request_id(request_id);
            l->set_event_time(event_time);
            l->set_status(static_cast<status_t>(event_header.f_status));
            l->set_file_index(index);
            l->set_offset(offset);
            l->set_size(data_size);

            f_event_locations[request_id] = l;
            f_timebased_replay[event_time] = l;

            if(found_compress_offset && compress)
            {
                // we are in compression mode and this event can be moved
                // "up" (lower offset), do so

                // save the header
                //
                l->set_offset(f->tellp());
                f->write(reinterpret_cast<char const *>(&event_header), sizeof(event_header));

                // save the request id string
                //
                f->write(request_id.data(), request_id.length());

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
                    std::size_t const s(std::min(remaining_size, buffer.size()));

                    // read
                    //
                    f->read(buffer.data(), s);
                    if(f->fail())
                    {
                        // TODO: handle the error better (i.e. mark event
                        //       as invalid)
                        break;
                    }

                    // write
                    //
                    f->write(buffer.data(), s);

                    remaining_size -= s;
                }

                f->set_next_append(f->tellp());
            }
            else
            {
                // skip the data, we don't need it for our index
                //
                f->seekg(data_size, std::ios::cur);

                f->set_next_append(offset + static_cast<std::ios::pos_type>(event_header.f_size));
            }
        }
    }

    rewind();

    return true;
}


journal::file::pointer_t journal::get_event_file(std::uint8_t index, bool create)
{
    if(index >= f_maximum_number_of_files)
    {
        std::stringstream ss;
        ss << "index too large in get_event_file() ("
           << std::to_string(static_cast<int>(index))
           << " > "
           << f_maximum_number_of_files
           << '.';
        SNAP_LOG_ERROR << ss.str() << SNAP_LOG_SEND;
        throw invalid_parameter(ss.str());
    }

    file::pointer_t f(f_event_files[index]);
    if(f != nullptr)
    {
        return f;
    }

    // create new file
    //
    std::string const filename(get_filename(index));
    f = std::make_shared<file>(this, filename, create);
    if(f->good())
    {
        f_event_files[index] = f;
    }
    else
    {
        f.reset();
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


bool journal::update_event_status(request_id_t const & request_id, status_t const status)
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

    switch(merge_status(it->second->get_status(), status))
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
            << static_cast<int>(it->second->get_status())
            << ", it cannot be changed to "
            << static_cast<int>(status)
            << '.'
            << SNAP_LOG_SEND;
        return false;

    }

    file::pointer_t f(it->second->get_file());
    if(f == nullptr)
    {
        // LCOV_EXCL_START
        // if we arrive here, we have a big problem since we create a location
        // with a file pointer and do not allow changes to that field
        //
        std::stringstream ss;
        ss << "location file for request identifier \""
           << request_id
           << "\" not found while attempting to update its status.";
        SNAP_LOG_EXCEPTION
            << ss
            << SNAP_LOG_SEND;
        throw logic_error(ss.str());
        // LCOV_EXCL_STOP
    }

    // TODO: a seekp() doesn't fail; instead, it may move the file pointer
    //       at the end of the file and then write there even if that's
    //       the wrong location
    //
    f->seekp(it->second->get_offset() + offsetof(event_journal_event_t, f_status));

    // type comes from the event_journal_event_t structure
    //
    event_journal_event_t::file_status_t const s(static_cast<std::uint8_t>(status));
    f->write(reinterpret_cast<char const *>(&s), sizeof(s));
    sync_if_requested(f);

    switch(status)
    {
    case status_t::STATUS_COMPLETED:
    case status_t::STATUS_FAILED:
        f_can_be_compressed = true;

        {
            auto const time_it(f_timebased_replay.find(it->second->get_event_time()));
            if(time_it == f_timebased_replay.end())
            {
                SNAP_LOG_ERROR
                    << "could not find event with time "
                    << it->second->get_event_time()
                    << " while updating event status."
                    << SNAP_LOG_SEND;
            }
            else
            {
                f_timebased_replay.erase(time_it);
            }
        }
        f_event_locations.erase(it);

        if(f_event_locations.empty())
        {
            // this allows for the file to:
            // . be reused (keep)
            // . shrink (truncate)
            // . be deleted (delete)
            //
            it->second->get_file()->set_next_append(sizeof(event_journal_header_t));
        }
        break;

    default:
        it->second->set_status(status);
        break;

    }

    return !f->fail();
}


void journal::sync_if_requested(file::pointer_t f)
{
    switch(f_sync)
    {
    case sync_t::SYNC_NONE:
        return;

    case sync_t::SYNC_FLUSH:
        f->flush();
        return;

    case sync_t::SYNC_FULL:
        f->fsync();
        return;

    }
    snapdev::NOT_REACHED();
}



} // namespace prinbee
// vim: ts=4 sw=4 et
