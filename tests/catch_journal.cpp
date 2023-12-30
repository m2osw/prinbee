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

// self
//
#include    "catch_main.h"


// prinbee
//
#include    <prinbee/exception.h>
#include    <prinbee/journal/journal.h>


// snapdev
//
#include    <snapdev/file_contents.h>
#include    <snapdev/mkdir_p.h>


// C++
//
#include    <random>


// advgetopt
//
#include    <advgetopt/conf_file.h>



namespace
{



std::string conf_filename(std::string const & path)
{
    return path + "/journal.conf";
}


std::string event_filename(std::string const & path, int index)
{
    return path + "/journal-" + std::to_string(index) + ".events";
}


void unlink_conf(std::string const & path)
{
    std::string const filename(conf_filename(path));
    if(unlink(filename.c_str()) != 0)
    {
        if(errno != ENOENT)
        {
            perror("unlink() returned unexpected error.");
            CATCH_REQUIRE(!"unlink() returned an unexpected error");
        }
    }
}


void unlink_events(std::string const & path)
{
    for(int idx(0);; ++idx)
    {
        std::string const filename(event_filename(path, idx));
        if(unlink(filename.c_str()) != 0)
        {
            if(errno != ENOENT)
            {
                perror("unlink() returned unexpected error.");
                CATCH_REQUIRE(!"unlink() returned an unexpected error");
            }
            break;
        }
    }
}


std::string conf_path(std::string const & sub_path, bool create_directory = false)
{
    std::string const path(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + '/' + sub_path);
    if(create_directory)
    {
        CATCH_REQUIRE(snapdev::mkdir_p(path) == 0);
    }
    unlink_conf(path);
    unlink_events(path);
    return path;
}


typedef std::map<std::string, std::string> conf_values_t;

conf_values_t load_conf(std::string const & path)
{
    conf_values_t conf_values;
    snapdev::file_contents file(conf_filename(path));
    CATCH_REQUIRE(file.read_all());
    std::string const contents(file.contents());
    std::list<std::string> lines;
    snapdev::tokenize_string(lines, contents, "\r\n", true);
    for(auto const & l : lines)
    {
        if(l.empty()
        || l[0] == '#')
        {
            continue;
        }
        std::string::size_type const pos(l.find('='));
        CATCH_REQUIRE(std::string::npos != pos);
        std::string const name(l.substr(0, pos));
        std::string const value(l.substr(pos + 1));
        conf_values.emplace(name, value);
    }
    return conf_values;
}


}


