// Copyright (c) 2023-2025  Made to Order Software Corp.  All Rights Reserved
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
 * A journal is a basic system that saves events in a journal (a file)
 * before forwarding them to the next systems. This is used to make sure
 * that the data does not get lost. If the next system does not acknowledge
 * receiving an event, the journal keeps it in its file until it does.
 *
 * The implementation makes use of two event classes: in_event and out_event.
 * The data is passed as event attachments. Each event is given a request
 * identifier, a status, and a timestamp.
 *
 * Additional information can be found in the implementation file (.cpp).
 */

// self
//
#include    <prinbee/exception.h>


// snapdev
//
#include    <snapdev/timespec_ex.h>


// C++
//
#include    <fstream>
#include    <memory>



namespace prinbee
{



constexpr std::uint32_t const       JOURNAL_DEFAULT_NUMBER_OF_FILES = 2;
constexpr std::uint32_t const       JOURNAL_MINIMUM_NUMBER_OF_FILES = 2;
constexpr std::uint32_t const       JOURNAL_MAXIMUM_NUMBER_OF_FILES = 255;

constexpr std::uint32_t const       JOURNAL_DEFAULT_FILE_SIZE = 1024 * 1024;
constexpr std::uint32_t const       JOURNAL_MINIMUM_FILE_SIZE = 64 * 1024;
constexpr std::uint32_t const       JOURNAL_MAXIMUM_FILE_SIZE = 128 * 1024 * 1024;

constexpr std::uint32_t const       JOURNAL_DEFAULT_EVENTS = 4096;
constexpr std::uint32_t const       JOURNAL_MINIMUM_EVENTS = 100;
constexpr std::uint32_t const       JOURNAL_MAXIMUM_EVENTS = 100'000;

constexpr std::uint32_t const       JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD = 4 * 1024;
constexpr std::uint32_t const       JOURNAL_INLINE_ATTACHMENT_SIZE_MINIMUM_THRESHOLD = 256;
constexpr std::uint32_t const       JOURNAL_INLINE_ATTACHMENT_SIZE_MAXIMUM_THRESHOLD = 16 * 1024;

typedef std::uint32_t               attachment_offsets_t;
constexpr std::uint32_t const       JOURNAL_IS_EXTERNAL_ATTACHMENT = 1UL << (sizeof(attachment_offsets_t) * CHAR_BIT - 1);

constexpr int const                 JOURNAL_ATTACHMENT_COUNTER_INDEX = 0;
constexpr std::size_t const         MAXIMUM_ATTACHMENT_COUNT = 255;


// enum uses specific numbers because these get saved in a file
// so it cannot change over time; just "cancel" old numbers
// and use new ones as required
//
enum class status_t : std::uint8_t
{
    STATUS_UNKNOWN = 0,       // equivalent to a "null"

    STATUS_READY = 1,
    STATUS_FORWARDED = 2,
    STATUS_ACKNOWLEDGED = 3,
    STATUS_COMPLETED = 4,

    STATUS_FAILED = 100,    // TBD: maybe have a clearer reason for failure since we have another ~150 numbers available?
};


enum class sync_t : std::uint8_t
{
    SYNC_NONE,           // no flushing or sync
    SYNC_FLUSH,          // just standard flush() -- flush rdbuf
    SYNC_FULL,           // fsync()
};


enum class file_management_t : std::uint8_t
{
    FILE_MANAGEMENT_KEEP,
    FILE_MANAGEMENT_TRUNCATE,
    FILE_MANAGEMENT_DELETE,
};


enum class attachment_copy_handling_t : std::uint8_t
{
    ATTACHMENT_COPY_HANDLING_DEFAULT,
    ATTACHMENT_COPY_HANDLING_SOFTLINK,
    ATTACHMENT_COPY_HANDLING_HARDLINK,
    ATTACHMENT_COPY_HANDLING_REFLINK,
    ATTACHMENT_COPY_HANDLING_FULL,
};


typedef std::string                 request_id_t;
typedef std::uint8_t                attachment_id_t;
typedef std::vector<std::uint8_t>   data_t;


class attachment
{
public:
    typedef std::vector<attachment>     vector_t;

    //                            attachment(attachment &) = default;
    //attachment                  operator = (attachment &) = default;

    void                        clear();
    void                        set_data(void * data, off_t sz);
    void                        save_data(void * data, off_t sz);
    void                        save_data(data_t const & data);
    void                        set_file(std::string const & filename, off_t sz = 0);

    off_t                       size() const;
    void *                      data() const;
    std::string const &         filename() const;
    bool                        load_file_data();
    bool                        empty() const;
    bool                        is_file() const;

private:
    off_t                       f_size = 0;
    void *                      f_data = nullptr;
    std::shared_ptr<data_t>     f_saved_data = std::shared_ptr<data_t>(); // used when caller is not going to hold the data (i.e. a copy is required)
    std::string                 f_filename = std::string();
};


class in_event
{
public:
    void                        set_request_id(request_id_t const & request_id);
    request_id_t const &        get_request_id() const;

