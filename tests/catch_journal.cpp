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


void unlink_conf(std::string const & path)
{
    std::string filename(conf_filename(path));
    if(unlink(filename.c_str()) != 0)
    {
        if(errno != ENOENT)
        {
            perror("unlink() returned unexpected error.");
            CATCH_REQUIRE(!"unlink() returned an unexpected error");
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
            REPLAY_ORDER,
            SYNC,
        };
        for(int index(0); index < 7; ++index)
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

            case REPLAY_ORDER:
                {
                    prinbee::replay_order_t const value(static_cast<prinbee::replay_order_t>(rand() % 2));
                    CATCH_REQUIRE(j.set_replay_order(value));
                    switch(value)
                    {
                    case prinbee::replay_order_t::REPLAY_ORDER_REQUEST_ID:
                        expected_result = "request-id";
                        break;

                    case prinbee::replay_order_t::REPLAY_ORDER_EVENT_TIME:
                        expected_result = "event-time";
                        break;

                    }
                }
                break;

            case SYNC:
                CATCH_REQUIRE(j.set_sync(true));
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

            it = conf_values.find("replay_order");
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE((index == REPLAY_ORDER ? expected_result : "request-id") == it->second);
            conf_values.erase(it);

            it = conf_values.find("sync");
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE((index == SYNC ? "true" : "false") == it->second);
            conf_values.erase(it);

            CATCH_REQUIRE(conf_values.empty());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: reducing the number of files generates a TODO")
    {
        std::string expected_result;
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

    CATCH_START_SECTION("journal_options: invalid replay order numbers")
    {
        std::string const path(conf_path("journal_options"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());

        for(int i(-100); i <= 100; ++i)
        {
            switch(static_cast<prinbee::replay_order_t>(i))
            {
            case prinbee::replay_order_t::REPLAY_ORDER_REQUEST_ID:
            case prinbee::replay_order_t::REPLAY_ORDER_EVENT_TIME:
                // these are valid, ignore
                break;

            default:
                CATCH_REQUIRE_THROWS_MATCHES(
                          j.set_replay_order(static_cast<prinbee::replay_order_t>(i))
                        , prinbee::invalid_parameter
                        , Catch::Matchers::ExceptionMessage(
                                  "prinbee_exception: unsupported replay order number"));
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
        for(auto const & sequence : next_status)
        {
            std::string expected_result;
            std::string const path(conf_path("journal_events"));
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
            snapdev::timespec_ex const event_time(snapdev::now());
            CATCH_REQUIRE(j.add_event(event, event_time));

            // the only way to verify that the event was sent to the journal
            // properly is to read it back using the next_event() function, but
            // since we just added a first even, the next_event() won't find
            // it (i.e. that iterator is already pointing to end()), so we'll
            // need a rewind() call first
            //
            prinbee::out_event_t out_event;
            CATCH_REQUIRE_FALSE(j.next_event(out_event));

            j.rewind();
            CATCH_REQUIRE(j.next_event(out_event));

            CATCH_REQUIRE("id-123" == out_event.f_request_id);
            CATCH_REQUIRE(size == out_event.f_data.size());
            CATCH_REQUIRE(data == out_event.f_data);
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
                    CATCH_REQUIRE(j.event_forwarded("id-123") == expect_success);
                    break;

                case prinbee::status_t::STATUS_ACKNOWLEDGED:
                    CATCH_REQUIRE(j.event_acknowledged("id-123") == expect_success);
                    break;

                case prinbee::status_t::STATUS_COMPLETED:
                    CATCH_REQUIRE(j.event_completed("id-123") == expect_success);
                    break;

                case prinbee::status_t::STATUS_FAILED:
                    CATCH_REQUIRE(j.event_failed("id-123") == expect_success);
                    break;

                }
                CATCH_REQUIRE_FALSE(j.next_event(out_event));
                j.rewind();
                CATCH_REQUIRE(j.next_event(out_event));

                CATCH_REQUIRE("id-123" == out_event.f_request_id);
                CATCH_REQUIRE(size == out_event.f_data.size());
                CATCH_REQUIRE(data == out_event.f_data);
                if(expect_success)
                {
                    CATCH_REQUIRE(status == out_event.f_status);
                    last_success = out_event.f_status;
                }
                else
                {
                    // on error, it does not change
                    //
                    CATCH_REQUIRE(last_success == out_event.f_status);
                }
                CATCH_REQUIRE(event_time == out_event.f_event_time);

                CATCH_REQUIRE_FALSE(j.next_event(out_event));
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("journal_event_errors", "[journal][error]")
{
    CATCH_START_SECTION("journal_event_errors: trying to re-add the same event multiple times fails")
    {
        std::string expected_result;
        std::string const path(conf_path("journal_events"));
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
        std::string expected_result;
        std::string const path(conf_path("journal_events"));
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
        std::string expected_result;
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
        std::string expected_result;
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
}


// vim: ts=4 sw=4 et
