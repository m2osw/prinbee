// Copyright (c) 2023-2024  Made to Order Software Corp.  All Rights Reserved
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
 *     char            f_magic[4];          // "EVTJ"
 *     uint8_t         f_major_version;     // 1
 *     uint8_t         f_minor_version;     // 0
 *     uint16_t        f_pad;
 *     event_t         f_event[n];   // n is 0 to `f_maximum_events - 1`
 *
 *     // where event_t looks like this
 *     uint8_t         f_magic[2];   // "eve"
 *     uint8_t         f_status;
 *     uint8_t         f_request_id_size;
 *     uint32_t        f_size;       // total size of the event
 *     uint64_t        f_time[2];
 *     uint8_t         f_attachment_count;
 *     uint8_t         f_pad[7];
 *     attachment_t    f_attachment_offsets[f_attachment_count]; // see union below
 *     uint8_t         f_request_id[f_request_id_size];
 *     uint8_t         f_attachement[<index>][<size>];
 *
 *     // where attachment_t looks like this
 *     union attachment_t
 *     {
 *         struct inline_attachment
 *         {
 *             uint32_t        f_mode : 1;      // = 0 -- inline
 *             uint32_t        f_size : 31;
 *         };
 *         struct external_attachment
 *         {
 *             uint32_t        f_mode : 1;      // = 1 -- external file
 *             uint32_t        f_identifier : 31;
 *         };
 *     };
 *     // the size of an attachment is defined as f_size[n + 1] - f_size[n]
 *     // the last attachment size uses the event_t.f_size - f_size[n]
 *     // the attachment with an `f_identifier` are skipped to compute the size
 *     
 * \endcode
 *
 * attachments.f_mode is one of:
 *
 * 0 -- small attachment; saved inline
 * 1 -- large attachment; saved in separate file
 *
 * \li Multi-threading Support
 *
 * At the moment, the journal is not multi-thread safe. You must make sure
 * to use the journal serially.
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
#include    <snapdev/pathinfo.h>
#include    <snapdev/stream_fd.h>
#include    <snapdev/unique_number.h>


// C
//
#include    <linux/fs.h>
#include    <sys/ioctl.h>


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
    //event_journal_event_t    f_events[n];
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
    std::uint8_t        f_attachment_count;
    std::uint8_t        f_pad[7];
    //std::uint32_t       f_attachment_offsets[f_attachment_count]; -- if bit 15 is set, the offset represents a filename number
    //std::uint8_t        f_request_id[f_request_id_size];
    //std::uint8_t        f_attachments[<index>][<attachment size>]; -- <attachment size> calculated using f_attachment_offsets
};


constexpr char const *      g_journal_conf = "journal.conf";



std::string ascii(std::uint8_t c)
{
    if(c < 0x20)
    {
        char buf[2] = {
            '^',
            static_cast<char>(c + 0x40),
        };
        return std::string(buf, 2);
    }
    else if(c > 0x7E)
    {
        char buf[4] = {
            '\\',
            'x',
            snapdev::to_hex((c >> 4) & 15),
            snapdev::to_hex(c & 15),
        };
        return std::string(buf, 4);
    }
    else
    {
        return std::string(1, static_cast<char>(c));
    }

    snapdev::NOT_REACHED();
}


}
// no name namespace





void attachment::clear()
{
    f_size = 0;
    f_data = nullptr;
    f_saved_data.reset();
    f_filename.clear();
}


void attachment::set_data(void * data, off_t sz)
{
    clear();

    if(sz < 0)
    {
        throw invalid_parameter("attachment cannot have a negative size.");
    }
    if(sz > 0 && data == nullptr)
    {
        throw invalid_parameter("attachment with a size > 0 must have a non null data pointer.");
    }

    f_size = sz;
    f_data = data;
}


void attachment::save_data(void * data, off_t sz)
{
    clear();

    if(sz < 0)
    {
        throw invalid_parameter("attachment cannot have a negative size.");
    }
    if(sz > 0 && data == nullptr)
    {
        throw invalid_parameter("attachment with a size > 0 must have a non null data pointer (2).");
    }

    f_saved_data = std::make_shared<data_t>(reinterpret_cast<std::uint8_t const *>(data), reinterpret_cast<std::uint8_t const *>(data) + sz);
    f_size = sz;
    f_data = f_saved_data->data();
}


void attachment::save_data(data_t const & data)
{
    clear();

    f_saved_data = std::make_shared<data_t>(data.begin(), data.end());
    f_size = data.size();
    f_data = f_saved_data->data();
}


void attachment::set_file(std::string const & filename, off_t sz)
{
    clear();

    struct stat s;
    if(stat(filename.c_str(), &s) != 0)
    {
        int const e(errno);
        std::stringstream ss;
        ss << "file \""
           << filename
           << "\" not accessible: "
           << strerror(e)
           << ".";
        throw file_not_found(ss.str());
    }
    if((s.st_mode & S_IFMT) != S_IFREG)
    {
        std::stringstream ss;
        ss << "file \""
           << filename
           << "\" does not represent a regular file.";
        throw invalid_parameter(ss.str());
    }

    if(sz == 0)
    {
        sz = s.st_size;
    }
    else if(sz > s.st_size)
    {
        throw invalid_parameter(
                  "trying to save more data ("
                + std::to_string(sz)
                + ") than available in file attachment \""
                + filename
                + "\" ("
                + std::to_string(s.st_size)
                + ").");
    }

    f_filename = filename;
    f_size = sz;
}


