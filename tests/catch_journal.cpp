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
#include    <snapdev/pathinfo.h>


// C++
//
#include    <random>


// advgetopt
//
#include    <advgetopt/conf_file.h>


// last include
//
#include    <snapdev/poison.h>



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
    bool const valid(file.read_all());
    if(!valid)
    {
        std::cerr << "--- error loading configuration file: "
            << file.last_error()
            << "\n";
    }
    CATCH_REQUIRE(valid);
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
    CATCH_START_SECTION("journal_options: set_maximum_number_of_files(): default does nothing")
    {
        std::string const path(conf_path("journal_options"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        CATCH_REQUIRE(j.set_maximum_number_of_files(prinbee::JOURNAL_DEFAULT_NUMBER_OF_FILES));
        std::string const filename(conf_filename(path));
        struct stat s;
        if(stat(filename.c_str(), &s) != 0)
        {
            CATCH_REQUIRE(errno == ENOENT);
        }
        else
        {
            CATCH_REQUIRE(!"set_maximum_number_of_files() default created a configuration file.");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: set_maximum_file_size(): default does nothing")
    {
        std::string const path(conf_path("journal_options"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        CATCH_REQUIRE(j.set_maximum_file_size(prinbee::JOURNAL_DEFAULT_FILE_SIZE));
        std::string const filename(conf_filename(path));
        struct stat s;
        if(stat(filename.c_str(), &s) != 0)
        {
            CATCH_REQUIRE(errno == ENOENT);
        }
        else
        {
            CATCH_REQUIRE(!"set_maximum_file_size() default created a configuration file.");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: set_maximum_events(): default does nothing")
    {
        std::string const path(conf_path("journal_options"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        CATCH_REQUIRE(j.set_maximum_events(prinbee::JOURNAL_DEFAULT_EVENTS));
        std::string const filename(conf_filename(path));
        struct stat s;
        if(stat(filename.c_str(), &s) != 0)
        {
            CATCH_REQUIRE(errno == ENOENT);
        }
        else
        {
            CATCH_REQUIRE(!"set_maximum_events() default created a configuration file.");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: set_inline_attachment_size_threshold(): default does nothing")
    {
        std::string const path(conf_path("journal_options"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        CATCH_REQUIRE(j.set_inline_attachment_size_threshold(prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD));
        std::string const filename(conf_filename(path));
        struct stat s;
        if(stat(filename.c_str(), &s) != 0)
        {
            CATCH_REQUIRE(errno == ENOENT);
        }
        else
        {
            CATCH_REQUIRE(!"set_inline_attachment_size_threshold() default created a configuration file.");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: set_sync(): default does nothing")
    {
        std::string const path(conf_path("journal_options"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        CATCH_REQUIRE(j.set_sync(prinbee::sync_t::SYNC_NONE));
        std::string const filename(conf_filename(path));
        struct stat s;
        if(stat(filename.c_str(), &s) != 0)
        {
            CATCH_REQUIRE(errno == ENOENT);
        }
        else
        {
            CATCH_REQUIRE(!"set_sync() default created a configuration file.");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: set_file_management(): default does nothing")
    {
        std::string const path(conf_path("journal_options"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        CATCH_REQUIRE(j.set_file_management(prinbee::file_management_t::FILE_MANAGEMENT_KEEP));
        std::string const filename(conf_filename(path));
        struct stat s;
        if(stat(filename.c_str(), &s) != 0)
        {
            CATCH_REQUIRE(errno == ENOENT);
        }
        else
        {
            CATCH_REQUIRE(!"set_file_management() default created a configuration file.");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: set_compress_when_full(): default does nothing")
    {
        std::string const path(conf_path("journal_options"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        CATCH_REQUIRE(j.set_compress_when_full(false));
        std::string const filename(conf_filename(path));
        struct stat s;
        if(stat(filename.c_str(), &s) != 0)
        {
            CATCH_REQUIRE(errno == ENOENT);
        }
        else
        {
            CATCH_REQUIRE(!"set_compress_when_full() default created a configuration file.");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: set_attachment_copy_handling(): default does nothing")
    {
        {
            std::string const path(conf_path("journal_options"));
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());
            CATCH_REQUIRE(j.set_attachment_copy_handling(prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK));
            std::string const filename(conf_filename(path));
            struct stat s;
            if(stat(filename.c_str(), &s) != 0)
            {
                CATCH_REQUIRE(errno == ENOENT);
            }
            else
            {
                CATCH_REQUIRE(!"set_attachment_copy_handling() default created a configuration file.");
            }
            CATCH_REQUIRE(j.get_attachment_copy_handling() == prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK);
        }

        // "default" is viewed as "softlink" so it's also the default
        {
            std::string const path(conf_path("journal_options"));
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            CATCH_REQUIRE(j.is_valid());
            CATCH_REQUIRE(j.set_attachment_copy_handling(prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_DEFAULT));
            std::string const filename(conf_filename(path));
            struct stat s;
            if(stat(filename.c_str(), &s) != 0)
            {
                CATCH_REQUIRE(errno == ENOENT);
            }
            else
            {
                CATCH_REQUIRE(!"set_attachment_copy_handling() default created a configuration file.");
            }
            CATCH_REQUIRE(j.get_attachment_copy_handling() == prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_options: verify set options")
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
            INLINE_ATTACHMENT_SIZE_THRESHOLD,
            ATTACHMENT_COPY_HANDLING_SOFTLINK,
            ATTACHMENT_COPY_HANDLING_HARDLINK,
            ATTACHMENT_COPY_HANDLING_REFLINK,
            ATTACHMENT_COPY_HANDLING_FULL,

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
                //CATCH_REQUIRE(j.get_compress_when_full());
                break;

            case FILE_MANAGEMENT:
                {
                    prinbee::file_management_t const value(static_cast<prinbee::file_management_t>(rand() % 3));

                    // just setting the default does not re-save the configuration file
                    // which we need to happen for this test
                    //
                    if(value == prinbee::file_management_t::FILE_MANAGEMENT_KEEP)
                    {
                        CATCH_REQUIRE(j.set_file_management(rand() % 2 == 0
                                    ? prinbee::file_management_t::FILE_MANAGEMENT_TRUNCATE
                                    : prinbee::file_management_t::FILE_MANAGEMENT_DELETE));
                    }

                    CATCH_REQUIRE(j.set_file_management(value));
                    CATCH_REQUIRE(j.get_file_management() == value);
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
                    std::uint32_t value(0);
                    do
                    {
                        value = rand();
                    }
                    while(value == prinbee::JOURNAL_DEFAULT_EVENTS);
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
                    std::uint32_t value;
                    do
                    {
                        value = rand() + 1;
                    }
                    while(value == prinbee::JOURNAL_DEFAULT_FILE_SIZE);
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
                    // avoid the default (i.e. 2) so the configuration file
                    // gets saved
                    //
                    int const value(rand() % (256 - 3) + 3);
                    CATCH_REQUIRE(j.set_maximum_number_of_files(value));
                    expected_result = std::to_string(value);
                }
                break;

            case FLUSH:
                CATCH_REQUIRE(j.set_sync(prinbee::sync_t::SYNC_FLUSH));
                //CATCH_REQUIRE(j.get_sync() == prinbee::sync_t::SYNC_FLUSH);
                break;

            case SYNC:
                CATCH_REQUIRE(j.set_sync(prinbee::sync_t::SYNC_FULL));
                //CATCH_REQUIRE(j.get_sync() == prinbee::sync_t::SYNC_FULL);
                break;

            case INLINE_ATTACHMENT_SIZE_THRESHOLD:
                {
                    int value(0);
                    do
                    {
                        value = (rand() % (prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_MAXIMUM_THRESHOLD - prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_MINIMUM_THRESHOLD)
                                        + prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_MINIMUM_THRESHOLD);
                    }
                    while(value == prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD);
                    CATCH_REQUIRE(j.set_inline_attachment_size_threshold(value));
                    expected_result = std::to_string(value);
                }
                break;

            case ATTACHMENT_COPY_HANDLING_SOFTLINK:
                // SOFTLINK is the default, to make sure we get a conf file,
                // first set HARDLINK and then switch back
                //
                CATCH_REQUIRE(j.set_attachment_copy_handling(prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_HARDLINK));
                CATCH_REQUIRE(j.set_attachment_copy_handling(prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK));
                CATCH_REQUIRE(j.get_attachment_copy_handling() == prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK);
                break;

            case ATTACHMENT_COPY_HANDLING_HARDLINK:
                CATCH_REQUIRE(j.set_attachment_copy_handling(prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_HARDLINK));
                CATCH_REQUIRE(j.get_attachment_copy_handling() == prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_HARDLINK);
                break;

            case ATTACHMENT_COPY_HANDLING_REFLINK:
                CATCH_REQUIRE(j.set_attachment_copy_handling(prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_REFLINK));
                CATCH_REQUIRE(j.get_attachment_copy_handling() == prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_REFLINK);
                break;

            case ATTACHMENT_COPY_HANDLING_FULL:
                CATCH_REQUIRE(j.set_attachment_copy_handling(prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_FULL));
                CATCH_REQUIRE(j.get_attachment_copy_handling() == prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_FULL);
                break;

            default:
                CATCH_REQUIRE(!"the test is invalid, add another case as required");
                break;

            }

            // load configuration we just updated
            //
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

            it = conf_values.find("inline_attachment_size_threshold");
            CATCH_REQUIRE(it != conf_values.end());
            CATCH_REQUIRE((index == INLINE_ATTACHMENT_SIZE_THRESHOLD
                                        ? expected_result
                                        : std::to_string(prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD)) == it->second);
            conf_values.erase(it);

            it = conf_values.find("attachment_copy_handling");
            CATCH_REQUIRE(it != conf_values.end());
            switch(index)
            {
            case ATTACHMENT_COPY_HANDLING_HARDLINK:
                CATCH_REQUIRE("hardlink" == it->second);
                break;

            case ATTACHMENT_COPY_HANDLING_REFLINK:
                CATCH_REQUIRE("reflink" == it->second);
                break;

            case ATTACHMENT_COPY_HANDLING_FULL:
                CATCH_REQUIRE("full" == it->second);
                break;

            // the default is "softlink"
            case ATTACHMENT_COPY_HANDLING_SOFTLINK:
            default:
                CATCH_REQUIRE("softlink" == it->second);
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

        for(int i(0); i < 256; ++i)
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

    CATCH_START_SECTION("journal_options: invalid attachment copy handling numbers")
    {
        std::string const path(conf_path("journal_options"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());

        for(int i(0); i < 256; ++i)
        {
            switch(static_cast<prinbee::attachment_copy_handling_t>(i))
            {
            case prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_DEFAULT:
            case prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK:
            case prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_HARDLINK:
            case prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_REFLINK:
            case prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_FULL:
                // these are valid, ignore
                break;

            default:
                {
                    std::stringstream ss;
                    ss << "prinbee_exception: unknown attachment_copy_handling_t enumeration type ("
                       << i
                       << ").";
                    CATCH_REQUIRE_THROWS_MATCHES(
                              j.set_attachment_copy_handling(static_cast<prinbee::attachment_copy_handling_t>(i))
                            , prinbee::invalid_parameter
                            , Catch::Matchers::ExceptionMessage(ss.str()));
                }
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
            prinbee::in_event event;
            event.set_request_id(request_id);
            {
                prinbee::attachment a;
                a.set_data(data.data(), data.size());
                event.add_attachment(a);
            }
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
            prinbee::out_event out_event;
            CATCH_REQUIRE_FALSE(j.next_event(out_event));

            j.rewind();
            CATCH_REQUIRE(j.next_event(out_event, true, true));

            std::string const filename(path + "/journal-0.events");
            CATCH_REQUIRE(filename == out_event.get_debug_filename());
            CATCH_REQUIRE(8U == out_event.get_debug_offset());

            CATCH_REQUIRE(request_id == out_event.get_request_id());
            CATCH_REQUIRE(prinbee::status_t::STATUS_READY == out_event.get_status());
            CATCH_REQUIRE(event_time == out_event.get_event_time());

            {
                CATCH_REQUIRE(out_event.get_attachment_size() == 1);
                prinbee::attachment a(out_event.get_attachment(0));
                CATCH_REQUIRE(size == static_cast<std::size_t>(a.size()));
                CATCH_REQUIRE_LONG_STRING(std::string(reinterpret_cast<char const *>(data.data()), data.size())
                                        , std::string(reinterpret_cast<char const *>(a.data()), a.size()));
            }

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
                    prinbee::out_event out_event2;
                    CATCH_REQUIRE(j.next_event(out_event2));

                    // at the moment the debug does not get cleared, so we
                    // used a separate structure to verify that by default
                    // the debug data remains untouched
                    //
                    CATCH_REQUIRE("" == out_event2.get_debug_filename());
                    CATCH_REQUIRE(0 == out_event2.get_debug_offset());

                    CATCH_REQUIRE(request_id == out_event2.get_request_id());

                    {
                        CATCH_REQUIRE(out_event2.get_attachment_size() == 1);
                        prinbee::attachment a(out_event2.get_attachment(0));
                        CATCH_REQUIRE(size == static_cast<std::size_t>(a.size()));
                        CATCH_REQUIRE_LONG_STRING(std::string(reinterpret_cast<char const *>(data.data()), data.size())
                                                , std::string(reinterpret_cast<char const *>(a.data()), a.size()));
                    }

                    if(expect_success)
                    {
                        CATCH_REQUIRE(status == out_event2.get_status());
                        last_success = out_event2.get_status();
                    }
                    else
                    {
                        // on error, it does not change
                        //
                        CATCH_REQUIRE(last_success == out_event2.get_status());
                    }
                    CATCH_REQUIRE(event_time == out_event2.get_event_time());
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
                    prinbee::in_event event;
                    event.set_request_id(prinbee::id_to_string(id));
                    {
                        prinbee::attachment a;
                        a.set_data(data.data(), size);
                        event.add_attachment(a);
                    }

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
                    prinbee::in_event event;
                    event.set_request_id(prinbee::id_to_string(id));
                    {
                        prinbee::attachment a;
                        a.set_data(data.data(), size);
                        event.add_attachment(a);
                    }
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
                    prinbee::in_event event;
                    event.set_request_id(prinbee::id_to_string(id));
                    {
                        prinbee::attachment a;
                        a.set_data(data.data(), size);
                        event.add_attachment(a);
                    }
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
                prinbee::in_event event;
                event.set_request_id(prinbee::id_to_string(ids[r]));
                {
                    prinbee::attachment a;
                    a.set_data(data.data(), size);
                    event.add_attachment(a);
                }
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
                prinbee::out_event event;
                CATCH_REQUIRE(j.next_event(event));
                CATCH_REQUIRE(event_time == event.get_event_time());
                CATCH_REQUIRE(prinbee::id_to_string(ids[r]) == event.get_request_id());
                ++event_time;
            }

            // make sure we reached the end
            //
            {
                prinbee::out_event event;
                CATCH_REQUIRE_FALSE(j.next_event(event));
            }
        }

        {
            prinbee::journal j(path);
            for(int r(0); r < 10; ++r)
            {
                prinbee::out_event event;
                CATCH_REQUIRE(j.next_event(event, false));
                CATCH_REQUIRE(times[r] == event.get_event_time());
                CATCH_REQUIRE(prinbee::id_to_string(r + 1) == event.get_request_id());
            }

            // make sure we reached the end
            //
            {
                prinbee::out_event event;
                CATCH_REQUIRE_FALSE(j.next_event(event, false));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_event_list: fill an event with files & direct data")
    {
        std::string const temp(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/files_of_mixed_test");
        CATCH_REQUIRE(snapdev::mkdir_p(temp) == 0);

        // we want a realpath (a.k.a. absolute path) and a relative path
        //
        std::string error_msg;
        std::string const temp_absolute(snapdev::pathinfo::realpath(temp, error_msg));
        std::string const cwd(snapdev::pathinfo::getcwd(error_msg));
        std::string const temp_relative(snapdev::pathinfo::relative_path(cwd, temp_absolute));

        prinbee::attachment_copy_handling_t const mode[] =
        {
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK,
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_HARDLINK,
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_REFLINK,
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_FULL,
        };

        for(auto const & handling : mode)
        {
            for(int count(0); count < 5; ++count)
            {
                std::string name("journal_event_with_mixed_data-");
                name += std::to_string(count + 1);
                name += '-';
                name += std::to_string(static_cast<int>(handling));
                std::string const path(conf_path(name));

                std::size_t const max(rand() % 100 + 150);
                std::vector<prinbee::data_t> data(max);

                // create one event in a journal with many attachments
                // some of which are direct others will be files
                {
                    advgetopt::conf_file::reset_conf_files();
                    prinbee::journal j(path);
                    CATCH_REQUIRE(j.set_file_management(prinbee::file_management_t::FILE_MANAGEMENT_DELETE));
                    CATCH_REQUIRE(j.set_attachment_copy_handling(handling));
                    CATCH_REQUIRE(j.is_valid());
                    CATCH_REQUIRE(j.empty());

                    // create the event with many attachments
                    //
                    prinbee::in_event event;
                    event.set_request_id(prinbee::id_to_string(count));

                    std::uint16_t select(0);
                    for(std::size_t r(0); r < max; ++r, select >>= 1)
                    {
                        if((r % 16) == 0)
                        {
                            select = rand();
                        }

                        std::size_t const size(rand() % (20 * 1024) + 1);
                        data[r].resize(size);
                        for(std::size_t idx(0); idx < size; ++idx)
                        {
                            data[r][idx] = rand();
                        }

                        prinbee::attachment a;
                        if((select & 1) == 0)
                        {
                            // add as direct data
                            //
                            a.set_data(data[r].data(), size);
                        }
                        else
                        {
                            // add as a file
                            //
                            std::string filename((rand() & 1) == 0 ? temp_absolute : temp_relative);
                            filename += "/in-";
                            filename += std::to_string(count + 1);
                            filename += '-';
                            filename += std::to_string(r + 1);
                            filename += ".data";
                            {
                                std::ofstream out(filename);
                                CATCH_REQUIRE(out.is_open());
                                out.write(reinterpret_cast<char const *>(data[r].data()), size);
                            }
                            a.set_file(filename);
                        }
                        event.add_attachment(a);
                    }

                    snapdev::timespec_ex event_time(snapdev::now());
                    CATCH_REQUIRE(j.add_event(event, event_time));
                    CATCH_REQUIRE(j.size() == 1ULL);
                    CATCH_REQUIRE_FALSE(j.empty());
                }

                // now reload that journal and see that we can retrieve all
                // the attachments as we added above
                {
                    prinbee::journal j(path);

                    prinbee::out_event event;
                    CATCH_REQUIRE(j.next_event(event));
                    CATCH_REQUIRE(prinbee::id_to_string(count) == event.get_request_id());
                    CATCH_REQUIRE(max == event.get_attachment_size());

                    for(std::size_t r(0); r < max; ++r)
                    {
                        prinbee::attachment const a(event.get_attachment(r));
                        void * d(a.data());
                        std::size_t sz(a.size());
                        CATCH_REQUIRE(data[r].size() == sz);
                        CATCH_REQUIRE(memcmp(d, data[r].data(), sz) == 0);
                    }

                    // make sure we reached the end
                    CATCH_REQUIRE_FALSE(j.next_event(event));
                }
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
            prinbee::in_event event;
            event.set_request_id("id-1");
            {
                prinbee::attachment a;
                a.set_data(data.data(), size);
                event.add_attachment(a);
            }
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


CATCH_TEST_CASE("journal_attachement", "[journal][attachment]")
{
    CATCH_START_SECTION("journal_attachment: attachment save_data() makes a copy")
    {
        for(int i(0); i < 100; ++i)
        {
            prinbee::attachment a;

            std::size_t sz(rand() & 1000 + 1);
            std::vector<std::uint8_t> data(sz);
            for(std::size_t idx(0); idx < sz; ++idx)
            {
                data[idx] = rand();
            }
            a.save_data(data.data(), sz);

            // keep a copy
            //
            std::vector<std::uint8_t> copy(data);

            // "mess up original"
            //
            for(std::size_t idx(0); idx < sz; ++idx)
            {
                do
                {
                    data[idx] = rand();
                }
                while(data[idx] == copy[idx]);
            }

            CATCH_REQUIRE(static_cast<off_t>(sz) == a.size());

            void const * p(a.data());
            std::uint8_t const * ptr(reinterpret_cast<std::uint8_t const *>(p));

            for(std::size_t idx(0); idx < sz; ++idx)
            {
                CATCH_REQUIRE(ptr[idx] == copy[idx]);
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_attachment: attachment set_file() saves a filename and reads its data")
    {
        std::string const content("This is the file.\n");
        std::string const path(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/set_file-test-file.txt");
        {
            std::ofstream out(path);
            CATCH_REQUIRE(out.is_open());
            out << content;
        }
        prinbee::attachment a;
        a.set_file(path);
        CATCH_REQUIRE_FALSE(a.empty());
        CATCH_REQUIRE(a.size() == static_cast<off_t>(content.length()));
        CATCH_REQUIRE(a.is_file());
        CATCH_REQUIRE(a.filename() == path);

        // the a.data() will read the file in memory inside the attachment
        // object then we can compare and make sure it is equal to our input
        //
        void const * p(a.data());
        char const * ptr(reinterpret_cast<char const *>(p));
        for(std::size_t idx(0); idx < content.length(); ++idx)
        {
            CATCH_REQUIRE(ptr[idx] == content[idx]);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("journal_errors", "[journal][error]")
{
    CATCH_START_SECTION("journal_errors: trying to re-add the same event multiple times fails")
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
        prinbee::in_event event;
        event.set_request_id("id-123");
        {
            prinbee::attachment a;
            a.set_data(data.data(), size);
            event.add_attachment(a);
        }
        snapdev::timespec_ex event_time(snapdev::now());
        CATCH_REQUIRE(j.add_event(event, event_time));

        // if we try again, it fails
        //
        event_time = snapdev::now();
        CATCH_REQUIRE_FALSE(j.add_event(event, event_time));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: attachment negative size (set_data)")
    {
        prinbee::attachment a;
        std::uint8_t buf[256];

        for(int i(0); i < 100; ++i)
        {
            off_t size(0);
            while(size >= 0)
            {
                size = -rand();
            }
            CATCH_REQUIRE_THROWS_MATCHES(
                      a.set_data(buf, size)
                    , prinbee::invalid_parameter
                    , Catch::Matchers::ExceptionMessage("prinbee_exception: attachment cannot have a negative size."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: attachment invalid size / pointer combo (set_data)")
    {
        prinbee::attachment a;

        for(int i(0); i < 100; ++i)
        {
            off_t size(0);
            while(size <= 0)
            {
                size = rand();
            }
            CATCH_REQUIRE_THROWS_MATCHES(
                      a.set_data(nullptr, size)
                    , prinbee::invalid_parameter
                    , Catch::Matchers::ExceptionMessage("prinbee_exception: attachment with a size > 0 must have a non null data pointer."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: attachment negative size (save_data)")
    {
        prinbee::attachment a;
        std::uint8_t buf[256];

        for(int i(0); i < 100; ++i)
        {
            off_t size(0);
            while(size >= 0)
            {
                size = -rand();
            }
            CATCH_REQUIRE_THROWS_MATCHES(
                      a.save_data(buf, size)
                    , prinbee::invalid_parameter
                    , Catch::Matchers::ExceptionMessage("prinbee_exception: attachment cannot have a negative size."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: attachment invalid size / pointer combo (save_data)")
    {
        prinbee::attachment a;

        for(int i(0); i < 100; ++i)
        {
            off_t size(0);
            while(size <= 0)
            {
                size = rand();
            }
            CATCH_REQUIRE_THROWS_MATCHES(
                      a.save_data(nullptr, size)
                    , prinbee::invalid_parameter
                    , Catch::Matchers::ExceptionMessage("prinbee_exception: attachment with a size > 0 must have a non null data pointer (2)."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: request_id too long")
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
        prinbee::in_event event;
        event.set_request_id("for a request identifier too be way to long here it needs to be some two hundred and fifty six or way more characters which means this is a really long sentence to make it happen and well, since I have a lot of imagination that is really no issue at all, right?");
        {
            prinbee::attachment a;
            a.set_data(data.data(), size);
            event.add_attachment(a);
        }
        snapdev::timespec_ex event_time(snapdev::now());
        CATCH_REQUIRE_FALSE(j.add_event(event, event_time));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: invalid number of files")
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

    CATCH_START_SECTION("journal_errors: missing folder")
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
        prinbee::in_event event;
        event.set_request_id("id-123");
        {
            prinbee::attachment a;
            a.set_data(data.data(), size);
            event.add_attachment(a);
        }
        snapdev::timespec_ex event_time(snapdev::now());
        CATCH_REQUIRE_FALSE(j.add_event(event, event_time));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: filled up journal (small size)")
    {
        std::string const path(conf_path("journal_filled"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());

        j.set_maximum_file_size(prinbee::JOURNAL_MINIMUM_FILE_SIZE);
        j.set_inline_attachment_size_threshold(prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_MAXIMUM_THRESHOLD);

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
            prinbee::in_event event;
            event.set_request_id("id-" + std::to_string(count));
            {
                prinbee::attachment a;
                a.set_data(data.data(), size);
                event.add_attachment(a);
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
            prinbee::in_event event;
            event.set_request_id("id-extra");
            {
                prinbee::attachment a;
                a.set_data(data.data(), data.size());
                event.add_attachment(a);
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

    CATCH_START_SECTION("journal_errors: fail with invalid size as ID is not complete and data is missing")
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
            prinbee::in_event event;
            event.set_request_id("this-id");
            {
                prinbee::attachment a;
                a.set_data(data, sizeof(data));
                event.add_attachment(a);
            }
            snapdev::timespec_ex now(snapdev::now());
            CATCH_REQUIRE(j.add_event(event, now));
            CATCH_REQUIRE(j.size() == 1ULL);
            CATCH_REQUIRE_FALSE(j.empty());
        }

        // open that journal and add a broken header (invalid identifier) 
        {
            std::string const filename(event_filename(path, 0));
            std::ofstream out(filename, std::ios::app | std::ios::binary);
            char data[1];
            std::size_t const size(32 /* == sizeof(header) */
                             + 1 * sizeof(prinbee::attachment_offsets_t)
                             + sizeof("next-id") - 1
                             + sizeof(data));
            CATCH_REQUIRE(size < 256ULL);
            std::uint8_t const header[] = {
                'e', 'v',                               // f_magic
                static_cast<std::uint8_t>(prinbee::status_t::STATUS_READY), // f_status
                sizeof("next-id") - 1,                  // f_request_id_size
                size, 0, 0, 0,                          // f_size
                0, 0, 0, 0, 0, 0, 0, 0,                 // f_time
                0, 0, 0, 0, 0, 0, 0, 0,
                1,                                      // f_attachment_offsets
                0, 0, 0, 0, 0, 0, 0,                    // f_pad[7]
            };
            out.write(reinterpret_cast<char const *>(header), sizeof(header));
            prinbee::attachment_offsets_t const offset(
                      sizeof(header)
                    + 1 * sizeof(prinbee::attachment_offsets_t)     // itself
                    + sizeof("next-id") - 1);
            out.write(reinterpret_cast<char const *>(&offset), sizeof(offset));
            out.write("next", 4);                           // <-- only 4 bytes
        }

        {
            prinbee::journal j(path);
            prinbee::out_event event;

            // we find the first valid event
            //
            CATCH_REQUIRE(j.next_event(event));
            CATCH_REQUIRE("this-id" == event.get_request_id());

            // make sure we reached the end; the second event was invalid
            //
            CATCH_REQUIRE_FALSE(j.next_event(event));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: invalid event date & time")
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
            prinbee::in_event event;
            event.set_request_id("this-id");
            {
                prinbee::attachment a;
                a.set_data(data, sizeof(data));
                event.add_attachment(a);
            }
            snapdev::timespec_ex now(snapdev::now());
            CATCH_REQUIRE(j.add_event(event, now));
            CATCH_REQUIRE(j.size() == 1ULL);
            CATCH_REQUIRE_FALSE(j.empty());

            // trying to add an event in the future fails
            //
            snapdev::timespec_ex soon(snapdev::now());
            soon += snapdev::timespec_ex(100, 0);            // 100 seconds in the future
            event.set_request_id("future");
            CATCH_REQUIRE_FALSE(j.add_event(event, soon));
        }

        // open that journal and add a broken header (invalid date & time) 
        {
            std::string const filename(event_filename(path, 0));
            std::ofstream out(filename, std::ios::app | std::ios::binary);
            snapdev::timespec_ex soon(snapdev::now());
            soon += snapdev::timespec_ex(100, 0);            // 100 seconds in the future
            char data[32]; // content not used by the test, no need to initialized
            std::size_t const size(32 /* == sizeof(header)*/
                                 + 1 * sizeof(prinbee::attachment_offsets_t)
                                 + sizeof("next-id") - 1
                                 + sizeof(data));
            CATCH_REQUIRE(size < 256ULL);
            std::uint8_t const header[] = {
                'e', 'v',                               // f_magic
                static_cast<std::uint8_t>(prinbee::status_t::STATUS_READY), // f_status
                sizeof("next-id") - 1,                  // f_request_id_size
                size, 0, 0, 0,                          // f_size
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
                1,                                      // f_attachment_count
                0, 0, 0, 0, 0, 0, 0,                    // f_pad[7]
            };
            out.write(reinterpret_cast<char const *>(header), sizeof(header));
            prinbee::attachment_offsets_t const offset(
                      sizeof(header)
                    + 1 * sizeof(prinbee::attachment_offsets_t)     // itself
                    + sizeof("next-id") - 1);
            out.write(reinterpret_cast<char const *>(&offset), sizeof(offset));
            out.write("next-id", sizeof("next-id") - 1);
            out.write(data, sizeof(data));
        }

        {
            prinbee::journal j(path);
            prinbee::out_event event;

            // we find the first valid event
            //
            CATCH_REQUIRE(j.next_event(event));
            CATCH_REQUIRE("this-id" == event.get_request_id());

            // make sure we reached the end; the second event was invalid
            //
            CATCH_REQUIRE_FALSE(j.next_event(event));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: invalid end marker")
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
                prinbee::in_event event;
                event.set_request_id("this-id");
                {
                    prinbee::attachment a;
                    a.set_data(data, sizeof(data));
                    event.add_attachment(a);
                }
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
                std::size_t const size(32 /* == sizeof(header) */
                                 + 1 * sizeof(prinbee::attachment_offsets_t)
                                 + sizeof("next-id") - 1
                                 + sizeof(data));
                CATCH_REQUIRE(size < 256ULL);
                std::uint8_t const header[] = {
                    static_cast<std::uint8_t>(bad_marker.a), // f_magic
                    static_cast<std::uint8_t>(bad_marker.b),
                    static_cast<std::uint8_t>(prinbee::status_t::STATUS_READY), // f_status
                    sizeof("next-id") - 1,                  // f_request_id_size
                    size, 0, 0, 0,                          // f_size
                    0, 0, 0, 0, 0, 0, 0, 0,                 // f_time
                    0, 0, 0, 0, 0, 0, 0, 0,
                    1,                                      // f_attachment_offsets
                    0, 0, 0, 0, 0, 0, 0,                    // f_pad[7]
                };
                out.write(reinterpret_cast<char const *>(header), sizeof(header));
                prinbee::attachment_offsets_t const offset(
                          sizeof(header)
                        + 1 * sizeof(prinbee::attachment_offsets_t)     // itself
                        + sizeof("next-id") - 1);
                out.write(reinterpret_cast<char const *>(&offset), sizeof(offset));
                out.write("next-id", sizeof("next-id") - 1);
                out.write(data, sizeof(data));
            }

            {
                prinbee::journal j(path);
                prinbee::out_event event;

                // we find the first valid event
                //
                CATCH_REQUIRE(j.next_event(event));
                CATCH_REQUIRE("this-id" == event.get_request_id());

                // make sure we reached the end; the second event was invalid
                //
                CATCH_REQUIRE_FALSE(j.next_event(event));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: incomplete header")
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
                prinbee::in_event event;
                event.set_request_id("this-id");
                {
                    prinbee::attachment a;
                    a.set_data(data, sizeof(data));
                    event.add_attachment(a);
                };
                snapdev::timespec_ex now(snapdev::now());
                CATCH_REQUIRE(j.add_event(event, now));
                CATCH_REQUIRE(j.size() == 1ULL);
                CATCH_REQUIRE_FALSE(j.empty());
            }

            // create a broken header (too small by 1 or more bytes)
            {
                std::string const filename(event_filename(path, 0));
                std::ofstream out(filename, std::ios::app | std::ios::binary);
                char data[1];
                std::size_t const size(32 /* == sizeof(header) */
                                 + 1 * sizeof(prinbee::attachment_offsets_t)
                                 + sizeof("next-id") - 1
                                 + sizeof(data));
                CATCH_REQUIRE(size < 256ULL);
                std::uint8_t const header[] = {
                    'e', 'v',                               // f_magic
                    static_cast<std::uint8_t>(prinbee::status_t::STATUS_READY), // f_status
                    sizeof("next-id") - 1,                  // f_request_id_size
                    size, 0, 0, 0,                          // f_size
                    0, 0, 0, 0, 0, 0, 0, 0,                 // f_time
                    0, 0, 0, 0, 0, 0, 0, 0,
                    1,                                      // f_attachment_offsets
                    0, 0, 0, 0, 0, 0, 0,                    // f_pad[7]
                };
                std::size_t const bad_size(rand() % (sizeof(header) - 1) + 1);
                out.write(reinterpret_cast<char const *>(header), bad_size);
            }

            {
                prinbee::journal j(path);
                prinbee::out_event event;

                // we find the first valid event
                //
                CATCH_REQUIRE(j.next_event(event));
                CATCH_REQUIRE("this-id" == event.get_request_id());

                // make sure we reached the end; the second event was invalid
                // note: in this case we do not get an error message
                //
                CATCH_REQUIRE_FALSE(j.next_event(event));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: invalid magic (start of file header magic tampered)")
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
                prinbee::in_event event;
                event.set_request_id("this-id");
                {
                    prinbee::attachment a;
                    a.set_data(data, sizeof(data));
                    event.add_attachment(a);
                }
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
                prinbee::out_event event;

                // we find no events
                //
                CATCH_REQUIRE_FALSE(j.next_event(event));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: short magic (start of file header)")
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
                prinbee::in_event event;
                event.set_request_id("this-id");
                {
                    prinbee::attachment a;
                    a.set_data(data, sizeof(data));
                    event.add_attachment(a);
                };
                snapdev::timespec_ex now(snapdev::now());
                CATCH_REQUIRE(j.add_event(event, now));
                CATCH_REQUIRE(j.size() == 1ULL);
                CATCH_REQUIRE_FALSE(j.empty());
            }

            // truncate the header to `size` bytes
            {
                std::string const filename(event_filename(path, 0));
                CATCH_REQUIRE(truncate(filename.c_str(), size) == 0);
            }

            {
                prinbee::journal j(path);
                prinbee::out_event event;

                // we find no events
                //
                CATCH_REQUIRE_FALSE(j.next_event(event));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: invalid out event statuses")
    {
        for(int idx(0); idx < 256; ++idx)
        {
            switch(static_cast<prinbee::status_t>(idx))
            {
            case prinbee::status_t::STATUS_UNKNOWN:
            case prinbee::status_t::STATUS_READY:
            case prinbee::status_t::STATUS_FORWARDED:
            case prinbee::status_t::STATUS_ACKNOWLEDGED:
            case prinbee::status_t::STATUS_COMPLETED:
            case prinbee::status_t::STATUS_FAILED:
                continue;

            default:
                break;

            }

            prinbee::out_event event;
            CATCH_REQUIRE_THROWS_MATCHES(
                      event.set_status(static_cast<prinbee::status_t>(idx))
                    , prinbee::invalid_parameter
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: unsupported status number."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: can't reduce number of files in a filled up journal")
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
            prinbee::in_event event;
            event.set_request_id("id-" + std::to_string(count));
            {
                prinbee::attachment a;
                a.set_data(data.data(), size);
                event.add_attachment(a);
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

    CATCH_START_SECTION("journal_errors: source file missing")
    {
        prinbee::attachment a;
        std::string const path(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/this-does-not-exist.txt");
        CATCH_REQUIRE_THROWS_MATCHES(
                  a.set_file(path)
                , prinbee::file_not_found
                , Catch::Matchers::ExceptionMessage(
                            std::string("prinbee_exception: file \"")
                          + path
                          + "\" not accessible: No such file or directory."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: file size mismatch")
    {
        prinbee::attachment a;
        std::size_t real_size(0);
        std::string const path(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/some-file.txt");
        {
            std::ofstream out(path);
            CATCH_REQUIRE(out.is_open());
            out << "This small file.\n";
            real_size = out.tellp();
        }
        CATCH_REQUIRE_THROWS_MATCHES(
                  a.set_file(path, 100)
                , prinbee::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: trying to save more data (100) than available in file attachment \""
                          + path
                          + "\" ("
                          + std::to_string(real_size)
                          + ")."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: delete attachment file then try to read the data")
    {
        std::string content("File about to be deleted.\n");
        std::string const path(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/set_file-unlink-file.txt");
        {
            std::ofstream out(path);
            CATCH_REQUIRE(out.is_open());
            out << content;
        }
        prinbee::attachment a;
        a.set_file(path);
        CATCH_REQUIRE_FALSE(a.empty());
        CATCH_REQUIRE(a.size() == static_cast<off_t>(content.length()));
        CATCH_REQUIRE(a.is_file());
        CATCH_REQUIRE(a.filename() == path);

        // deleting the file before call a.data() means we get an error
        //
        CATCH_REQUIRE(unlink(path.c_str()) == 0);
        CATCH_REQUIRE_THROWS_MATCHES(
                  a.data()
                , prinbee::file_not_found
                , Catch::Matchers::ExceptionMessage(
                            "prinbee_exception: file \""
                          + path
                          + "\" not found or permission denied."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: delete small attachment file before adding event to journal")
    {
        std::string const path(conf_path("journal_small_attachment"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);

        std::string content("Another file about to be deleted.\n");
        std::string const to_unlink(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/set_file-add_event-unlink-file.txt");
        {
            std::ofstream out(to_unlink);
            CATCH_REQUIRE(out.is_open());
            out << content;
        }
        prinbee::attachment a;
        a.set_file(to_unlink);
        CATCH_REQUIRE_FALSE(a.empty());
        CATCH_REQUIRE(a.size() == static_cast<off_t>(content.length()));
        CATCH_REQUIRE(a.is_file());
        CATCH_REQUIRE(a.filename() == to_unlink);

        prinbee::in_event event;
        event.set_request_id("unlinked");
        event.add_attachment(a);

        // deleting the file before calling j.add_event()
        //
        CATCH_REQUIRE(unlink(to_unlink.c_str()) == 0);

        // the add fails as a result
        //
        snapdev::timespec_ex event_time(snapdev::now());
        CATCH_REQUIRE_FALSE(j.add_event(event, event_time));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: delete large attachment file before adding event to journal")
    {
        prinbee::attachment_copy_handling_t const mode[] =
        {
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK,
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_HARDLINK,
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_REFLINK,
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_FULL,
        };

        for(auto const & handling : mode)
        {
            std::string const path(conf_path("journal_large_attachment"));
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            j.set_attachment_copy_handling(handling);

            // create a large string so we go through the large file case
            //
            std::string const content(SNAP_CATCH2_NAMESPACE::random_string(
                    prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD,
                    prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD * 2,
                    SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_ZUNICODE));
            std::string const to_unlink(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/set_file-add_event-unlink-file.txt");
            {
                std::ofstream out(to_unlink);
                CATCH_REQUIRE(out.is_open());
                out << content;
            }
            prinbee::attachment a;
            a.set_file(to_unlink);
            CATCH_REQUIRE_FALSE(a.empty());
            CATCH_REQUIRE(a.size() == static_cast<off_t>(content.length()));
            CATCH_REQUIRE(a.is_file());
            CATCH_REQUIRE(a.filename() == to_unlink);

            prinbee::in_event event;
            event.set_request_id("unlinked");
            event.add_attachment(a);

            // deleting the file before calling j.add_event()
            //
            CATCH_REQUIRE(unlink(to_unlink.c_str()) == 0);

            // the add fails as a result
            //
            snapdev::timespec_ex event_time(snapdev::now());
            if(handling == prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK)
            {
                // softlink does not require access to the original file so
                // the test passes in this case (oops?)
                //
                CATCH_REQUIRE(j.add_event(event, event_time));
            }
            else
            {
                CATCH_REQUIRE_FALSE(j.add_event(event, event_time));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: large attachment file destination is a directory")
    {
        prinbee::attachment_copy_handling_t const mode[] =
        {
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_SOFTLINK,
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_HARDLINK,
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_REFLINK,
            prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_FULL,
        };

        prinbee::attachment_offsets_t id(0);
        for(auto const & handling : mode)
        {
            std::string const path(conf_path("journal_attachment_to_directory"));
            advgetopt::conf_file::reset_conf_files();
            prinbee::journal j(path);
            j.set_attachment_copy_handling(handling);

            // create a large string so we go through the large file case
            //
            std::string const content(SNAP_CATCH2_NAMESPACE::random_string(
                    prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD,
                    prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD * 2,
                    SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_ZUNICODE));
            std::string const to_unlink(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/set_file-add_event-unlink-file.txt");
            {
                std::ofstream out(to_unlink);
                CATCH_REQUIRE(out.is_open());
                out << content;
            }
            prinbee::attachment a;
            a.set_file(to_unlink);
            CATCH_REQUIRE_FALSE(a.empty());
            CATCH_REQUIRE(a.size() == static_cast<off_t>(content.length()));
            CATCH_REQUIRE(a.is_file());
            CATCH_REQUIRE(a.filename() == to_unlink);

            prinbee::in_event event;
            event.set_request_id("directory_as_destination");
            event.add_attachment(a);

            // create a directory preventing creation of destination file
            //
            // note: we use the same directory so the sequence counter will
            // continue to increase instead of using 1.bin each time
            //
            ++id;
            std::string const dirname(path + "/" + std::to_string(id) + ".bin");
            CATCH_REQUIRE(snapdev::mkdir_p(dirname.c_str()) == 0);

            // the add fails as a result
            //
            snapdev::timespec_ex event_time(snapdev::now());
            CATCH_REQUIRE_FALSE(j.add_event(event, event_time));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: large attachment buffer destination is a directory")
    {
        std::string const path(conf_path("journal_large_buffer_attachment_to_directory"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);

        // create a large string so we go through the large file case
        //
        std::string const content(SNAP_CATCH2_NAMESPACE::random_string(
                prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD,
                prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD * 2,
                SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_ZUNICODE));
        prinbee::attachment a;
        a.set_data(const_cast<char *>(content.data()), content.size());
        CATCH_REQUIRE_FALSE(a.empty());
        CATCH_REQUIRE(a.size() == static_cast<off_t>(content.length()));
        CATCH_REQUIRE_FALSE(a.is_file());
        CATCH_REQUIRE(a.filename() == "");

        prinbee::in_event event;
        event.set_request_id("directory_as_destination");
        event.add_attachment(a);

        // create a directory preventing creation of destination file
        //
        std::string const dirname(path + "/1.bin");
        CATCH_REQUIRE(snapdev::mkdir_p(dirname.c_str()) == 0);

        // the add fails as a result
        //
        snapdev::timespec_ex event_time(snapdev::now());
        CATCH_REQUIRE_FALSE(j.add_event(event, event_time));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: large attachment file shorten before added to journal in FULL copy mode")
    {
        std::string const path(conf_path("journal_shorten_large_attachment"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        j.set_attachment_copy_handling(prinbee::attachment_copy_handling_t::ATTACHMENT_COPY_HANDLING_FULL);

        // create a large string so we go through the large file case
        //
        std::string const content(SNAP_CATCH2_NAMESPACE::random_string(
                prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD,
                prinbee::JOURNAL_INLINE_ATTACHMENT_SIZE_DEFAULT_THRESHOLD * 2,
                SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_ZUNICODE));
        std::string const to_unlink(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/set_file-add_event-unlink-file.txt");
        {
            std::ofstream out(to_unlink);
            CATCH_REQUIRE(out.is_open());
            out << content;
        }
        prinbee::attachment a;
        a.set_file(to_unlink);
        CATCH_REQUIRE_FALSE(a.empty());
        CATCH_REQUIRE(a.size() == static_cast<off_t>(content.length()));
        CATCH_REQUIRE(a.is_file());
        CATCH_REQUIRE(a.filename() == to_unlink);

        prinbee::in_event event;
        event.set_request_id("shorten");
        event.add_attachment(a);

        // shortening the file before calling j.add_event()
        //
        CATCH_REQUIRE(truncate(to_unlink.c_str(), content.length() / 2) == 0);

        // the add fails as a result
        //
        snapdev::timespec_ex event_time(snapdev::now());
        CATCH_REQUIRE_FALSE(j.add_event(event, event_time));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: special file cannot be used")
    {
        prinbee::attachment a;
        CATCH_REQUIRE_THROWS_MATCHES(
              a.set_file("/dev/null")
            , prinbee::invalid_parameter
            , Catch::Matchers::ExceptionMessage(
                        "prinbee_exception: file \"/dev/null\" does not represent a regular file."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: directories cannot be used")
    {
        prinbee::attachment a;
        CATCH_REQUIRE_THROWS_MATCHES(
              a.set_file("/usr/bin")
            , prinbee::invalid_parameter
            , Catch::Matchers::ExceptionMessage(
                        "prinbee_exception: file \"/usr/bin\" does not represent a regular file."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: add too many attachments (in)")
    {
        // create a journal
        //
        std::string const path(conf_path("journal_events"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        prinbee::in_event event;

        // add the maximum number of attachments
        //
        for(std::size_t count(0); count < prinbee::MAXIMUM_ATTACHMENT_COUNT; ++count)
        {
            for(std::size_t id(count); id < prinbee::MAXIMUM_ATTACHMENT_COUNT; ++id)
            {
                CATCH_REQUIRE_THROWS_MATCHES(
                          event.get_attachment(id)
                        , prinbee::out_of_range
                        , Catch::Matchers::ExceptionMessage(
                                    "out_of_range: identifier out of range retrieving attachment from in_event."));
            }

            prinbee::attachment a;
            std::size_t const size(rand() % 25 + 1);
            std::vector<std::uint8_t> data(size);
            for(std::size_t idx(0); idx < size; ++idx)
            {
                data[idx] = static_cast<std::uint8_t>(rand());
            }
            a.set_data(data.data(), size);
            event.add_attachment(a);
        }

        // try to add one more attachment, that must fail
        //
        {
            prinbee::attachment a;
            std::size_t const size(rand() % 25 + 1);
            std::vector<std::uint8_t> data(size);
            for(std::size_t idx(0); idx < size; ++idx)
            {
                data[idx] = static_cast<std::uint8_t>(rand());
            }
            a.set_data(data.data(), size);

            CATCH_REQUIRE_THROWS_MATCHES(
                      event.add_attachment(a)
                    , prinbee::full
                    , Catch::Matchers::ExceptionMessage(
                                "prinbee_exception: attachment table is full, this attachment cannot be added (in_event)."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("journal_errors: add too many attachments (out)")
    {
        // create a journal
        //
        std::string const path(conf_path("journal_events"));
        advgetopt::conf_file::reset_conf_files();
        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        prinbee::out_event event;

        // add the maximum number of attachments
        //
        for(std::size_t count(0); count < prinbee::MAXIMUM_ATTACHMENT_COUNT; ++count)
        {
            for(std::size_t id(count); id < prinbee::MAXIMUM_ATTACHMENT_COUNT; ++id)
            {
                CATCH_REQUIRE_THROWS_MATCHES(
                          event.get_attachment(id)
                        , prinbee::out_of_range
                        , Catch::Matchers::ExceptionMessage(
                                    "out_of_range: identifier out of range retrieving attachment from out_event."));
            }

            prinbee::attachment a;
            std::size_t const size(rand() % 25 + 1);
            std::vector<std::uint8_t> data(size);
            for(std::size_t idx(0); idx < size; ++idx)
            {
                data[idx] = static_cast<std::uint8_t>(rand());
            }
            a.set_data(data.data(), size);
            event.add_attachment(a);
        }

        // try to add one more attachment, that must fail
        //
        {
            prinbee::attachment a;
            std::size_t const size(rand() % 25 + 1);
            std::vector<std::uint8_t> data(size);
            for(std::size_t idx(0); idx < size; ++idx)
            {
                data[idx] = static_cast<std::uint8_t>(rand());
            }
            a.set_data(data.data(), size);

            CATCH_REQUIRE_THROWS_MATCHES(
                      event.add_attachment(a)
                    , prinbee::full
                    , Catch::Matchers::ExceptionMessage(
                                "prinbee_exception: attachment table is full, this attachment cannot be added (out_event)."));
        }
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
