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


/** \file
 * \brief Compute CRC16 of some input data.
 *
 * This tool allows us to compute CRC16 of various input data. Either
 * directly on the command line or from a file.
 *
 * It can also be used to verify that a file is valid, assuming you
 * have its CRC16 saved somewhere.
 */

// prinbee
//
#include    "prinbee/network/crc16.h"
#include    "prinbee/version.h"

//#include    "prinbee/exception.h"


// advgetopt
//
#include    <advgetopt/advgetopt.h>
//#include    <advgetopt/conf_file.h>
#include    <advgetopt/exception.h>
#include    <advgetopt/options.h>


// libexcept
//
#include    <libexcept/file_inheritance.h>
#include    <libexcept/report_signal.h>


// libutf8
//
//#include    <libutf8/libutf8.h>


// snapdev
//
#include    <snapdev/file_contents.h>
#include    <snapdev/stringize.h>


// ICU
//
//#include    <unicode/locid.h>


// C++
//
//#include    <random>
#include    <iomanip>
#include    <iostream>


// last include
//
#include    <snapdev/poison.h>



namespace
{



const advgetopt::option g_options[] =
{
    advgetopt::define_option(
          advgetopt::Name("hex")
        , advgetopt::Flags(advgetopt::all_flags<
              advgetopt::GETOPT_FLAG_GROUP_COMMANDS
            , advgetopt::GETOPT_FLAG_MULTIPLE>())
        , advgetopt::Help("compute the CRC16 of the specified hexadecimal numbers.")
    ),
    advgetopt::define_option(
          advgetopt::Name("file")
        , advgetopt::ShortName('f')
        , advgetopt::Flags(advgetopt::all_flags<
              advgetopt::GETOPT_FLAG_GROUP_COMMANDS
            , advgetopt::GETOPT_FLAG_MULTIPLE>())
        , advgetopt::Help("compute the CRC16 of the specified input file.")
    ),
    advgetopt::define_option(
          advgetopt::Name("verbose")
        , advgetopt::ShortName('v')
        , advgetopt::Flags(advgetopt::standalone_command_flags<
              advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("make the tool verbose about its work.")
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

advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "crc16",
    .f_group_name = "prinbee",
    .f_options = g_options,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [-<opt>]\n"
                     "where -<opt> is one or more of:",
    .f_help_footer = "Try `man crc16` for more info.\n%c",
    .f_version = PRINBEE_VERSION_STRING,
    .f_license = "GPL v3 or newer",
    .f_copyright = "Copyright (c) 2025-"
                   SNAPDEV_STRINGIZE(UTC_BUILD_YEAR)
                   "  Made to Order Software Corporation",
    .f_build_date = UTC_BUILD_DATE,
    .f_build_time = UTC_BUILD_TIME,
    .f_groups = g_group_descriptions
};





class crc16_manager
{
public:
                                crc16_manager(int argc, char * argv[]);

    int                         run();

private:
    int                         handle_hex();
    int                         handle_file();

    advgetopt::getopt           f_opts;
    bool                        f_verbose = false;
};


crc16_manager::crc16_manager(int argc, char * argv[])
    : f_opts(g_options_environment, argc, argv)
{
    f_verbose = f_opts.is_defined("verbose");
}


int crc16_manager::run()
{
    if(f_opts.is_defined("hex"))
    {
        if(f_opts.is_defined("file"))
        {
            std::cerr << "error: --hex and --file are mutually exclusive.\n";
            return 1;
        }
        return handle_hex();
    }

    if(f_opts.is_defined("file"))
    {
        return handle_file();
    }

    std::cerr << "error: one of --hex or --file must be specified.\n";
    return 1;
}


int crc16_manager::handle_hex()
{
    std::vector<std::uint8_t> data;
    std::size_t const max(f_opts.size("hex"));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        std::uint8_t value(f_opts.get_long("hex", idx, -128, 255));
        data.push_back(value);
    }
    prinbee::crc16_t const result(prinbee::crc16_compute(data.data(), data.size()));
    std::cout << std::hex << std::setfill('0')
        << std::setw(2) << static_cast<int>(result & 0xFF)
        << ' '
        << std::setw(2) << static_cast<int>((result >> 8) & 0xFF)
        << '\n';
    return 0;
}


int crc16_manager::handle_file()
{
    std::size_t const max(f_opts.size("file"));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        snapdev::file_contents in(f_opts.get_string("file", idx));
        in.read_all();
        std::string const & data(in.contents());
        prinbee::crc16_t const result(prinbee::crc16_compute(reinterpret_cast<std::uint8_t const *>(data.data()), data.length()));
        if(max != 1)
        {
            std::cout << in.filename() << ": ";
        }
        std::cout << std::hex << std::setfill('0')
            << std::setw(2) << static_cast<int>(result & 0xFF)
            << ' '
            << std::setw(2) << static_cast<int>((result >> 8) & 0xFF)
            << '\n';
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
        crc16_manager m(argc, argv);
        return m.run();
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