    attachment_id_t             add_attachment(attachment const & a);
    std::size_t                 get_attachment_size() const;
    attachment const &          get_attachment(attachment_id_t id) const;

private:
    request_id_t                f_request_id = request_id_t();
    attachment::vector_t        f_attachments = attachment::vector_t();
};


class out_event
{
public:
    void                        set_request_id(request_id_t const & request_id);
    request_id_t const &        get_request_id() const;

    void                        set_status(status_t status);
    status_t                    get_status() const;

    void                        set_event_time(snapdev::timespec_ex const & event_time);
    snapdev::timespec_ex const &get_event_time() const;

    attachment_id_t             add_attachment(attachment const & a);
    std::size_t                 get_attachment_size() const;
    attachment const &          get_attachment(attachment_id_t id) const;

    void                        set_debug_filename(std::string debug_filename);
    std::string                 get_debug_filename() const;

    void                        set_debug_offset(std::uint32_t debug_offset);
    std::uint32_t               get_debug_offset() const;

private:
    request_id_t                f_request_id = request_id_t();
    status_t                    f_status = status_t::STATUS_UNKNOWN;
    snapdev::timespec_ex        f_event_time = snapdev::timespec_ex();
    attachment::vector_t        f_attachments = attachment::vector_t();

    // if the `debug` flag is set to true, these will also be set
    //
    std::string                 f_debug_filename = std::string();
    std::uint32_t               f_debug_offset = 0;
};


/** \brief Convert an identifier represented by a number in a string.
 *
 * This helper function can be used to convert an identifier in a
 * string. This is useful if you'd like to use a number as your
 * identifiers.
 *
 * The transformation uses the number in bigendian order so that way
 * they are sorted as expected (smallest to largest assuming all numbers
 * are positive).
 *
 * Here is an example of usage:
 *
 * \code
 *     prinbee::in_event event;
 *     event.set_request_id(prinbee::id_to_string(id)),
 *     prinbee::data_t buffer =
 *     {
 *         .f_size = data.size(),
 *         .f_data = data.data(),
 *     };
 *     event.set_data(buffer);
 *     prinbee::data_t image_buffer =
 *     {
 *         .f_size = image_size,
 *         .f_data = image_data.data(),
 *     };
 *     event.add_attachment(image_buffer);
 *     snapdev::timespec_ex event_time(snapdev::now());
 *     journal.add_event(event, event_time);
 * \endcode
 *
 * \note
 * This function does not convert the number in ASCII digits in a string.
 * It actually creates a string with the binary data.
 *
 * \note
 * If you are going to re-use the same identifier many times, avoid calling
 * the function over and over again. It should be considered slow. Saving
 * the result in a variable is best.
 *
 * \tparam T  The type of integer.
 * \param[in] id  The identifier to transform in a string.
 *
 * \return The `id` number in an std::string.
 */
template<
      typename T
    , std::enable_if_t<std::is_integral_v<T>, int> = 0>
inline std::string id_to_string(T id)
{
    std::string result(sizeof(id), '\0');
    for(int idx(sizeof(id) - 1); id != 0; --idx)
    {
        result[idx] = id & 255;
        id >>= 8;
    }
    return result;
} // LCOV_EXCL_LINE


template<
      typename T
    , std::enable_if_t<std::is_integral_v<T>, int> = 0>
inline T string_to_id(T & id, std::string const & value)
{
    if(value.length() != sizeof(T))
    {
        throw invalid_parameter("input string is not the right size");
    }
    id = 0;
    for(std::size_t idx(0); idx < sizeof(id); ++idx)
    {
        id = (id << 8) | static_cast<std::uint8_t>(value[idx]);
    }
    return id;
}


class journal
{
public:
    typedef std::shared_ptr<journal>
                                pointer_t;

                                journal(std::string const & path);

    std::string const &         get_path() const;
    bool                        is_valid() const;
    sync_t                      sync();

    // options
    //
    bool                        set_maximum_number_of_files(std::uint32_t maximum_number_of_files);
    bool                        set_maximum_file_size(std::uint32_t maximum_file_size);
    bool                        set_maximum_events(std::uint32_t maximum_events);
    std::uint32_t               get_inline_attachment_size_threshold() const;
    bool                        set_inline_attachment_size_threshold(std::uint32_t inline_attachment_size_threshold);
    bool                        set_sync(sync_t sync);
    file_management_t           get_file_management() const;
    bool                        set_file_management(file_management_t file_management);
    bool                        set_compress_when_full(bool compress_when_full);
    attachment_copy_handling_t  get_attachment_copy_handling() const;
    bool                        set_attachment_copy_handling(attachment_copy_handling_t attachment_copy_handling);

    // events status
    //
    bool                        add_event(
                                    in_event const & event,
                                    snapdev::timespec_ex & event_time);
    bool                        event_forwarded(request_id_t const & request_id);
    bool                        event_acknowledged(request_id_t const & request_id);
    bool                        event_completed(request_id_t const & request_id);
    bool                        event_failed(request_id_t const & request_id);

