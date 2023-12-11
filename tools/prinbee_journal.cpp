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
 * \brief Journal Utility.
 *
 * This tool allows you to read the events in a journal.
 */

// prinbee
//
#include    "prinbee/journal/journal.h"
#include    "prinbee/version.h"

//#include    "prinbee/exception.h"


// advgetopt
//
#include    <advgetopt/advgetopt.h>
#include    <advgetopt/conf_file.h>
#include    <advgetopt/exception.h>
#include    <advgetopt/options.h>


// libexcept
//
#include    <libexcept/file_inheritance.h>
#include    <libexcept/report_signal.h>


// snapdev
//
#include    <snapdev/stringize.h>


// last include
//
#include    <snapdev/poison.h>



namespace
{



const advgetopt::option g_options[] =
{
    advgetopt::define_option(
          advgetopt::Name("list")
        , advgetopt::ShortName('l')
        , advgetopt::Flags(advgetopt::standalone_command_flags<
              advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("list the events, do not show their content.")
    ),
    advgetopt::define_option(
          advgetopt::Name("text")
        , advgetopt::ShortName('t')
        , advgetopt::Flags(advgetopt::all_flags<
              advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("assume events are can be printed as is in your console.")
    ),
    advgetopt::define_option(
          advgetopt::Name("--")
        , advgetopt::Flags(advgetopt::all_flags<
              advgetopt::GETOPT_FLAG_REQUIRED
            , advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("path to journal environment.")
    ),
    advgetopt::end_options()
};

advgetopt::group_description const g_group_descriptions[] =
{
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
        , advgetopt::GroupName("command")
        , advgetopt::GroupDescription("Commands:")
    ),
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
        , advgetopt::GroupName("option")
        , advgetopt::GroupDescription("Options:")
    ),
    advgetopt::end_groups()
};

constexpr char const * const g_configuration_files[] =
{
    "/etc/prinbee/prinbee-journal.conf",
    nullptr
};

// TODO: once we have stdc++20, remove all defaults & pragma
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wpedantic"
advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "prinbee-journal",
    .f_group_name = "prinbee",
    .f_options = g_options,
    .f_options_files_directory = nullptr,
    .f_environment_variable_name = "PRINBEE_JOURNAL",
    .f_environment_variable_intro = "PRINBEE_JOURNAL_",
    .f_section_variables_name = nullptr,
    .f_configuration_files = g_configuration_files,
    .f_configuration_filename = nullptr,
    .f_configuration_directories = nullptr,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [-<opt>]\n"
                     "where -<opt> is one or more of:",
    .f_help_footer = "Try `man prinbee-journal` for more info.\n%c",
    .f_version = PRINBEE_VERSION_STRING,
    .f_license = "GPL v3 or newer",
    .f_copyright = "Copyright (c) 2023-"
                   SNAPDEV_STRINGIZE(UTC_BUILD_YEAR)
                   "  Made to Order Software Corporation",
    .f_build_date = UTC_BUILD_DATE,
    .f_build_time = UTC_BUILD_TIME,
    .f_groups = g_group_descriptions
};
//#pragma GCC diagnostic pop





class prinbee_journal
{
public:
                            prinbee_journal(int argc, char * argv[]);

    int                     run();

private:
    int                     get_path();
    int                     load_journal();
    int                     scan_journal();

