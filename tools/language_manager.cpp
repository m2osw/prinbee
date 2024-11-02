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
 * \brief Language Manager tool.
 *
 * Prinbee has the ability to manage any number of versions for one key.
 * In our "content" table, one key represents a page on a website. That
 * page may include translations and updates.
 *
 * The updates are versions (1.0, 1.1, 1.2, 2.0, 2.1, 2.2 2.3, 2.4, etc.).
 * This works in a way very similar to a source repository like git.
 *
 * As for the translations are distinguished using a language tag such as
 * "fr-TD". However, in the database, we want to use a 16 bit identifier.
 * An identifier that cannot change because otherwise restoring a database
 * may not be possible. The 16 bit identifier is important to allow 
 * sequencial tables.
 *
 * This tool is used to create new entries and manage existing entries.
 * The idea is that each entry is assigned a unique identifier. When
 * updating, that identifier is never changed. When creating a new entry,
 * we use a new unique identifier.
 *
 * The tool works in steps as follow:
 *
 * 1. load the Prinbee language file; this file must exist, but it can be empty
 * 2. load the Unicode locales
 * 3. update the Prinbee language file
 * 4. save the Prinbee language file
 */

// prinbee
//
#include    "prinbee/data/language.h"
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


// ICU
//
#include    <unicode/locid.h>


// last include
//
#include    <snapdev/poison.h>



namespace
{



const advgetopt::option g_options[] =
{
    advgetopt::define_option(
          advgetopt::Name("create")
        , advgetopt::ShortName('c')
        , advgetopt::Flags(advgetopt::standalone_command_flags<
              advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("ignore the existing language file.")
    ),
    advgetopt::define_option(
          advgetopt::Name("list")
        , advgetopt::ShortName('l')
        , advgetopt::Flags(advgetopt::standalone_command_flags<
              advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("list the languages as Prinbee sees them.")
    ),
    advgetopt::define_option(
          advgetopt::Name("list-available")
        , advgetopt::ShortName('L')
        , advgetopt::Flags(advgetopt::standalone_command_flags<
              advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("list the languages found on this system using the Unicode C++ library.")
    ),
    advgetopt::define_option(
          advgetopt::Name("file")
        , advgetopt::ShortName('f')
        , advgetopt::Flags(advgetopt::all_flags<
              advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("define the path and filename of the .ini file where the Prinbee languages get saved.")
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
    "/etc/prinbee/language-manager.conf",
    nullptr
};

// TODO: once we have stdc++20, remove all defaults & pragma
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wpedantic"
advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "language-manager",
    .f_group_name = "prinbee",
    .f_options = g_options,
    .f_options_files_directory = nullptr,
    .f_environment_variable_name = "PRINBEE_LANGUAGE_MANAGER",
    .f_environment_variable_intro = "PRINBEE_LANGUAGE_MANAGER_",
    .f_section_variables_name = nullptr,
    .f_configuration_files = g_configuration_files,
    .f_configuration_filename = nullptr,
    .f_configuration_directories = nullptr,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [-<opt>]\n"
                     "where -<opt> is one or more of:",
    .f_help_footer = "Try `man language-manager` for more info.\n%c",
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





class language_manager
{
public:
                            language_manager(int argc, char * argv[]);

    int                     run();

private:
    int                     list();
    int                     list_available();
    int                     list_languages(prinbee::language::map_t const & languages);

    advgetopt::getopt       f_opt;
    std::string             f_file = prinbee::get_language_filename();
};


language_manager::language_manager(int argc, char * argv[])
    : f_opt(g_options_environment, argc, argv)
{
}


int language_manager::run()
{
    if(f_opt.is_defined("list-available"))
    {
        if(f_opt.is_defined("list"))
        {
            std::cerr << "warning: --list is ignored when --list-available is used.\n";
        }
        return list_available();
    }

    if(f_opt.is_defined("file"))
    {
        f_file = f_opt.get_string("file");
    }

    prinbee::load_languages(f_file);

    if(f_opt.is_defined("list"))
    {
        return list();
    }

    return 0;
}


int language_manager::list_available()
{
    std::int32_t count(0);
    icu::Locale const * locales(icu::Locale::getAvailableLocales(count));
    prinbee::language::map_t languages;

    for(std::int32_t idx(0); idx < count; ++idx)
    {
        prinbee::language::pointer_t l(std::make_shared<prinbee::language>());
        l->set_id(idx + 1); // this is not the "real" identifier in this case

        icu::UnicodeString country_name;
        locales[idx].getDisplayCountry(country_name);
        std::string utf8_country_name;
        country_name.toUTF8String(utf8_country_name);
        l->set_country(utf8_country_name);

        icu::UnicodeString language_name;
        locales[idx].getDisplayLanguage(language_name);
        std::string utf8_language_name;
        language_name.toUTF8String(utf8_language_name);
        l->set_language(utf8_language_name);

        l->set_country_2_letters(locales[idx].getCountry());
        l->set_language_2_letters(locales[idx].getLanguage());
        l->set_language_3_letters(locales[idx].getISO3Language());

        languages[l->get_id()] = l;
    }

    return list_languages(languages);
}


int language_manager::list()
{
    return list_languages(prinbee::get_all_languages());
}


int language_manager::list_languages(prinbee::language::map_t const & languages)
{
    std::size_t max_country_length(7);
    std::size_t max_language_length(8);
    for(auto const & l : languages)
    {
        max_country_length = std::max(max_country_length, l.second->get_country().length());
        max_language_length = std::max(max_language_length, l.second->get_language().length());
    }
    std::cout
        << "+-------+---------" << std::string(max_country_length - 7, '-')
                          << "+----------" << std::string(max_language_length - 8, '-')
                                     << "+----+----+-----+\n"
           "| ID    | Country " << std::string(max_country_length - 7, ' ')
                          << "| Language " << std::string(max_language_length - 8, ' ')
                                     << "| C2 | L2 | L3  |\n";
    for(auto const & l : languages)
    {
        std::cout
            << std::setw(5) << l.second->get_id()
            << " | "
            << std::setw(max_country_length - l.second->get_country().length()) << l.second->get_country()
            << " | "
            << std::setw(max_language_length - l.second->get_country().length()) << l.second->get_language()
            << " | "
            << l.second->get_country_2_letters()
            << " | "
            << l.second->get_language_2_letters()
            << " | "
            << l.second->get_language_3_letters()
            << " |\n";
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
        language_manager m(argc, argv);
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