    // events replay
    //
    bool                        empty() const;
    std::size_t                 size() const;
    void                        rewind();
    bool                        next_event(
                                    out_event & event,
                                    bool by_time = true,
                                    bool debug = false);

private:
    class file
    {
    public:
        typedef std::shared_ptr<file>
                                    pointer_t;
        typedef std::weak_ptr<file> weak_pointer_t;
        typedef std::vector<weak_pointer_t>
                                    vector_t;

                                    file(
                                          journal * j
                                        , std::string const & filename
                                        , bool create);
                                    file(file const &) = delete;
                                    ~file();
        file &                      operator = (file const &) = delete;

        std::string const &         get_path() const;
        std::string const &         filename() const;
        bool                        good() const;
        bool                        fail() const;
        void                        clear();
        void                        seekg(std::ios::pos_type offset, std::ios::seekdir dir = std::ios::beg);
        void                        seekp(std::ios::pos_type offset, std::ios::seekdir dir = std::ios::beg);
        std::ios::pos_type          tellg() const;
        std::ios::pos_type          tellp() const;
        std::ios::pos_type          tell() const;
        std::ios::pos_type          size() const;
        void                        read(void * data, std::size_t size);
        void                        write(void const * data, std::size_t size);
        void                        truncate();
        void                        flush();
        void                        fsync();
        void                        reset_event_count();
        void                        increase_event_count();
        void                        decrease_event_count();
        std::uint32_t               get_event_count() const;
        void                        set_next_append(std::uint32_t offset);
        std::uint32_t               get_next_append() const;
        std::uint32_t               get_inline_attachment_size_threshold() const;
        attachment_copy_handling_t  get_attachment_copy_handling() const;

    private:
        std::string                 f_filename = std::string();
        journal *                   f_journal = nullptr;
        std::shared_ptr<std::fstream>
                                    f_event_file = std::shared_ptr<std::fstream>();
        std::ios::pos_type          f_pos_read = 0;
        std::ios::pos_type          f_pos_write = 0;
        std::uint32_t               f_event_count = 0;
        std::uint32_t               f_next_append = 0;
    };

    class location
    {
    public:
        typedef std::shared_ptr<location>
                                    pointer_t;
        typedef std::map<request_id_t, pointer_t>
                                    request_id_map_t;
        typedef std::map<snapdev::timespec_ex, pointer_t>
                                    time_map_t;

                                    location(file::pointer_t f);

        file::pointer_t             get_file() const;

        void                        set_request_id(std::string const & request_id);
        snapdev::timespec_ex        get_event_time() const;
        void                        set_event_time(snapdev::timespec_ex const & event_time);
        status_t                    get_status() const;
        void                        set_status(status_t status);
        void                        set_file_index(std::uint8_t file_index);
        void                        set_attachment_count(std::uint32_t count);
        std::uint32_t               get_offset() const;
        void                        set_offset(std::uint32_t offset);
        void                        set_size(std::uint32_t size);

        bool                        read_data(out_event & event, bool debug);
        bool                        write_new_event(in_event const & event);

    private:
        file::pointer_t             f_file = file::pointer_t();

        request_id_t                f_request_id = request_id_t();
        snapdev::timespec_ex        f_event_time = snapdev::timespec_ex();
        status_t                    f_status = status_t::STATUS_UNKNOWN;
        std::uint8_t                f_file_index = 0;
        std::uint8_t                f_attachment_count = 0;
        std::uint32_t               f_offset = 0;
        std::uint32_t               f_size = 0;
    };

    std::string                 get_configuration_filename() const;
    bool                        load_configuration();
    bool                        save_configuration();
    bool                        load_event_locations(bool compress = false);
    bool                        append_new_event();
    bool                        update_event_status(request_id_t const & request_id, status_t const status);
    file::pointer_t             get_event_file(std::uint8_t index, bool create = false);
    std::string                 get_filename(std::uint8_t index);
    void                        sync_if_requested(file::pointer_t file);

    std::string                 f_path = std::string();
    bool                        f_valid = false;
    bool                        f_can_be_compressed = false;

    // options (from .conf file)
    //
    sync_t                      f_sync = sync_t::SYNC_NONE;
    bool                        f_compress_when_full = false;
    file_management_t           f_file_management = file_management_t::FILE_MANAGEMENT_KEEP;
    std::uint8_t                f_maximum_number_of_files = JOURNAL_DEFAULT_NUMBER_OF_FILES;
    std::uint32_t               f_maximum_file_size = JOURNAL_DEFAULT_FILE_SIZE;
    std::uint32_t               f_maximum_events = JOURNAL_DEFAULT_EVENTS;
    std::uint32_t               f_inline_attachment_size_threshold = JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD;
    attachment_copy_handling_t  f_attachment_copy_handling = attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK;

    // the actual journal data
    //
    std::uint8_t                f_current_file_index = 0;
    file::vector_t              f_event_files = file::vector_t();
    location::request_id_map_t  f_event_locations = location::request_id_map_t();
    location::time_map_t        f_timebased_replay = location::time_map_t();
    location::request_id_map_t::iterator 
                                f_event_locations_iterator = location::request_id_map_t::iterator();
    location::time_map_t::iterator
                                f_timebased_replay_iterator = location::time_map_t::iterator();
};



} // namespace prinbee
// vim: ts=4 sw=4 et