off_t attachment::size() const
{
    return f_size;
}


void * attachment::data() const
{
    if(is_file() && f_data == nullptr)
    {
        const_cast<attachment *>(this)->load_file_data();
    }

    return f_data;
}


std::string const & attachment::filename() const
{
    return f_filename;
}


bool attachment::load_file_data()
{
    if(is_file() && f_saved_data == nullptr)
    {
        std::ifstream in(f_filename);
        if(!in.is_open())
        {
            throw file_not_found("file \""
                + f_filename
                + "\" not found or permission denied.");
        }
        f_saved_data = std::make_shared<data_t>(f_size);
        in.read(reinterpret_cast<char *>(f_saved_data->data()), f_size);
        if(in.fail())
        {
            f_saved_data.reset();       // LCOV_EXCL_LINE
            return false;               // LCOV_EXCL_LINE
        }

        f_data = f_saved_data->data();
    }

    return true;
}


bool attachment::empty() const
{
    return f_size == 0;
}


bool attachment::is_file() const
{
    return !f_filename.empty();
}






void in_event::set_request_id(request_id_t const & request_id)
{
    f_request_id = request_id;
}


request_id_t const & in_event::get_request_id() const
{
    return f_request_id;
}


attachment_id_t in_event::add_attachment(attachment const & a)
{
    attachment_id_t const id(f_attachments.size());
    if(id >= MAXIMUM_ATTACHMENT_COUNT)
    {
        throw full("attachment table is full, this attachment cannot be added (in_event).");
    }

    f_attachments.push_back(a);
    return id;
}


std::size_t in_event::get_attachment_size() const
{
    return f_attachments.size();
}


attachment const & in_event::get_attachment(attachment_id_t id) const
{
    if(id >= f_attachments.size())
    {
        throw out_of_range("identifier out of range retrieving attachment from in_event.");
    }

    return f_attachments[id];
}







void out_event::set_request_id(request_id_t const & request_id)
{
    f_request_id = request_id;
}


request_id_t const & out_event::get_request_id() const
{
    return f_request_id;
}


void out_event::set_status(status_t status)
{
    switch(status)
    {
    case status_t::STATUS_UNKNOWN:
    case status_t::STATUS_READY:
    case status_t::STATUS_FORWARDED:
    case status_t::STATUS_ACKNOWLEDGED:
    case status_t::STATUS_COMPLETED:
    case status_t::STATUS_FAILED:
        break;

    default:
        throw invalid_parameter("unsupported status number.");

    }

    f_status = status;
}


status_t out_event::get_status() const
{
    return f_status;
}


void out_event::set_event_time(snapdev::timespec_ex const & event_time)
{
    f_event_time = event_time;
}


snapdev::timespec_ex const & out_event::get_event_time() const
{
    return f_event_time;
}


attachment_id_t out_event::add_attachment(attachment const & a)
{
    attachment_id_t const id(f_attachments.size());
    if(id >= MAXIMUM_ATTACHMENT_COUNT)
    {
        throw full("attachment table is full, this attachment cannot be added (out_event).");
    }

    f_attachments.push_back(a);
    return id;
}


std::size_t out_event::get_attachment_size() const
{
    return f_attachments.size();
}


attachment const & out_event::get_attachment(attachment_id_t id) const
{
    if(id >= f_attachments.size())
    {
        throw out_of_range("identifier out of range retrieving attachment from out_event.");
    }

    return f_attachments[id];
}


void out_event::set_debug_filename(std::string debug_filename)
{
    f_debug_filename = debug_filename;
}


std::string out_event::get_debug_filename() const
{
    return f_debug_filename;
}


void out_event::set_debug_offset(std::uint32_t debug_offset)
{
    f_debug_offset = debug_offset;
}


std::uint32_t out_event::get_debug_offset() const
{
    return f_debug_offset;
}











journal::file::file(journal * j, std::string const & filename, bool create)
    : f_filename(filename)
    , f_journal(j)
{
    f_event_file = std::make_shared<std::fstream>(f_filename, std::ios::in | std::ios::out | std::ios::binary);
    if(!f_event_file->good()
    && create)
    {
        // it may not exist yet, create and then try reopening
        {
            std::fstream new_file(f_filename, std::ios::out | std::ios::binary);
        }
        f_event_file = std::make_shared<std::fstream>(f_filename, std::ios::in | std::ios::out | std::ios::binary);
    }

    if(!f_event_file->good())
    {
        f_event_file.reset();
    }
}


journal::file::~file()
{
    truncate();
}


std::string const & journal::file::get_path() const
{
    return f_journal->get_path();
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
        return true; // LCOV_EXCL_LINE
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

    // LCOV_EXCL_START
    case std::ios::end:
        f_pos_read = size() + offset;
        break;
    // LCOV_EXCL_STOP

    }
}