CATCH_TEST_CASE("journal_helper_functions", "[journal]")
{
    CATCH_START_SECTION("journal_helper_functions: id_to_string()")
    {
        std::uint32_t const id((0x31 << 24) | (0x32 << 16) | (0x33 << 8) | 0x34);
        CATCH_REQUIRE(prinbee::id_to_string(id) == "1234");
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("journal_options", "[journal]")
{
    CATCH_START_SECTION("journal_options: default options")
    {
        enum
        {
            COMPRESS_WHEN_FULL,
            FILE_MANAGEMENT,
            MAXIMUM_EVENTS,
            MAXIMUM_FILE_SIZE,
            MAXIMUM_NUMBER_OF_FILES,
            FLUSH,
            SYNC,

            max_options
        };
        for(int index(0); index < max_options; ++index)
        {
            std::string expected_result;
            std::string const path(conf_path("journal_options"));
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());
            switch(index)
            {
            case COMPRESS_WHEN_FULL:
                CATCH_REQUIRE(j.set_compress_when_full(true));
                break;

            case FILE_MANAGEMENT:
                {
                    prinbee::file_management_t const value(static_cast<prinbee::file_management_t>(rand() % 3));
                    CATCH_REQUIRE(j.set_file_management(value));
                    switch(value)
                    {
                    case prinbee::file_management_t::FILE_MANAGEMENT_KEEP:
                        expected_result = "keep";
                        break;

                    case prinbee::file_management_t::FILE_MANAGEMENT_TRUNCATE:
                        expected_result = "truncate";
                        break;

                    case prinbee::file_management_t::FILE_MANAGEMENT_DELETE:
                        expected_result = "delete";
                        break;

                    }
                }
                break;

            case MAXIMUM_EVENTS:
                {
                    std::uint32_t const value(rand());
                    CATCH_REQUIRE(j.set_maximum_events(value));
                    if(value < prinbee::JOURNAL_MINIMUM_EVENTS)
                    {
                        expected_result = std::to_string(prinbee::JOURNAL_MINIMUM_EVENTS);
                    }
                    else if(value > prinbee::JOURNAL_MAXIMUM_EVENTS)
                    {
                        expected_result = std::to_string(prinbee::JOURNAL_MAXIMUM_EVENTS);
                    }
                    else
                    {
                        expected_result = std::to_string(value);
                    }
                }
                break;

            case MAXIMUM_FILE_SIZE:
                {
                    std::uint32_t const value(rand() + 1);
                    CATCH_REQUIRE(j.set_maximum_file_size(value));
                    if(value < prinbee::JOURNAL_MINIMUM_FILE_SIZE)
                    {
                        expected_result = std::to_string(prinbee::JOURNAL_MINIMUM_FILE_SIZE);
                    }
                    else if(value > prinbee::JOURNAL_MAXIMUM_FILE_SIZE)
                    {
                        expected_result = std::to_string(prinbee::JOURNAL_MAXIMUM_FILE_SIZE);
                    }
                    else
                    {
                        expected_result = std::to_string(value);
                    }
                }
                break;

            case MAXIMUM_NUMBER_OF_FILES:
                {
                    int const value(rand() % (256 - 2) + 2);
                    CATCH_REQUIRE(j.set_maximum_number_of_files(value));
                    expected_result = std::to_string(value);
                }
                break;

            case FLUSH:
                CATCH_REQUIRE(j.set_sync(prinbee::sync_t::SYNC_FLUSH));
                break;

            case SYNC:
                CATCH_REQUIRE(j.set_sync(prinbee::sync_t::SYNC_FULL));
                break;

            }
            conf_values_t conf_values(load_conf(path));

            auto it(conf_values.find("compress_when_full"));
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE((index == COMPRESS_WHEN_FULL ? "true" : "false") == it->second);
            conf_values.erase(it);

            it = conf_values.find("file_management");
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE((index == FILE_MANAGEMENT ? expected_result : "keep") == it->second);
            conf_values.erase(it);

            it = conf_values.find("maximum_events");
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE((index == MAXIMUM_EVENTS ? expected_result : "4096") == it->second);
            conf_values.erase(it);

            it = conf_values.find("maximum_file_size");
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE((index == MAXIMUM_FILE_SIZE ? expected_result : "1048576") == it->second);
            conf_values.erase(it);

            it = conf_values.find("maximum_number_of_files");
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE((index == MAXIMUM_NUMBER_OF_FILES ? expected_result : "2") == it->second);
            conf_values.erase(it);

            it = conf_values.find("sync");
            CATCH_REQUIRE(it != conf_values.end());
            switch(index)
            {
            case FLUSH:
                CATCH_REQUIRE("flush" == it->second);
                break;

            case SYNC:
                CATCH_REQUIRE("full" == it->second);
                break;

            default:
                CATCH_REQUIRE("none" == it->second);
                break;

            }
            conf_values.erase(it);

            CATCH_REQUIRE(conf_values.empty());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: reducing the number of files generates a TODO")
    {
        std::string const path(conf_path("journal_options"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        CATCH_REQUIRE(j.set_maximum_file_size(10));
        CATCH_REQUIRE(j.set_maximum_file_size(5));  // <- here (TODO: add logger output capture to verify what happens)
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: invalid file management numbers")
    {
        std::string const path(conf_path("journal_options"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());

        for(int i(-100); i <= 100; ++i)
        {
            switch(static_cast<prinbee::file_management_t>(i))
            {
            case prinbee::file_management_t::FILE_MANAGEMENT_KEEP:
            case prinbee::file_management_t::FILE_MANAGEMENT_TRUNCATE:
            case prinbee::file_management_t::FILE_MANAGEMENT_DELETE:
                // these are valid, ignore
                break;

            default:
                CATCH_REQUIRE_THROWS_MATCHES(
                          j.set_file_management(static_cast<prinbee::file_management_t>(i))
                        , prinbee::invalid_parameter
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: unsupported file management number"));
                break;

            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: minimum number of events")
    {
        for(std::uint32_t count(0); count <= prinbee::JOURNAL_MINIMUM_EVENTS; ++count)
        {
            std::string const path(conf_path("journal_options"));
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());
            CATCH_REQUIRE(j.set_maximum_events(count));
            conf_values_t conf_values(load_conf(path));

            auto const it(conf_values.find("maximum_events"));
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE(std::to_string(prinbee::JOURNAL_MINIMUM_EVENTS) == it->second);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: maximum number of events")
    {
        for(std::uint32_t count(prinbee::JOURNAL_MAXIMUM_EVENTS); count <= 1'000'000; count += rand() % 2'500 + 1)
        {
            std::string const path(conf_path("journal_options"));
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());
            CATCH_REQUIRE(j.set_maximum_events(count));
            conf_values_t conf_values(load_conf(path));

            auto const it(conf_values.find("maximum_events"));
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE(std::to_string(prinbee::JOURNAL_MAXIMUM_EVENTS) == it->second);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: minimum file size")
    {
        for(std::uint32_t size(0); size <= prinbee::JOURNAL_MINIMUM_FILE_SIZE; ++size)
        {
            std::string const path(conf_path("journal_options"));
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());
            CATCH_REQUIRE(j.set_maximum_file_size(size));
            conf_values_t conf_values(load_conf(path));

            auto const it(conf_values.find("maximum_file_size"));
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE(std::to_string(prinbee::JOURNAL_MINIMUM_FILE_SIZE) == it->second);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: maximum file size")
    {
        for(std::uint32_t size(prinbee::JOURNAL_MAXIMUM_FILE_SIZE); size <= 0x6000'0000; size += rand() % 65536 + 1)
        {
            std::string const path(conf_path("journal_options"));
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());
            CATCH_REQUIRE(j.set_maximum_file_size(size));
            conf_values_t conf_values(load_conf(path));

            auto const it(conf_values.find("maximum_file_size"));
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE(std::to_string(prinbee::JOURNAL_MAXIMUM_FILE_SIZE) == it->second);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("journal_event_status_sequence", "[journal]")
{
    CATCH_START_SECTION("journal_event_status_sequence: all valid/invalid sequences")
    {
        std::vector<prinbee::status_t> next_status[]
        {
            // ready -> ... -> completed
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_COMPLETED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_COMPLETED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_COMPLETED,
            },
            {
                prinbee::status_t::STATUS_COMPLETED,
            },

            // ready -> ... -> fails
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_FAILED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_FAILED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_FAILED,
            },
            {
                prinbee::status_t::STATUS_FAILED,
            },

            // impossible
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FORWARDED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FORWARDED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FORWARDED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FORWARDED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_COMPLETED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FORWARDED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_COMPLETED,
            },
            {
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FORWARDED,
            },
            {
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
            },
            {
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_COMPLETED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FORWARDED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FAILED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FORWARDED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FAILED,
            },
            {
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FORWARDED,
            },
            {
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
            },
            {
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FAILED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_COMPLETED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_COMPLETED,
            },
            {
                prinbee::status_t::STATUS_FAILED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_COMPLETED,
            },
            {
                prinbee::status_t::STATUS_FORWARDED,
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FAILED,
            },
            {
                prinbee::status_t::STATUS_ACKNOWLEDGED,
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FAILED,
            },
            {
                prinbee::status_t::STATUS_COMPLETED,
                prinbee::status_t::STATUS_UNKNOWN,
                prinbee::status_t::STATUS_FAILED,
            },
        };
        int count(0);
        for(auto const & sequence : next_status)
        {
            std::string const path(conf_path("journal_events"));
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());

            ++count;
            std::cerr << "--- running sequence #" << count << "\n";
            std::size_t const size(rand() % 10 * 1024 + 1);
            std::vector<std::uint8_t> data(size);
            for(std::size_t idx(0); idx < size; ++idx)
            {
                data[idx] = static_cast<std::uint8_t>(rand());
            }
            std::string const request_id(SNAP_CATCH2_NAMESPACE::random_string(1, 255));
            prinbee::in_event_t const event =
            {
                .f_request_id = request_id,
                .f_size = size,
                .f_data = data.data(),
            };
            snapdev::timespec_ex const event_time(snapdev::now());
            snapdev::timespec_ex pass_time(event_time);
            CATCH_REQUIRE(j.add_event(event, pass_time));
            CATCH_REQUIRE(event_time == pass_time);

            // the only way to verify that the event was sent to the journal
            // properly is to read it back using the next_event() function, but
            // since we just added a first even, the next_event() won't find
            // it (i.e. that iterator is already pointing to end()), so we'll
            // need a rewind() call first
            //
            prinbee::out_event_t out_event;
            CATCH_REQUIRE_FALSE(j.next_event(out_event));

            j.rewind();
            CATCH_REQUIRE(j.next_event(out_event, true, true));

            std::string const filename(path + "/journal-0.events");
            CATCH_REQUIRE(filename == out_event.f_debug_filename);
            CATCH_REQUIRE(8U == out_event.f_debug_offset);

            CATCH_REQUIRE(request_id == out_event.f_request_id);
            CATCH_REQUIRE(size == out_event.f_data.size());
            CATCH_REQUIRE_LONG_STRING(std::string(reinterpret_cast<char const *>(data.data()), data.size())
                                    , std::string(reinterpret_cast<char const *>(out_event.f_data.data()), out_event.f_data.size()));
            CATCH_REQUIRE(prinbee::status_t::STATUS_READY == out_event.f_status);
            CATCH_REQUIRE(event_time == out_event.f_event_time);

            CATCH_REQUIRE_FALSE(j.next_event(out_event));

            CATCH_REQUIRE_FALSE(j.event_forwarded("inexistant"));
            CATCH_REQUIRE_FALSE(j.event_acknowledged("inexistant"));
            CATCH_REQUIRE_FALSE(j.event_completed("inexistant"));
            CATCH_REQUIRE_FALSE(j.event_failed("inexistant"));

            // Process sequence
            //
            bool expect_success(true);
            bool gone(false);
            prinbee::status_t last_success(prinbee::status_t::STATUS_UNKNOWN);
            for(auto const & status : sequence)
            {
                switch(status)
                {
                case prinbee::status_t::STATUS_UNKNOWN:
                    expect_success = false;
                    continue;

                case prinbee::status_t::STATUS_READY:
                    CATCH_REQUIRE(status != prinbee::status_t::STATUS_READY);
                    break;

                case prinbee::status_t::STATUS_FORWARDED:
                    CATCH_REQUIRE(j.event_forwarded(request_id) == expect_success);
                    break;

                case prinbee::status_t::STATUS_ACKNOWLEDGED:
                    CATCH_REQUIRE(j.event_acknowledged(request_id) == expect_success);
                    break;

                case prinbee::status_t::STATUS_COMPLETED:
                    CATCH_REQUIRE(j.event_completed(request_id) == expect_success);
                    gone = true;
                    break;

                case prinbee::status_t::STATUS_FAILED:
                    CATCH_REQUIRE(j.event_failed(request_id) == expect_success);
                    gone = true;
                    break;

                }
                CATCH_REQUIRE_FALSE(j.next_event(out_event));
                j.rewind();
                if(gone)
                {
                    // if gone, a second attempt still fails
                    //
                    CATCH_REQUIRE_FALSE(j.next_event(out_event));
                }
                else
                {
                    // not gone yet, all the data is still accessible
                    //
                    prinbee::out_event_t out_event2;
                    CATCH_REQUIRE(j.next_event(out_event2));

                    // at the moment the debug does not get cleared, so we
                    // used a separate structure to verify that by default
                    // the debug data remains untouched
                    //
                    CATCH_REQUIRE("" == out_event2.f_debug_filename);
                    CATCH_REQUIRE(0 == out_event2.f_debug_offset);

                    CATCH_REQUIRE(request_id == out_event2.f_request_id);
                    CATCH_REQUIRE(size == out_event2.f_data.size());
                    CATCH_REQUIRE(data == out_event2.f_data);
                    if(expect_success)
                    {
                        CATCH_REQUIRE(status == out_event2.f_status);
                        last_success = out_event2.f_status;
                    }
                    else
                    {
                        // on error, it does not change
                        //
                        CATCH_REQUIRE(last_success == out_event2.f_status);
                    }
                    CATCH_REQUIRE(event_time == out_event2.f_event_time);
                }

                CATCH_REQUIRE_FALSE(j.next_event(out_event));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_status_sequence: verify the delete functionality when empty")
    {
        std::random_device rd;
        std::mt19937 g(rd());

        std::string const path(conf_path("journal_delete"));

        for(int sync(0); sync < 3; ++sync)
        {
            {
                advgetopt::conf_file::reset_conf_files();
                prinbee::journal j(path);
                CATCH_REQUIRE(j.set_file_management(prinbee::file_management_t::FILE_MANAGEMENT_DELETE));
                CATCH_REQUIRE(j.set_maximum_events(5));
                CATCH_REQUIRE(j.set_sync(static_cast<prinbee::sync_t>(sync)));
                CATCH_REQUIRE(j.is_valid());

                std::vector<int> ids;
                for(int id(1); id <= 10; ++id)
                {
                    std::size_t const size(rand() % 1024 + 1);
                    std::vector<std::uint8_t> data(size);
                    for(std::size_t idx(0); idx < size; ++idx)
                    {
                        data[idx] = rand();
                    }
                    prinbee::in_event_t const event =
                    {
                        .f_request_id = prinbee::id_to_string(id),
                        .f_size = size,
                        .f_data = data.data(),
                    };
                    snapdev::timespec_ex const event_time(snapdev::now());
                    snapdev::timespec_ex pass_time(event_time);
                    CATCH_REQUIRE(j.add_event(event, pass_time));
                    CATCH_REQUIRE(event_time == pass_time);

                    ids.push_back(id);
                }

                for(int status(0); status < 3; ++status)
                {
                    std::shuffle(ids.begin(), ids.end(), g);

                    for(auto const & id : ids)
                    {
                        switch(status)
                        {
                        case 0:
                            CATCH_REQUIRE(j.event_forwarded(prinbee::id_to_string(id)));
                            break;

                        case 1:
                            CATCH_REQUIRE(j.event_acknowledged(prinbee::id_to_string(id)));
                            break;

                        case 2:
                            CATCH_REQUIRE(j.event_completed(prinbee::id_to_string(id)));
                            break;

                        default:
                            CATCH_REQUIRE(!"unknown status");

                        }
                    }
                }
            }

            // make sure the DELETE happened
            //
            for(int idx(0); idx < 3; ++idx)
            {
                std::string const filename(event_filename(path, idx));
                CATCH_REQUIRE(access(filename.c_str(), R_OK) != 0);
            }

            // just re-opening does not re-create files
            {
                prinbee::journal j(path);
                CATCH_REQUIRE(j.empty());
            }

            // make sure the files were not re-created
            //
            for(int idx(0); idx < 3; ++idx)
            {
                std::string const filename(event_filename(path, idx));
                CATCH_REQUIRE(access(filename.c_str(), R_OK) != 0);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_status_sequence: verify the delete functionality when not empty")
    {
        std::random_device rd;
        std::mt19937 g(rd());

        for(int sync(0); sync < 3; ++sync)
        {
            std::string const name("journal_truncate_delete-" + std::to_string(sync));
            std::string const path(conf_path(name));

            {
                advgetopt::conf_file::reset_conf_files();
                prinbee::journal j(path);
                CATCH_REQUIRE(j.set_file_management(prinbee::file_management_t::FILE_MANAGEMENT_DELETE));
                CATCH_REQUIRE(j.set_maximum_events(5));
                CATCH_REQUIRE(j.set_sync(static_cast<prinbee::sync_t>(sync)));
                CATCH_REQUIRE(j.is_valid());

                std::vector<int> ids;
                for(int id(1); id <= 10; ++id)
                {
                    std::size_t const size(rand() % 1024 + 1);
                    std::vector<std::uint8_t> data(size);
                    for(std::size_t idx(0); idx < size; ++idx)
                    {
                        data[idx] = rand();
                    }
                    prinbee::in_event_t const event =
                    {
                        .f_request_id = prinbee::id_to_string(id),
                        .f_size = size,
                        .f_data = data.data(),
                    };
                    snapdev::timespec_ex const event_time(snapdev::now());
                    snapdev::timespec_ex pass_time(event_time);
                    CATCH_REQUIRE(j.add_event(event, pass_time));
                    CATCH_REQUIRE(event_time == pass_time);

                    if(rand() % 2 != 0)
                    {
                        ids.push_back(id);
                    }
                }
                if(ids.size() == 10)
                {
                    // make sure at least one entry is out
                    //
                    ids.erase(ids.begin() + rand() % 10);
                }

                for(int status(0); status < 3; ++status)
                {
                    std::shuffle(ids.begin(), ids.end(), g);

                    for(auto const & id : ids)
                    {
                        switch(status)
                        {
                        case 0:
                            CATCH_REQUIRE(j.event_forwarded(prinbee::id_to_string(id)));
                            break;

                        case 1:
                            CATCH_REQUIRE(j.event_acknowledged(prinbee::id_to_string(id)));
                            break;

                        case 2:
                            CATCH_REQUIRE(j.event_completed(prinbee::id_to_string(id)));
                            break;

                        default:
                            CATCH_REQUIRE(!"unknown status");

                        }
                    }
                }
            }

            {
                // make sure the DELETE does not happen when not empty
                //
                for(int idx(0); idx < 3; ++idx)
                {
                    std::string const filename(event_filename(path, idx));
                    //CATCH_REQUIRE(access(filename.c_str(), R_OK) != 0);

                    struct stat s = {};
                    if(stat(filename.c_str(), &s) == 0)
                    {
                          // main header is 8 bytes (See event_journal_header_t)
                          //
                          CATCH_REQUIRE(s.st_size > 8);
                    }
                    else
                    {
                        // we (probably) reached the last file
                        //
                        int const e(errno);
                        CATCH_REQUIRE(e == ENOENT);

                        // we at least needed 1 file to save the few entries
                        // created above, so idx should never be zero if it
                        // worked as expected
                        //
                        CATCH_REQUIRE(idx > 0);
                        break;
                    }
                }
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_status_sequence: verify the truncate functionality")
    {
        std::random_device rd;
        std::mt19937 g(rd());

        std::string const path(conf_path("journal_truncate"));

        for(int sync(0); sync < 3; ++sync)
        {
            {
                advgetopt::conf_file::reset_conf_files();
                prinbee::journal j(path);
                CATCH_REQUIRE(j.set_file_management(prinbee::file_management_t::FILE_MANAGEMENT_TRUNCATE));
                CATCH_REQUIRE(j.set_maximum_events(5));
                CATCH_REQUIRE(j.set_sync(static_cast<prinbee::sync_t>(sync)));
                CATCH_REQUIRE(j.is_valid());

                std::vector<int> ids;
                for(int id(1); id <= 10; ++id)
                {
                    std::size_t const size(rand() % 1024 + 1);
                    std::vector<std::uint8_t> data(size);
                    for(std::size_t idx(0); idx < size; ++idx)
                    {
                        data[idx] = rand();
                    }
                    prinbee::in_event_t const event =
                    {
                        .f_request_id = prinbee::id_to_string(id),
                        .f_size = size,
                        .f_data = data.data(),
                    };
                    snapdev::timespec_ex const event_time(snapdev::now());
                    snapdev::timespec_ex pass_time(event_time);
                    CATCH_REQUIRE(j.add_event(event, pass_time));
                    CATCH_REQUIRE(event_time == pass_time);

                    ids.push_back(id);
                }

                for(int status(0); status < 3; ++status)
                {
                    std::shuffle(ids.begin(), ids.end(), g);

                    for(auto const & id : ids)
                    {
                        switch(status)
                        {
                        case 0:
                            CATCH_REQUIRE(j.event_forwarded(prinbee::id_to_string(id)));
                            break;

                        case 1:
                            CATCH_REQUIRE(j.event_acknowledged(prinbee::id_to_string(id)));
                            break;

                        case 2:
                            CATCH_REQUIRE(j.event_completed(prinbee::id_to_string(id)));
                            break;

                        default:
                            CATCH_REQUIRE(!"unknown status");

                        }
                    }
                }
            }

            {
                // make sure the TRUNCATE happened
                //
                for(int idx(0); idx < 3; ++idx)
                {
                    std::string const filename(event_filename(path, idx));
                    struct stat s = {};
                    if(stat(filename.c_str(), &s) == 0)
                    {
                        CATCH_REQUIRE(s.st_size == 8);  // main header is 8 bytes (See event_journal_header_t)
                    }
                    else
                    {
                        // we (probably) reached the last file
                        //
                        int const e(errno);
                        CATCH_REQUIRE(e == ENOENT);

                        // we at least needed 1 file to save the few entries
                        // created above, so idx should never be zero if it
                        // worked as expected
                        //
                        CATCH_REQUIRE(idx > 0);
                        break;
                    }
                }
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("journal_event_list", "[journal]")
{
    CATCH_START_SECTION("journal_event_list: verify the unicity of the timestamp")
    {
        std::random_device rd;
        std::mt19937 g(rd());

        std::string const name("journal_repeated_event_time");
        std::string const path(conf_path(name));

        snapdev::timespec_ex const start_time(snapdev::now());
        snapdev::timespec_ex event_time(start_time);
        snapdev::timespec_ex pass_time(event_time);

        // we want the ids to be in a different order than the time
        //
        std::vector<int> ids;
        for(int id(1); id <= 10; ++id)
        {
            ids.push_back(id);
        }
        std::shuffle(ids.begin(), ids.end(), g);

        std::vector<snapdev::timespec_ex> times(ids.size());
        {
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            CATCH_REQUIRE(j.set_file_management(prinbee::file_management_t::FILE_MANAGEMENT_DELETE));
            CATCH_REQUIRE(j.set_maximum_events(5));
            CATCH_REQUIRE(j.is_valid());
            CATCH_REQUIRE(j.empty());

            for(int r(0); r < 10; ++r)
            {
                std::size_t const size(rand() % 124 + 1);
                std::vector<std::uint8_t> data(size);
                for(std::size_t idx(0); idx < size; ++idx)
                {
                    data[idx] = rand();
                }
                prinbee::in_event_t const event =
                {
                    .f_request_id = prinbee::id_to_string(ids[r]),
                    .f_size = size,
                    .f_data = data.data(),
                };
                CATCH_REQUIRE(j.add_event(event, pass_time));
                CATCH_REQUIRE(event_time == pass_time);
                CATCH_REQUIRE(j.size() == r + 1ULL);
                CATCH_REQUIRE_FALSE(j.empty());
                times[ids[r] - 1] = pass_time;

                ++event_time; // next time it will be incremented by one
            }
        }

        {
            prinbee::journal j(path);
            event_time = start_time;
            for(int r(0); r < 10; ++r)
            {
                prinbee::out_event_t event;
                CATCH_REQUIRE(j.next_event(event));
                CATCH_REQUIRE(event_time == event.f_event_time);
                CATCH_REQUIRE(prinbee::id_to_string(ids[r]) == event.f_request_id);
                ++event_time;
            }

            // make sure we reached the end
            //
            {
                prinbee::out_event_t event;
                CATCH_REQUIRE_FALSE(j.next_event(event));
            }
        }

        {
            prinbee::journal j(path);
            for(int r(0); r < 10; ++r)
            {
                prinbee::out_event_t event;
                CATCH_REQUIRE(j.next_event(event, false));
                CATCH_REQUIRE(times[r] == event.f_event_time);
                CATCH_REQUIRE(prinbee::id_to_string(r + 1) == event.f_request_id);
            }

            // make sure we reached the end
            //
            {
                prinbee::out_event_t event;
                CATCH_REQUIRE_FALSE(j.next_event(event, false));
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("journal_event_files", "[journal]")
{
    CATCH_START_SECTION("journal_event_files: reduce number of files with missing files")
    {
        std::string const path(conf_path("journal_event_files"));
        advgetopt::conf_file::reset_conf_files();

        {
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());
            CATCH_REQUIRE(j.set_maximum_number_of_files(5));

            // 9 to 10 Kb of data per message so we should be able to add
            // between 6 and 7 messages per file; i.e. 14 maximum then we
            // are expecting an error on the add_event()
            //
            std::size_t const size(rand() % 1024 + 1);
            std::vector<std::uint8_t> data(size);
            for(std::size_t idx(0); idx < size; ++idx)
            {
                data[idx] = rand();
            }
            prinbee::in_event_t const event =
            {
                .f_request_id = "id-1",
                .f_size = size,
                .f_data = data.data(),
            };
            snapdev::timespec_ex event_time(snapdev::now());
            CATCH_REQUIRE(j.add_event(event, event_time));

            // trying to reduce the number of files works fine when events are
            // only in the very first file
            //
            CATCH_REQUIRE(j.set_maximum_number_of_files(prinbee::JOURNAL_MINIMUM_NUMBER_OF_FILES));
        }

        {
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());
            CATCH_REQUIRE(j.get_file_management() == prinbee::file_management_t::FILE_MANAGEMENT_KEEP);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("journal_event_errors", "[journal][error]")
{
    CATCH_START_SECTION("journal_event_errors: trying to re-add the same event multiple times fails")
    {
        std::string const path(conf_path("journal_duplicate_event"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());

        std::size_t const size(rand() % 10 * 1024 + 1);
        std::vector<std::uint8_t> data(size);
        for(std::size_t idx(0); idx < size; ++idx)
        {
            data[idx] = rand();
        }
        prinbee::in_event_t const event =
        {
            .f_request_id = "id-123",
            .f_size = size,
            .f_data = data.data(),
        };
        snapdev::timespec_ex event_time(snapdev::now());
        CATCH_REQUIRE(j.add_event(event, event_time));

        // if we try again, it fails
        //
        event_time = snapdev::now();
        CATCH_REQUIRE_FALSE(j.add_event(event, event_time));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_errors: request_id too long")
    {
        std::string const path(conf_path("journal_large_event"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());

        std::size_t const size(rand() % 10 * 1024 + 1);
        std::vector<std::uint8_t> data(size);
        for(std::size_t idx(0); idx < size; ++idx)
        {
            data[idx] = rand();
        }
        prinbee::in_event_t const event =
        {
            .f_request_id = "for a request identifier too be way to long here it needs to be some two hundred and fifty six or way more characters which means this is a really long sentence to make it happen and well, since I have a lot of imagination that is really no issue at all, right?",
            .f_size = size,
            .f_data = data.data(),
        };
        snapdev::timespec_ex event_time(snapdev::now());
        CATCH_REQUIRE_FALSE(j.add_event(event, event_time));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_errors: invalid number of files")
    {
        std::string const path(conf_path("journal_out_of_range"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);

        for(std::uint32_t count(0); count < prinbee::JOURNAL_MINIMUM_NUMBER_OF_FILES; ++count)
        {
            std::stringstream ss;
            ss << "out_of_range: maximum number of files ("
               << std::to_string(count)
               << ") is out of range: ["
               << std::to_string(prinbee::JOURNAL_MINIMUM_NUMBER_OF_FILES)
               << ".."
               << std::to_string(prinbee::JOURNAL_MAXIMUM_NUMBER_OF_FILES)
               << "]";
            CATCH_REQUIRE_THROWS_MATCHES(
                      j.set_maximum_number_of_files(count)
                    , prinbee::out_of_range
                    , Catch::Matchers::ExceptionMessage(ss.str()));
        }
        for(std::uint32_t count(prinbee::JOURNAL_MAXIMUM_NUMBER_OF_FILES + 1); count < prinbee::JOURNAL_MAXIMUM_NUMBER_OF_FILES + 100; ++count)
        {
            std::stringstream ss;
            ss << "out_of_range: maximum number of files ("
               << std::to_string(count)
               << ") is out of range: ["
               << std::to_string(prinbee::JOURNAL_MINIMUM_NUMBER_OF_FILES)
               << ".."
               << std::to_string(prinbee::JOURNAL_MAXIMUM_NUMBER_OF_FILES)
               << "]";
            CATCH_REQUIRE_THROWS_MATCHES(
                      j.set_maximum_number_of_files(count)
                    , prinbee::out_of_range
                    , Catch::Matchers::ExceptionMessage(ss.str()));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_errors: missing folder")
    {
        std::string const path(conf_path("journal_missing", true));
        chmod(path.c_str(), 0); // remove permissions so the add_event() fails with EPERM
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());

        std::size_t const size(rand() % 10 * 1024 + 1);
        std::vector<std::uint8_t> data(size);
        for(std::size_t idx(0); idx < size; ++idx)
        {
            data[idx] = rand();
        }
        prinbee::in_event_t const event =
        {
            .f_request_id = "id-123",
            .f_size = size,
            .f_data = data.data(),
        };
        snapdev::timespec_ex event_time(snapdev::now());
        CATCH_REQUIRE_FALSE(j.add_event(event, event_time));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_errors: filled up journal (small size)")
    {
        std::string const path(conf_path("journal_filled"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());

        j.set_maximum_file_size(prinbee::JOURNAL_MINIMUM_FILE_SIZE);

        // 9 to 10 Kb of data per message so we should be able to add
        // between 6 and 7 messages per file; i.e. 14 maximum then we
        // are expecting an error on the add_event()
        //
        std::vector<std::uint8_t> data;
        bool success(false);
        int count(0);
        for(; count < 15; ++count)
        {
            std::size_t const size(rand() % 1024 + 1024 * 9);
            data.resize(size);
            for(std::size_t idx(0); idx < size; ++idx)
            {
                data[idx] = rand();
            }
            prinbee::in_event_t const event =
            {
                .f_request_id = "id-" + std::to_string(count),
                .f_size = size,
                .f_data = data.data(),
            };
            snapdev::timespec_ex event_time(snapdev::now());
            bool const r(j.add_event(event, event_time));
            if(!r)
            {
                success = true;
                break;
            }
        }
        CATCH_REQUIRE(success);

        // mark a few as complete and attempt another insert, it should
        // still fail
        //
        std::vector<int> ids(count);
        for(int mark_complete(0); mark_complete < count; ++mark_complete)
        {
            ids[mark_complete] = mark_complete;
        }
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(ids.begin(), ids.end(), g);
        int const complete_count(rand() % 3 + 1);
        for(int idx(0); idx < complete_count; ++idx)
        {
            std::string const request_id("id-" + std::to_string(ids[idx]));
            if((rand() & 1) == 0)
            {
                CATCH_REQUIRE(j.event_completed(request_id));
            }
            else
            {
                CATCH_REQUIRE(j.event_failed(request_id));
            }
        }

        {
            // as is, it still overflows (because we are not compressing)
            //
            //std::size_t const size(rand() % 1024 + 1024 * 8);
            //std::vector<std::uint8_t> data(size);
            //for(std::size_t idx(0); idx < size; ++idx)
            //{
            //    data[idx] = rand();
            //}
            prinbee::in_event_t const event =
            {
                .f_request_id = "id-extra",
                .f_size = data.size(),
                .f_data = data.data(),
            };
            snapdev::timespec_ex event_time(snapdev::now());
            CATCH_REQUIRE_FALSE(j.add_event(event, event_time));

            // however, if we turn on the "allow compression" flag, it works
            //
            CATCH_REQUIRE(j.set_compress_when_full(true));
            CATCH_REQUIRE(j.add_event(event, event_time));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_errors: fail with invalid size as ID is not complete and data is missing")
    {
        std::string const name("journal_incomplete_id");
        std::string const path(conf_path(name));

        // create a journal file with one valid event
        {
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());
            CATCH_REQUIRE(j.empty());

            std::uint8_t data[20] = {};
            prinbee::in_event_t const event =
            {
                .f_request_id = "this-id",
                .f_size = sizeof(data),
                .f_data = data,
            };
            snapdev::timespec_ex now(snapdev::now());
            CATCH_REQUIRE(j.add_event(event, now));
            CATCH_REQUIRE(j.size() == 1ULL);
            CATCH_REQUIRE_FALSE(j.empty());
        }

        // open that journal and add a broken header (invalid identifier) 
        {
            std::string const filename(event_filename(path, 0));
            std::ofstream out(filename, std::ios::app | std::ios::binary);
            std::uint8_t const header[] = {
                'e', 'v',                               // f_magic
                static_cast<std::uint8_t>(prinbee::status_t::STATUS_READY), // f_status
                sizeof("next-id") - 1,                  // f_request_id_size
                23, 0, 0, 0,                            // f_size
                0, 0, 0, 0, 0, 0, 0, 0,                 // f_time
                0, 0, 0, 0, 0, 0, 0, 0,
            };
            out.write(reinterpret_cast<char const *>(header), sizeof(header));
            out.write("next", 4);                           // <-- only 4 bytes
        }

        {
            prinbee::journal j(path);
            prinbee::out_event_t event;

            // we find the first valid event
            //
            CATCH_REQUIRE(j.next_event(event));
            CATCH_REQUIRE("this-id" == event.f_request_id);

            // make sure we reached the end; the second event was invalid
            //
            CATCH_REQUIRE_FALSE(j.next_event(event));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_errors: invalid event date & time")
    {
        std::string const name("journal_wrong_time");
        std::string const path(conf_path(name));

        // create a journal file with one valid event
        {
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());
            CATCH_REQUIRE(j.empty());

            std::uint8_t data[20] = {};
            prinbee::in_event_t event =
            {
                .f_request_id = "this-id",
                .f_size = sizeof(data),
                .f_data = data,
            };
            snapdev::timespec_ex now(snapdev::now());
            CATCH_REQUIRE(j.add_event(event, now));
            CATCH_REQUIRE(j.size() == 1ULL);
            CATCH_REQUIRE_FALSE(j.empty());

            // trying to add an event in the future fails
            //
            snapdev::timespec_ex soon(snapdev::now());
            soon += snapdev::timespec_ex(100, 0);            // 100 seconds in the future
            event.f_request_id = "future";
            CATCH_REQUIRE_FALSE(j.add_event(event, soon));
        }

        // open that journal and add a broken header (invalid date & time) 
        {
            std::string const filename(event_filename(path, 0));
            std::ofstream out(filename, std::ios::app | std::ios::binary);
            snapdev::timespec_ex soon(snapdev::now());
            soon += snapdev::timespec_ex(100, 0);            // 100 seconds in the future
            char data[32];
            std::uint8_t const header[] = {
                'e', 'v',                               // f_magic
                static_cast<std::uint8_t>(prinbee::status_t::STATUS_READY), // f_status
                sizeof("next-id") - 1,                  // f_request_id_size
                sizeof(data), 0, 0, 0,                  // f_size
                static_cast<std::uint8_t>(soon.tv_sec >>  0), // f_time
                static_cast<std::uint8_t>(soon.tv_sec >>  8),
                static_cast<std::uint8_t>(soon.tv_sec >> 16),
                static_cast<std::uint8_t>(soon.tv_sec >> 24),
                static_cast<std::uint8_t>(soon.tv_sec >> 32),
                static_cast<std::uint8_t>(soon.tv_sec >> 40),
                static_cast<std::uint8_t>(soon.tv_sec >> 48),
                static_cast<std::uint8_t>(soon.tv_sec >> 56),
                static_cast<std::uint8_t>(soon.tv_nsec >>  0),
                static_cast<std::uint8_t>(soon.tv_nsec >>  8),
                static_cast<std::uint8_t>(soon.tv_nsec >> 16),
                static_cast<std::uint8_t>(soon.tv_nsec >> 24),
                static_cast<std::uint8_t>(soon.tv_nsec >> 32),
                static_cast<std::uint8_t>(soon.tv_nsec >> 40),
                static_cast<std::uint8_t>(soon.tv_nsec >> 48),
                static_cast<std::uint8_t>(soon.tv_nsec >> 56),
            };
            out.write(reinterpret_cast<char const *>(header), sizeof(header));
            out.write("next-id", sizeof("next-id") - 1);
            out.write(data, sizeof(data));
        }

        {
            prinbee::journal j(path);
            prinbee::out_event_t event;

            // we find the first valid event
            //
            CATCH_REQUIRE(j.next_event(event));
            CATCH_REQUIRE("this-id" == event.f_request_id);

            // make sure we reached the end; the second event was invalid
            //
            CATCH_REQUIRE_FALSE(j.next_event(event));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_errors: invalid end marker")
    {
        // to test the conversions, we need multiple cases so use a loop
        //
        struct marker
        {
            char a;
            char b;
        };
        std::vector<marker> invalid_markers{
            { 'n', 'g' },
            { '\0', '@' },       // starts well, bad ending
            { 0x03, 0x07 },
            { 0x7F, static_cast<char>(0x97) },
        };
        int count(0);
        for(auto const & bad_marker : invalid_markers)
        {
            ++count;
            std::string name("journal_invalid_end_marker-");
            name += std::to_string(count);
            std::string const path(conf_path(name));

            // create a journal file with one valid event
            {
                advgetopt::conf_file::reset_conf_files();
                prinbee::journal j(path);
                CATCH_REQUIRE(j.is_valid());
                CATCH_REQUIRE(j.empty());

                std::uint8_t data[20] = {};
                prinbee::in_event_t event =
                {
                    .f_request_id = "this-id",
                    .f_size = sizeof(data),
                    .f_data = data,
                };
                snapdev::timespec_ex now(snapdev::now());
                CATCH_REQUIRE(j.add_event(event, now));
                CATCH_REQUIRE(j.size() == 1ULL);
                CATCH_REQUIRE_FALSE(j.empty());
            }

            // open that journal and add a broken end marker
            // the header and data are otherwise valid
            {
                std::string const filename(event_filename(path, 0));
                std::ofstream out(filename, std::ios::app | std::ios::binary);
                char data[1];
                std::uint8_t const header[] = {
                    static_cast<std::uint8_t>(bad_marker.a), // f_magic
                    static_cast<std::uint8_t>(bad_marker.b),
                    static_cast<std::uint8_t>(prinbee::status_t::STATUS_READY), // f_status
                    sizeof("next-id") - 1,                  // f_request_id_size
                    sizeof(data), 0, 0, 0,                  // f_size
                    0, 0, 0, 0, 0, 0, 0, 0,                 // f_time
                    0, 0, 0, 0, 0, 0, 0, 0,
                };
                out.write(reinterpret_cast<char const *>(header), sizeof(header));
                out.write("next-id", sizeof("next-id") - 1);
                out.write(data, sizeof(data));
            }

            {
                prinbee::journal j(path);
                prinbee::out_event_t event;

                // we find the first valid event
                //
                CATCH_REQUIRE(j.next_event(event));
                CATCH_REQUIRE("this-id" == event.f_request_id);

                // make sure we reached the end; the second event was invalid
                //
                CATCH_REQUIRE_FALSE(j.next_event(event));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_errors: incomplete header")
    {
        for(int idx(0); idx < 5; ++idx)
        {
            std::string name("journal_incomplete_header-");
            name += std::to_string(idx + 1);
            std::string const path(conf_path(name));

            // create a journal file with one valid event
            {
                advgetopt::conf_file::reset_conf_files();
                prinbee::journal j(path);
                CATCH_REQUIRE(j.is_valid());
                CATCH_REQUIRE(j.empty());

                std::uint8_t data[20] = {};
                prinbee::in_event_t event =
                {
                    .f_request_id = "this-id",
                    .f_size = sizeof(data),
                    .f_data = data,
                };
                snapdev::timespec_ex now(snapdev::now());
                CATCH_REQUIRE(j.add_event(event, now));
                CATCH_REQUIRE(j.size() == 1ULL);
                CATCH_REQUIRE_FALSE(j.empty());
            }

            // open that journal and add a broken end marker
            // the header and data are otherwise valid
            {
                std::string const filename(event_filename(path, 0));
                std::ofstream out(filename, std::ios::app | std::ios::binary);
                char data[1];
                std::uint8_t const header[] = {
                    'e', 'v',                               // f_magic
                    static_cast<std::uint8_t>(prinbee::status_t::STATUS_READY), // f_status
                    sizeof("next-id") - 1,                  // f_request_id_size
                    sizeof(data), 0, 0, 0,                  // f_size
                    0, 0, 0, 0, 0, 0, 0, 0,                 // f_time
                    0, 0, 0, 0, 0, 0, 0, 0,
                };
                std::size_t const size(rand() % (sizeof(header) - 1) + 1);
                out.write(reinterpret_cast<char const *>(header), size);
            }

            {
                prinbee::journal j(path);
                prinbee::out_event_t event;

                // we find the first valid event
                //
                CATCH_REQUIRE(j.next_event(event));
                CATCH_REQUIRE("this-id" == event.f_request_id);

                // make sure we reached the end; the second event was invalid
                // note: in this case we do not get an error message
                //
                CATCH_REQUIRE_FALSE(j.next_event(event));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_errors: invalid magic (start of file header magic tampered)")
    {
        for(int pos(0); pos < 6; ++pos)
        {
            std::string name("journal_invalid_magic-");
            name += std::to_string(pos);
            std::string const path(conf_path(name));

            // create a journal file with one valid event
            // (without the event, it does not create the file)
            {
                advgetopt::conf_file::reset_conf_files();
                prinbee::journal j(path);
                CATCH_REQUIRE(j.is_valid());
                CATCH_REQUIRE(j.empty());

                std::uint8_t data[20] = {};
                prinbee::in_event_t event =
                {
                    .f_request_id = "this-id",
                    .f_size = sizeof(data),
                    .f_data = data,
                };
                snapdev::timespec_ex now(snapdev::now());
                CATCH_REQUIRE(j.add_event(event, now));
                CATCH_REQUIRE(j.size() == 1ULL);
                CATCH_REQUIRE_FALSE(j.empty());
            }

            // smash one of the header characters
            {
                std::string const filename(event_filename(path, 0));
                std::fstream out(filename);
                out.seekp(pos);
                std::string header;
                header += 'E';
                header += 'V';
                header += 'T';
                header += 'J';
                header += '\1';
                header += '\0';
                for(;;)
                {
                    char c(static_cast<char>(rand()));
                    if(c != header[pos])
                    {
                        out.write(&c, 1);
                        break;
                    }
                }
            }

            {
                prinbee::journal j(path);
                prinbee::out_event_t event;

                // we find no events
                //
                CATCH_REQUIRE_FALSE(j.next_event(event));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_errors: short magic (start of file header)")
    {
        for(int size(0); size < 8; ++size)
        {
            std::string name("journal_short_magic-");
            name += std::to_string(size);
            std::string const path(conf_path(name));

            // create a journal file with one valid event
            // (without the event, it does not create the file)
            {
                advgetopt::conf_file::reset_conf_files();
                prinbee::journal j(path);
                CATCH_REQUIRE(j.is_valid());
                CATCH_REQUIRE(j.empty());

                std::uint8_t data[20] = {};
                prinbee::in_event_t event =
                {
                    .f_request_id = "this-id",
                    .f_size = sizeof(data),
                    .f_data = data,
                };
                snapdev::timespec_ex now(snapdev::now());
                CATCH_REQUIRE(j.add_event(event, now));
                CATCH_REQUIRE(j.size() == 1ULL);
                CATCH_REQUIRE_FALSE(j.empty());
            }

            // truncate the header to `size` bytes
            {
                std::string const filename(event_filename(path, 0));
                CATH_REQUIRE(truncate(filename.c_str(), size) == 0);
            }

            {
                prinbee::journal j(path);
                prinbee::out_event_t event;

                // we find no events
                //
                CATCH_REQUIRE_FALSE(j.next_event(event));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_errors: can't reduce number of files in a filled up journal")
    {
        std::string const path(conf_path("journal_reduce_max_files"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        CATCH_REQUIRE(j.set_maximum_number_of_files(5));

        j.set_maximum_file_size(prinbee::JOURNAL_MINIMUM_FILE_SIZE);

        // 9 to 10 Kb of data per message so we should be able to add
        // between 6 and 7 messages per file; i.e. 14 maximum then we
        // are expecting an error on the add_event()
        //
        std::vector<std::uint8_t> data;
        bool success(false);
        int count(0);
        for(;; ++count)
        {
            std::size_t const size(rand() % 1024 + 1024 * 9);
            data.resize(size);
            for(std::size_t idx(0); idx < size; ++idx)
            {
                data[idx] = rand();
            }
            prinbee::in_event_t const event =
            {
                .f_request_id = "id-" + std::to_string(count),
                .f_size = size,
                .f_data = data.data(),
            };
            snapdev::timespec_ex event_time(snapdev::now());
            bool const r(j.add_event(event, event_time));
            if(!r)
            {
                success = true;
                break;
            }
        }
        CATCH_REQUIRE(success);

        // trying to reduce the number of files when full fails with
        // an exception
        //
        CATCH_REQUIRE_THROWS_MATCHES(
                  j.set_maximum_number_of_files(prinbee::JOURNAL_MINIMUM_NUMBER_OF_FILES)
                , prinbee::file_still_in_use
                , Catch::Matchers::ExceptionMessage(
                          "prinbee_exception: it is not currently possible to reduce the maximum number of files when some of those over the new limit are still in use."));

        // mark all as complete and re-attempt the reduction
        //
        for(int idx(0); idx < count; ++idx)
        {
            std::string const request_id("id-" + std::to_string(idx));
            if((rand() & 1) == 0)
            {
                CATCH_REQUIRE(j.event_completed(request_id));
            }
            else
            {
                CATCH_REQUIRE(j.event_failed(request_id));
            }
        }

        CATCH_REQUIRE(j.set_maximum_number_of_files(prinbee::JOURNAL_MINIMUM_NUMBER_OF_FILES));
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