    advgetopt::getopt       f_opt;
    std::string             f_path = std::string();
    prinbee::journal::pointer_t
                            f_journal = prinbee::journal::pointer_t();
};


prinbee_journal::prinbee_journal(int argc, char * argv[])
    : f_opt(g_options_environment, argc, argv)
{
}


int prinbee_journal::run()
{
    int r(0);

    r = get_path();
    if(r != 0)
    {
        return r;
    }

    r = load_journal();
    if(r != 0)
    {
        return r;
    }

    r = scan_journal();
    if(r != 0)
    {
        return r;
    }

    return 0;
}


int prinbee_journal::get_path()
{
    if(!f_opt.is_defined("--"))
    {
        std::cerr << "error: a journal <path> is required.\n";
        return 1;
    }

    f_path = f_opt.get_string("--");
    if(f_path.empty())
    {
        std::cerr << "error: <path> cannot be an empty string.\n";
        return 1;
    }

    return 0;
}


int prinbee_journal::load_journal()
{
    f_journal = std::make_shared<prinbee::journal>(f_path);
    if(!f_journal->is_valid())
    {
        std::cerr
            << "error: could not load journal at \""
            << f_path
            << "\".\n";
        return 1;
    }

    return 0;
}


int prinbee_journal::scan_journal()
{
    bool const text(f_opt.is_defined("text"));
    bool const list(f_opt.is_defined("list"));

    prinbee::out_event_t event;
    while(f_journal->next_event(event))
    {
        std::cout << "Event: " << event.f_request_id << '\n';

        std::cout << "Status: ";
        switch(event.f_status)
        {
        case prinbee::status_t::STATUS_UNKNOWN:
            std::cout << "Unknown\n";
            break;

        case prinbee::status_t::STATUS_READY:
            std::cout << "Ready\n";
            break;

        case prinbee::status_t::STATUS_FORWARDED:
            std::cout << "Forwarded\n";
            break;

        case prinbee::status_t::STATUS_ACKNOWLEDGED:
            std::cout << "Acknowledged\n";
            break;

        case prinbee::status_t::STATUS_COMPLETED:
            std::cout << "Completed\n";
            break;

        case prinbee::status_t::STATUS_FAILED:
            std::cout << "Failed\n";
            break;

        default:
            std::cout << "<unknown status:" << static_cast<int>(event.f_status) << '\n';
            break;

        }

        std::cout << "Event Time: " << event.f_event_time.to_string("%Y/%m/%d %T.%N") << '\n';

        if(!list)
        {
            if(text)
            {
                std::cout << "Event: " << reinterpret_cast<char const *>(event.f_data.data()) << '\n';
            }
            else
            {
                auto show_ascii = [](std::uint8_t const * data, std::size_t len)
                {
                    for(std::size_t idx(0); idx < len; ++idx)
                    {
                        char const c(static_cast<char>(data[idx]));
                        if(c < ' ' || c > '~')
                        {
                            std::cout << '.';
                        }
                        else
                        {
                            std::cout << c;
                        }
                    }
                };

                char const * indent = "Event:";
                std::cout << std::hex;
                std::size_t const size(event.f_data.size());
                for(std::size_t idx(0); idx < size; ++idx)
                {
                    std::size_t const edge(idx & 0xF);
                    if(edge == 0)
                    {
                        if(idx != 0)
                        {
                            show_ascii(event.f_data.data() + idx - 16, 16);
                            std::cout << '\n';
                        }
                        std::cout << indent;
                    }
                    else if(edge == 7)
                    {
                        std::cout << ' ';
                    }
                    std::cout << ' ' << std::setw(2) << std::setfill('0') << event.f_data[idx];
                }
                std::size_t last_few(size & 0xF);
                if(last_few == 0 && size > 0)
                {
                    last_few = 16;
                }
                if(last_few > 0)
                {
                    show_ascii(event.f_data.data() + size - last_few, last_few);
                    std::cout << '\n';
                }
                std::cout << std::dec;
            }
        }
    }

    return 0;
}



}
// noname namespace



int main(int argc, char * argv[])
{
    libexcept::init_report_signal();
    libexcept::verify_inherited_files();

    try
    {
        prinbee_journal j(argc, argv);
        return j.run();
    }
    catch(advgetopt::getopt_exit const & e)
    {
        return e.code();
    }
    catch(libexcept::exception_t const & e)
    {
        std::cerr
            << "error: a libexcept exception occurred: \""
            << e.what()
            << "\".\n";
    }
    catch(std::exception const & e)
    {
        std::cerr
            << "error: a standard exception occurred: \""
            << e.what()
            << "\".\n";
    }
    catch(...)
    {
        std::cerr << "error: an unknown exception occurred.\n";
    }

    return 1;
}



// vim: ts=4 sw=4 et