void journal::file::seekp(std::ios::pos_type offset, std::ios::seekdir dir)
{
    switch(dir)
    {
    case std::ios::beg:
        f_pos_write = offset;
        break;

    // LCOV_EXCL_START
    case std::ios::cur:
        f_pos_write += offset;
        break;

    case std::ios::end:
        f_pos_write = size() + offset;
        break;
    // LCOV_EXCL_STOP

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
// LCOV_EXCL_START
std::ios::pos_type journal::file::tellg() const
{
    return f_pos_read;
}
// LCOV_EXCL_STOP


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

    return 0; // LCOV_EXCL_LINE
}


std::ios::pos_type journal::file::size() const
{
    if(f_event_file != nullptr)
    {
        // note: all the read() and write() calls will do a seek before the
        //       actual system call so there is no need to save & restore
        //       the current position here
        //
        f_event_file->seekg(0, std::ios::end);
        return f_event_file->tellg();
    }

    return 0; // LCOV_EXCL_LINE
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
        return; // LCOV_EXCL_LINE
    }

    file_management_t const file_management(f_journal->get_file_management());
    switch(file_management)
    {
    case file_management_t::FILE_MANAGEMENT_KEEP:
        if(f_next_append > 0)
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
                int const r(::unlink(f_filename.c_str()));
                if(r != 0)
                {
                    // LCOV_EXCL_START
                    int const e(errno);
                    SNAP_LOG_ERROR
                        << "unlink() generated an error ("
                        << e
                        << ", "
                        << strerror(e)
                        << ")."
                        << SNAP_LOG_SEND;
                    // LCOV_EXCL_STOP
                }
                f_next_append = 0;
            }
            else
            {
                int const r(::ftruncate(fd, size));
                if(r != 0)
                {
                    // LCOV_EXCL_START
                    int const e(errno);
                    SNAP_LOG_ERROR
                        << "ftruncate() generated an error ("
                        << e
                        << ", "
                        << strerror(e)
                        << ")."
                        << SNAP_LOG_SEND;
                    // LCOV_EXCL_STOP
                }
            }

            // those should not be necessary, but I think it makes sense to
            // fix the position if out of scope
            //
            if(f_pos_read > f_next_append)
            {
                f_pos_read = f_next_append; // LCOV_EXCL_LINE
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


void journal::file::decrease_event_count()
{
    --f_event_count;
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


std::uint32_t journal::file::get_inline_attachment_size_threshold() const
{
    return f_journal->get_inline_attachment_size_threshold();
}


attachment_copy_handling_t journal::file::get_attachment_copy_handling() const
{
    return f_journal->get_attachment_copy_handling();
}











journal::location::location(file::pointer_t f)
    : f_file(f)
{
}


bool journal::location::read_data(out_event & event, bool debug)
{
    event.set_request_id(f_request_id);
    event.set_status(f_status);
    event.set_event_time(f_event_time);
    if(debug)
    {
        event.set_debug_filename(f_file->filename());
        event.set_debug_offset(f_offset);
    }

    f_file->seekg(f_offset + sizeof(event_journal_event_t));

    std::vector<attachment_offsets_t> offsets(f_attachment_count);
    f_file->read(offsets.data(), f_attachment_count * sizeof(attachment_offsets_t));

    for(std::uint32_t idx(0); idx < f_attachment_count; ++idx)
    {
        attachment a;
        if((offsets[idx] & JOURNAL_IS_EXTERNAL_ATTACHMENT) != 0)
        {
            std::uint32_t const identifier(offsets[idx] & ~JOURNAL_IS_EXTERNAL_ATTACHMENT);
            std::string external_filename(f_file->get_path());
            external_filename += '/';
            external_filename += std::to_string(identifier);
            external_filename += ".bin";
            a.set_file(external_filename);
        }
        else
        {
            std::size_t size(f_size - offsets[idx]);
            for(std::uint32_t j(idx + 1); j < f_attachment_count; ++j)
            {
                if((offsets[j] & JOURNAL_IS_EXTERNAL_ATTACHMENT) == 0)
                {
                    size = offsets[j] - offsets[idx];
                    break;
                }
            }
            data_t data(size);
            f_file->seekg(f_offset + offsets[idx]);
            f_file->read(data.data(), size);
            a.save_data(data);
        }

        event.add_attachment(a);
    }

    if(f_file->fail())
    {
        // LCOV_EXCL_START
        SNAP_LOG_CRITICAL
            << "could not read data of event "
            << f_request_id
            << " at "
            << f_offset
            << " in \""
            << f_file->filename()
            << "\"."
            << SNAP_LOG_SEND;
        return false;
        // LCOV_EXCL_STOP
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


void journal::location::set_attachment_count(std::uint32_t count)
{
    f_attachment_count = count;
}


void journal::location::set_offset(std::uint32_t offset)
{
    f_offset = offset;
}


void journal::location::set_size(std::uint32_t size)
{
    f_size = size;
}


bool journal::location::write_new_event(in_event const & event)
{
    if(f_file->get_next_append() == 0)
    {
        event_journal_header_t journal_header;
        f_file->seekp(0);
        f_file->write(&journal_header, sizeof(journal_header));
        f_file->set_next_append(sizeof(journal_header));
    }

    f_request_id = event.get_request_id();
    f_status = status_t::STATUS_READY;
    f_offset = f_file->get_next_append();

    // compute the size of the event, including its attachments
    //
    f_size = sizeof(event_journal_event_t);
    std::size_t const number_of_attachments(event.get_attachment_size());
    f_size += number_of_attachments * sizeof(attachment_offsets_t);
    f_size += f_request_id.length();
    std::uint32_t const attachment_size_threshold(f_file->get_inline_attachment_size_threshold());
    std::vector<attachment_offsets_t> attachment_offsets(number_of_attachments);
    for(std::size_t idx(0); idx < number_of_attachments; ++idx)
    {
        attachment const & data(event.get_attachment(idx));
        if(data.size() >= attachment_size_threshold)
        {
            // too big to be saved in the main file, save in a separate file
            //
            std::string const & path(f_file->get_path());
            std::string counter_filename(path + "/counters.seq");

            // only keep the lower 31 bits of the counter
            //
            attachment_offsets_t const identifier(static_cast<attachment_offsets_t>(snapdev::unique_number(counter_filename, JOURNAL_ATTACHMENT_COUNTER_INDEX)) & ~JOURNAL_IS_EXTERNAL_ATTACHMENT);

            std::string const external_filename(path + "/" + std::to_string(identifier) + ".bin");
            if(data.is_file())
            {
                int r(0);
                if(f_file->get_attachment_copy_handling() == attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_HARDLINK)
                {
                    r = link(data.filename().c_str(), external_filename.c_str());
                }
                if(r != 0 || f_file->get_attachment_copy_handling() == attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_REFLINK)
                {
                    std::ifstream in(data.filename());
                    if(!in.is_open())
                    {
                        SNAP_LOG_FATAL
                            << "could not open \""
                            << data.filename()
                            << "\" to create a reflink from."
                            << SNAP_LOG_SEND;
                        return false;
                    }
                    std::ofstream out(external_filename);
                    if(!out.is_open())
                    {
                        SNAP_LOG_FATAL
                            << "could not open \""
                            << external_filename
                            << "\" to create a reflink to \""
                            << data.filename()
                            << "\"."
                            << SNAP_LOG_SEND;
                        return false;
                    }
                    file_clone_range range{
                        .src_fd = snapdev::stream_fd(in),
                        .src_offset = 0,
                        .src_length = static_cast<__u64>(data.size()),
                        .dest_offset = 0,
                    };
                    r = ioctl(snapdev::stream_fd(out), FICLONERANGE, &range);
                }
                if(r != 0 || f_file->get_attachment_copy_handling() == attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_FULL)
                {
                    char buf[64 * 1024];
                    std::ifstream in(data.filename());
                    if(!in.is_open())
                    {
                        SNAP_LOG_FATAL
                            << "could not open \""
                            << data.filename()
                            << "\" to create a copy from."
                            << SNAP_LOG_SEND;
                        return false;
                    }
                    std::ofstream out(external_filename);
                    if(!out.is_open())
                    {
                        SNAP_LOG_FATAL
                            << "could not open \""
                            << external_filename
                            << "\" to copy \""
                            << data.filename()
                            << "\" into."
                            << SNAP_LOG_SEND;
                        return false;
                    }
                    std::size_t size(data.size());
                    while(size > 0)
                    {
                        ssize_t const segment_size(std::min(size, sizeof(buf)));
                        in.read(buf, segment_size);
                        if(in.fail())
                        {
                            SNAP_LOG_FATAL
                                << "could not read all the input data from \""
                                << data.filename()
                                << "\" to copy into \""
                                << external_filename
                                << "\"."
                                << SNAP_LOG_SEND;
                            return false;
                        }
                        out.write(buf, segment_size);
                        if(out.fail())
                        {
                            // LCOV_EXCL_START
                            snapdev::NOT_USED(unlink(external_filename.c_str()));
                            r = -1;
                            break;
                            // LCOV_EXCL_STOP
                        }
                        size -= segment_size;
                    }
                }
                if(r != 0 || f_file->get_attachment_copy_handling() == attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK)
                {
                    r = unlink(external_filename.c_str());
                    if(r != 0 && errno != ENOENT)
                    {
                        SNAP_LOG_FATAL
                            << "could not unlink \""
                            << external_filename
                            << "\" to create a soft link."
                            << SNAP_LOG_SEND;
                        return false;
                    }
                    if(snapdev::pathinfo::is_relative(data.filename()))
                    {
                        std::string error_msg;
                        std::string const cwd(snapdev::pathinfo::getcwd(error_msg));
                        if(cwd.empty())
                        {
                            // LCOV_EXCL_START
                            SNAP_LOG_FATAL
                                << "could not determine current working directory: "
                                << error_msg
                                << SNAP_LOG_SEND;
                            return false;
                            // LCOV_EXCL_STOP
                        }

                        // TODO: consider computing a relative path from
                        //       our destination location, that way the
                        //       administrator may be able to move the data
                        //       without having to tweak the softlinks
                        //
                        std::string const absolute_path(cwd + '/' + data.filename());
                        r = symlink(absolute_path.c_str(), external_filename.c_str());
                    }
                    else
                    {
                        r = symlink(data.filename().c_str(), external_filename.c_str());
                    }
                }
                if(r != 0)
                {
                    // LCOV_EXCL_START
                    // TODO: generate messages on each error above so this
                    //       error here can be more precise (i.e. EPERM,
                    //       ENOENT, etc.) -- at the same time, it looks like
                    //       this error is not reachable because the symlink()
                    //       function is not very likely to fail... (because
                    //       we first do an unlink() and return on error
                    //       there so then down here we should not fail ever)
                    //
                    SNAP_LOG_FATAL
                        << "could not save file \""
                        << data.filename()
                        << "\" in the journal as \""
                        << external_filename
                        << "\"."
                        << SNAP_LOG_SEND;
                    return false;
                    // LCOV_EXCL_STOP
                }
            }
            else
            {
                std::ofstream out(external_filename);
                if(!out.is_open())
                {
                    SNAP_LOG_FATAL
                        << "could not open \""
                        << external_filename
                        << "\" to save large attachment."
                        << SNAP_LOG_SEND;
                    return false;
                }
                out.write(reinterpret_cast<char const *>(data.data()), data.size());
                if(out.fail())
                {
                    // LCOV_EXCL_START
                    SNAP_LOG_FATAL
                        << "failed write_new_event() while writing external file \""
                        << external_filename
                        << "\"."
                        << SNAP_LOG_SEND;
                    return false;
                    // LCOV_EXCL_STOP
                }
            }

            attachment_offsets[idx] = identifier | JOURNAL_IS_EXTERNAL_ATTACHMENT;
        }
        else
        {
            attachment_offsets[idx] = f_size;
            f_size += data.size();
        }
    }

    event_journal_event_t event_header = {};
    event_header.f_magic[0] = 'e';
    event_header.f_magic[1] = 'v';
    event_header.f_status = static_cast<event_journal_event_t::file_status_t>(f_status);
    event_header.f_request_id_size = f_request_id.length();
    event_header.f_size = f_size;
    event_header.f_time[0] = f_event_time.tv_sec;
    event_header.f_time[1] = f_event_time.tv_nsec;
    event_header.f_attachment_count = number_of_attachments;
    //event_header.f_pad = {}; -- this is not valid, instead I initialize to zero above

    f_file->seekp(f_offset);
    f_file->write(reinterpret_cast<char const *>(&event_header), sizeof(event_header));
    f_file->write(attachment_offsets.data(), attachment_offsets.size() * sizeof(decltype(attachment_offsets)::value_type));
    f_file->write(f_request_id.data(), f_request_id.length());

    // write inline attachments
    //
    for(std::size_t idx(0); idx < number_of_attachments; ++idx)
    {
        attachment const & a(event.get_attachment(idx));
        if(a.size() < attachment_size_threshold)
        {
            if(a.is_file())
            {
                // small files are copied inside the journal file directly
                //
                std::ifstream in(a.filename());
                data_t data(a.size());
                in.read(reinterpret_cast<char *>(data.data()), data.size());
                if(in.fail())
                {
                    SNAP_LOG_FATAL
                        << "failed write_new_event() while reading file \""
                        << a.filename()
                        << "\"."
                        << SNAP_LOG_SEND;
                    return false;
                }
                f_file->write(reinterpret_cast<char const *>(data.data()), data.size());
            }
            else
            {
                f_file->write(reinterpret_cast<char const *>(a.data()), a.size());
            }
        }
    }

    if(f_file->fail())
    {
        // TODO: a partial write happened we would need to clear the magic
        //       if that was saved properly otherwise a load will think that
        //       was correct...
        //
        // LCOV_EXCL_START
        SNAP_LOG_FATAL
            << "failed write_new_event() while writing."
            << SNAP_LOG_SEND;
        return false;
        // LCOV_EXCL_STOP
    }

    f_file->set_next_append(f_file->tell());
    f_file->increase_event_count();

    f_attachment_count = number_of_attachments;

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


std::string const & journal::get_path() const
{
    return f_path;
}


bool journal::is_valid() const
{
    return f_valid;
}


/** \brief Change the maximum number of files used by this journal.
 *
 * When you first setup your journal, you may want to change the total
 * number of files used by this instance. By default, it is set to 2
 * which may not be sufficient for your specific case.
 *
 * \exception out_of_range
 * If the new maximum is less than JOURNAL_MINIMUM_NUMBER_OF_FILES
 * or larger than JOURNAL_MAXIMUM_NUMBER_OF_FILES then this exception
 * is raised.
 *
 * \exception file_still_in_use
 * You may try to reduce the number of files using this function. However,
 * if any of the last few files still include events, this function raises
 * this error.
 *
 * \todo
 * When shrinking the number of files used, try to move the events still
 * found in the last few files to ealier files if there are any. Only
 * raise the file_still_in_use exception if that fails.
 *
 * \todo
 * When reducing the number of files, the code does not attempt to delete
 * still existing files.
 *
 * \param[in] maximum_number_of_files  The new maximum number of files
 * to use.
 *
 * \return true if the change succeeded (i.e. got saved to the journal
 * configuration file).
 */
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
    if(f_maximum_number_of_files == maximum_number_of_files)
    {
        return true;
    }

    // verify that we can apply this change
    // otherwise throw an error
    //
    for(std::uint32_t idx(maximum_number_of_files); idx < f_maximum_number_of_files; ++idx)
    {
        if(idx >= f_event_files.size())
        {
            break; // LCOV_EXCL_LINE
        }
        file::pointer_t f(get_event_file(idx));
        if(f == nullptr)
        {
            continue;
        }
        for(auto const & l : f_event_locations)
        {
            if(l.second->get_file() == f)
            {
                // if the file is still used by a location, then there is
                // data in there
                //
                throw file_still_in_use("it is not currently possible to reduce the maximum number of files when some of those over the new limit are still in use.");
            }
        }
    }

    f_maximum_number_of_files = maximum_number_of_files;
    f_event_files.resize(f_maximum_number_of_files);

    return save_configuration();
}


bool journal::set_maximum_file_size(std::uint32_t maximum_file_size)
{
    maximum_file_size = std::clamp(
                              maximum_file_size
                            , JOURNAL_MINIMUM_FILE_SIZE
                            , JOURNAL_MAXIMUM_FILE_SIZE);

    if(f_maximum_file_size == maximum_file_size)
    {
        return true;
    }

    f_maximum_file_size = maximum_file_size;
    return save_configuration();
}


bool journal::set_maximum_events(std::uint32_t maximum_events)
{
    maximum_events = std::clamp(
                          maximum_events
                        , JOURNAL_MINIMUM_EVENTS
                        , JOURNAL_MAXIMUM_EVENTS);

    if(f_maximum_events == maximum_events)
    {
        return true;
    }

    f_maximum_events = maximum_events;
    return save_configuration();
}


std::uint32_t journal::get_inline_attachment_size_threshold() const
{
    return f_inline_attachment_size_threshold;
}


bool journal::set_inline_attachment_size_threshold(std::uint32_t inline_attachment_size_threshold)
{
    inline_attachment_size_threshold = std::clamp(
                                      inline_attachment_size_threshold
                                    , JOURNAL_INLINE_ATTACHMENT_SIZE_MINIMUM_THRESHOLD
                                    , JOURNAL_INLINE_ATTACHMENT_SIZE_MAXIMUM_THRESHOLD);

    if(inline_attachment_size_threshold == f_inline_attachment_size_threshold)
    {
        return true;
    }

    f_inline_attachment_size_threshold = inline_attachment_size_threshold;
    return save_configuration();
}


bool journal::set_sync(sync_t sync)
{
    if(f_sync == sync)
    {
        return true;
    }

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
    if(f_file_management == file_management)
    {
        return true;
    }

    f_file_management = file_management;
    return save_configuration();
}


bool journal::set_compress_when_full(bool compress_when_full)
{
    if(f_compress_when_full == compress_when_full)
    {
        return true;
    }

    f_compress_when_full = compress_when_full;
    return save_configuration();
}


attachment_copy_handling_t journal::get_attachment_copy_handling() const
{
    return f_attachment_copy_handling;
}


bool journal::set_attachment_copy_handling(attachment_copy_handling_t attachment_copy_handling)
{
    switch(attachment_copy_handling)
    {
    case attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_DEFAULT:
        attachment_copy_handling = attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK;
        break;

    case attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK:
    case attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_HARDLINK:
    case attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_REFLINK:
    case attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_FULL:
        break;

    default:
        std::stringstream ss;
        ss << "unknown attachment_copy_handling_t enumeration type ("
           << static_cast<int>(attachment_copy_handling)
           << ").";
        SNAP_LOG_ERROR << ss.str() << SNAP_LOG_SEND;
        throw invalid_parameter(ss.str());

    }

    if(f_attachment_copy_handling == attachment_copy_handling)
    {
        return true;
    }

    f_attachment_copy_handling = attachment_copy_handling;
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
 * \param[in,out] event_time  The time when the event occurred. May be
 * updated by this function if an ealier event already used this exact time.
 *
 * \return true if the event was added to the journal.
 */
bool journal::add_event(
    in_event const & event,
    snapdev::timespec_ex & event_time)
{
    if(f_event_locations.contains(event.get_request_id()))
    {
        SNAP_LOG_FATAL
            << "request_id already exists in the list of events, it cannot be re-added."
            << SNAP_LOG_SEND;
        return false;
    }
    if(event_time.is_in_the_future(g_time_epsilon))
    {
        SNAP_LOG_FATAL
            << "trying to add an event created in the future: "
            << event_time.to_string("%Y/%m/%d %H:%M:%S.%N")
            << '.'
            << SNAP_LOG_SEND;
        return false;
    }

    if(event.get_request_id().empty()
    || event.get_request_id().length() > 255)
    {
        SNAP_LOG_FATAL
            << "request_id must be between 1 and 255 characters."
            << SNAP_LOG_SEND;
        return false;
    }

    std::size_t const attachment_size(event.get_attachment_size());

    std::size_t event_size(sizeof(event_journal_event_t));
    event_size += attachment_size * sizeof(attachment_offsets_t);
    event_size += event.get_request_id().length();

    for(std::size_t idx(0); idx < attachment_size; ++idx)
    {
        attachment const & a(event.get_attachment(idx));
        if(a.size() < f_inline_attachment_size_threshold)
        {
            event_size += a.size();
        }
        //else -- this will be in a separate file
    }

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

                if(!l->write_new_event(event))
                {
                    return false;
                }

                sync_if_requested(f);

                f_event_locations[event.get_request_id()] = l;
                f_timebased_replay[event_time] = l;
                return true;
            }

            // event too large for this file, try the next file
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


/** \brief Read the next event.
 *
 * This function reads the next event and saves the data and metadata
 * available to the \p event parameter.
 *
 * The \p by_time flag is used to know whether you'd like to load the
 * the next event by time (true) or by identifier (false). The default
 * is to return the data by identifier.
 *
 * The \p debug flag can be used for debug purposes. In that case, the
 * \p event debug fields get set. This includes the name of the file
 * and the position of the data in the file (offset).
 *
 * \note
 * For speed, the \p event structure is not cleared on a call to the
 * next_event() function. If the \p debug flag is set to false (default),
 * then those fields remain the same (i.e. the same value as they were
 * on entry). So if you set it to true once and false afterward, the
 * debug data comes from that call when you once set the flag to true.
 *
 * \param[in,out] event  The object where the event data and metadata is saved.
 * \param[in] by_time  Whether to search for the next event by time or by
 * identifier.
 * \param[in] debug  Whether to gather the debug parameters.
 *
 * \return true if an event is available and saved in the \p event parameter.
 */
bool journal::next_event(out_event & event, bool by_time, bool debug)
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
            // LCOV_EXCL_START
            SNAP_LOG_WARNING
                << "unknown sync type \""
                << sync
                << "\"."
                << SNAP_LOG_SEND;
            // LCOV_EXCL_STOP
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
            // LCOV_EXCL_START
            SNAP_LOG_WARNING
                << "unknown file management type \""
                << file_management
                << "\"."
                << SNAP_LOG_SEND;
            // LCOV_EXCL_STOP
        }
    }

    if(config->has_parameter("maximum_number_of_files"))
    {
        std::string const maximum_number_of_files(config->get_parameter("maximum_number_of_files"));
        std::int64_t max(0);
        advgetopt::validator_integer::convert_string(maximum_number_of_files, max);
        f_maximum_number_of_files = std::clamp(
                                          static_cast<std::uint32_t>(max)
                                        , JOURNAL_MINIMUM_NUMBER_OF_FILES
                                        , JOURNAL_MAXIMUM_NUMBER_OF_FILES);
    }
    f_event_files.resize(f_maximum_number_of_files);

    if(config->has_parameter("maximum_file_size"))
    {
        std::string const maximum_file_size(config->get_parameter("maximum_file_size"));
        std::int64_t max(0);
        advgetopt::validator_integer::convert_string(maximum_file_size, max);
        f_maximum_file_size = std::clamp(static_cast<std::uint32_t>(max), JOURNAL_MINIMUM_FILE_SIZE, JOURNAL_MAXIMUM_FILE_SIZE);
    }

    if(config->has_parameter("maximum_events"))
    {
        std::string const maximum_events(config->get_parameter("maximum_events"));
        std::int64_t max(0);
        advgetopt::validator_integer::convert_string(maximum_events, max);
        f_maximum_events = std::clamp(static_cast<std::uint32_t>(max), JOURNAL_MINIMUM_EVENTS, JOURNAL_MAXIMUM_EVENTS);
    }

    if(config->has_parameter("inline_attachment_size_threshold"))
    {
        std::string const inline_attachment_size(config->get_parameter("inline_attachment_size_threshold"));
        std::int64_t max(0);
        advgetopt::validator_integer::convert_string(inline_attachment_size, max);
        f_inline_attachment_size_threshold = std::clamp(static_cast<std::uint32_t>(max), JOURNAL_INLINE_ATTACHMENT_SIZE_MINIMUM_THRESHOLD, JOURNAL_INLINE_ATTACHMENT_SIZE_MAXIMUM_THRESHOLD);
    }

    if(config->has_parameter("attachment_copy_handling"))
    {
        std::string const attachment_copy_handling(config->get_parameter("attachment_copy_handling"));
        if(attachment_copy_handling == "default"
        || attachment_copy_handling == "softlink")
        {
            f_attachment_copy_handling = attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK;
        }
        else if(attachment_copy_handling == "hardlink")
        {
            f_attachment_copy_handling = attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_HARDLINK;
        }
        else if(attachment_copy_handling == "reflink")
        {
            f_attachment_copy_handling = attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_REFLINK;
        }
        else if(attachment_copy_handling == "full")
        {
            f_attachment_copy_handling = attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_FULL;
        }
        else
        {
            // LCOV_EXCL_START
            SNAP_LOG_WARNING
                << "unknown attachment copy handling type \""
                << attachment_copy_handling
                << "\"."
                << SNAP_LOG_SEND;
            // LCOV_EXCL_STOP
        }
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
    case sync_t::SYNC_FLUSH:
        sync = "flush";
        break;

    case sync_t::SYNC_FULL:
        sync = "full";
        break;

    //case sync_t::SYNC_NONE:
    default:
        sync = "none";
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

    config->set_parameter(
        std::string(),
        "inline_attachment_size_threshold",
        std::to_string(static_cast<int>(f_inline_attachment_size_threshold)));

    std::string attachment_copy_handling;
    switch(f_attachment_copy_handling)
    {
    case attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_HARDLINK:
        attachment_copy_handling = "hardlink";
        break;

    case attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_REFLINK:
        attachment_copy_handling = "reflink";
        break;

    case attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_FULL:
        attachment_copy_handling = "full";
        break;

    // the default is softlink
    //case attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_DEFAULT:
    //case attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK:
    default:
        attachment_copy_handling = "softlink";
        break;

    }
    config->set_parameter(
        std::string(),
        "attachment_copy_handling",
        attachment_copy_handling);

    config->save_configuration(".bak", true);

    return true;
}


bool journal::load_event_locations(bool compress)
{
    std::vector<char> compress_buffer;
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
            SNAP_LOG_MAJOR
                << "found event file with invalid magic and/or version ("
                << ascii(journal_header.f_magic[0])
                << ascii(journal_header.f_magic[1])
                << ascii(journal_header.f_magic[2])
                << ascii(journal_header.f_magic[3])
                << ") version "
                << static_cast<int>(journal_header.f_major_version)
                << '.'
                << static_cast<int>(journal_header.f_minor_version)
                << " in \""
                << f->filename()
                << '"'
                << SNAP_LOG_SEND;
            continue;
        }

        bool found_compress_offset(false);
        bool good(true);
        while(good)
        {
            std::ios::pos_type const offset(f->tellg());
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

            // LCOV_EXCL_START
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
            // LCOV_EXCL_STOP

            }
            ssize_t const data_size(event_header.f_size
                                        - sizeof(event_header)
                                        - event_header.f_attachment_count * sizeof(attachment_offsets_t)
                                        - event_header.f_request_id_size);
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
                    << "\"."
                    << SNAP_LOG_SEND;
                break;
            }

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

            // skip the attachment offsets, we don't need them at the moment
            // unless we are compressing then we'll need to copy them so keep
            // the offset
            //
            off_t const attachment_offsets(f->tellg());
            f->seekg(event_header.f_attachment_count * sizeof(attachment_offsets_t), std::ios::cur);

            std::string request_id(event_header.f_request_id_size, ' ');
            f->read(request_id.data(), event_header.f_request_id_size);
            if(!f->good())
            {
                // LCOV_EXCL_START
                SNAP_LOG_FATAL
                    << "could not read request identifier at "
                    << offset
                    << " in \""
                    << get_filename(index)
                    << '"'
                    << SNAP_LOG_SEND;
                break;
                // LCOV_EXCL_STOP
            }

            location::pointer_t l(std::make_shared<location>(f));
            l->set_request_id(request_id);
            l->set_event_time(event_time);
            l->set_status(static_cast<status_t>(event_header.f_status));
            l->set_file_index(index);
            l->set_attachment_count(event_header.f_attachment_count);
            l->set_offset(offset);
            l->set_size(event_header.f_size); // full size, allows us to compute the size of the last attachment

            f_event_locations[request_id] = l;
            f_timebased_replay[event_time] = l;
            f->increase_event_count();

            if(found_compress_offset && compress)
            {
                // we are in compression mode and this event can be moved
                // "up" (lower offset), do so

                if(compress_buffer.size() == 0)
                {
                    // copy up to 64Kb at a time
                    //
                    constexpr std::size_t const COMPRESS_BUFFER_SIZE(64 * 1024);
                    static_assert(COMPRESS_BUFFER_SIZE >= MAXIMUM_ATTACHMENT_COUNT * sizeof(attachment_offsets_t));
                    compress_buffer.resize(COMPRESS_BUFFER_SIZE);
                }

                // read the attachment offsets in this case, we also need
                // to move them
                //
                off_t const current_offset(f->tellg());
                f->seekg(attachment_offsets);
                std::size_t const offset_size(event_header.f_attachment_count * sizeof(attachment_offsets_t));
                f->read(compress_buffer.data(), offset_size);
                f->seekg(current_offset);

                // save the header
                //
                l->set_offset(f->tellp());
                f->write(reinterpret_cast<char const *>(&event_header), sizeof(event_header));

                // save the attachment offsets
                //
                f->write(compress_buffer.data(), offset_size);

                // save the request id string
                //
                f->write(request_id.data(), request_id.length());

                // now copy the data, one block at a time
                //
                std::size_t remaining_size(data_size);
                while(remaining_size > 0)
                {
                    std::size_t const s(std::min(remaining_size, compress_buffer.size()));

                    // read
                    //
                    f->read(compress_buffer.data(), s);
                    if(f->fail())
                    {
                        // TODO: handle the error better (i.e. mark event
                        //       as invalid)
                        break; // LCOV_EXCL_LINE
                    }

                    // write
                    //
                    f->write(compress_buffer.data(), s);

                    remaining_size -= s;
                }

                f->set_next_append(f->tellp());
            }
            else
            {
                // skip the data, we don't need it for our index
                //
                f->seekg(data_size, std::ios::cur);

                f->set_next_append(f->tellg());
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
        // LCOV_EXCL_START
        std::stringstream ss;
        ss << "index too large in get_event_file() ("
           << static_cast<int>(index)
           << " > "
           << f_maximum_number_of_files
           << ").";
        SNAP_LOG_ERROR << ss.str() << SNAP_LOG_SEND;
        throw invalid_parameter(ss.str());
        // LCOV_EXCL_STOP
    } // LCOV_EXCL_LINE

    file::pointer_t f(f_event_files[index].lock());
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
} // LCOV_EXCL_LINE


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

    default: // LCOV_EXCL_LINE
        throw invalid_parameter("the status in `update_event_status` is not valid"); // LCOV_EXCL_LINE

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
    } // LCOV_EXCL_LINE

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
                // LCOV_EXCL_START
                SNAP_LOG_ERROR
                    << "could not find event with time "
                    << it->second->get_event_time()
                    << " while updating event status."
                    << SNAP_LOG_SEND;
                // LCOV_EXCL_STOP
            }
            else
            {
                f_timebased_replay.erase(time_it);
            }
        }
        f_event_locations.erase(it);
        f->decrease_event_count();

        if(f_event_locations.empty())
        {
            // this allows for the file to:
            // . be reused (keep)
            // . shrink (truncate)
            // . be deleted (delete)
            //
            f->set_next_append(sizeof(event_journal_header_t));
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
    snapdev::NOT_REACHED(); // LCOV_EXCL_LINE
}



} // namespace prinbee
// vim: ts=4 sw=4 et
