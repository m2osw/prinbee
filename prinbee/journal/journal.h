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


typedef std::string     request_id_t;


struct in_event_t
{
    request_id_t                f_request_id = std::string();
    std::size_t                 f_size = 0;
    void *                      f_data = nullptr;
};


struct out_event_t
{
    request_id_t                f_request_id = std::string();
    status_t                    f_status = status_t::STATUS_UNKNOWN;
    snapdev::timespec_ex        f_event_time = snapdev::timespec_ex();
    std::vector<std::uint8_t>   f_data = std::vector<std::uint8_t>();

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
 *     prinbee::in_event_t const event =
 *     {
 *         .f_request_id = prinbee::id_to_string(id),
 *         .f_size = size,
 *         .f_data = data.data(),
 *     };
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

    bool                        is_valid() const;
    sync_t                      sync();

    // options
    //
    bool                        set_maximum_number_of_files(std::uint32_t maximum_number_of_files);
    bool                        set_maximum_file_size(std::uint32_t maximum_file_size);
    bool                        set_maximum_events(std::uint32_t maximum_events);
    bool                        set_sync(sync_t sync);
    file_management_t           get_file_management() const;
    bool                        set_file_management(file_management_t file_management);
    bool                        set_compress_when_full(bool compress_when_full);

    // events status
    //
    bool                        add_event(
                                    in_event_t const & event,
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
    bool                        next_event(out_event_t & event, bool by_time = true, bool debug = false);

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
        std::uint32_t               get_offset() const;
        void                        set_offset(std::uint32_t offset);
        void                        set_size(std::uint32_t size);

        bool                        read_data(out_event_t & event, bool debug);
        bool                        write_new_event(in_event_t const & event);

    private:
        file::pointer_t             f_file = file::pointer_t();

        request_id_t                f_request_id = request_id_t();
        snapdev::timespec_ex        f_event_time = snapdev::timespec_ex();
        status_t                    f_status = status_t::STATUS_UNKNOWN;
        std::uint8_t                f_file_index = 0;
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
