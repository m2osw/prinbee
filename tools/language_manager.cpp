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


// libutf8
//
#include    <libutf8/libutf8.h>


// snapdev
//
#include    <snapdev/stringize.h>


// ICU
//
#include    <unicode/locid.h>


// C++
//
#include    <random>
#include    <sstream>


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
    advgetopt::define_option(
          advgetopt::Name("verbose")
        , advgetopt::ShortName('v')
        , advgetopt::Flags(advgetopt::all_flags<
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

    int                         run();

private:
    int                         list();
    int                         list_available();
    prinbee::language::map_t    get_icu_languages();
    int                         update_language_list();

    advgetopt::getopt           f_opt;
    bool                        f_verbose = false;
    std::string                 f_file = prinbee::get_language_filename();
};


language_manager::language_manager(int argc, char * argv[])
    : f_opt(g_options_environment, argc, argv)
{
    f_verbose = f_opt.is_defined("verbose");
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

    return update_language_list();
}


int language_manager::list_available()
{
    prinbee::display_languages(get_icu_languages());
    return 0;
}


int language_manager::list()
{
    prinbee::display_languages(prinbee::get_all_languages());
    return 0;
}


prinbee::language::map_t language_manager::get_icu_languages()
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

#if 0
std::cout << std::setw(3) << idx << ". "
    << " country=" << utf8_country_name
    << " language=" << utf8_language_name
    << " 2C=" << locales[idx].getCountry()
    << " 2L=" << locales[idx].getLanguage()
    << " 3L=" << locales[idx].getISO3Language()
    << std::endl;
#endif
        char const * country_code(locales[idx].getCountry());
        if(country_code != nullptr
        && strlen(country_code) != 0)
        {
            // country not of 2 letters are three digit codes which
            // we convert as follow
            //
            //    001  --  world; use XW
            //    150  --  europe; use EU
            //    419  --  latin america; use XL
            //
            // some details about the chosen two letter codes can be found
            // on wikipedia:
            // https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2#ZZ
            //
            if(strlen(country_code) == 2)
            {
                l->set_country_2_letters(country_code);
            }
            else if(strcmp(country_code, "001") == 0)
            {
                l->set_country_2_letters("XW");
            }
            else if(strcmp(country_code, "150") == 0)
            {
                l->set_country_2_letters("EU");
            }
            else if(strcmp(country_code, "419") == 0)
            {
                l->set_country_2_letters("XL");
            }
            else
            {
                l->set_country(l->get_country() + " (" + country_code + ")");
                std::cout
                    << "warning: country code \""
                    << country_code
                    << "\" is not exactly 2 characters for country \""
                    << utf8_country_name
                    << "\"."
                    << std::endl;
            }
        }
        char const * language_code(locales[idx].getLanguage());
        if(language_code != nullptr
        && strlen(language_code) == 2)
        {
            l->set_language_2_letters(language_code);
        }
        l->set_language_3_letters(locales[idx].getISO3Language());

        languages[l->get_id()] = l;
    }

    return languages;
}


int language_manager::update_language_list()
{
    // get the list of languages from both sides
    //
    prinbee::language::map_t const icu_languages(get_icu_languages());
    prinbee::language::map_t prinbee_languages(prinbee::get_all_languages());

    if(f_verbose)
    {
        std::cout
            << "language-manager:info: cheking Prinbee languages for update. "
            << prinbee_languages.size()
            << " existing Prinbee languages; "
            << icu_languages.size()
            << " ICU languages.\n";
    }

    // convert to a list by code to eliminate duplicates from the ICU
    // list
    //
    prinbee::language::map_by_code_t const icu_by_code(languages_by_code(
              icu_languages
            , f_verbose
                ? prinbee::duplicate_t::DUPLICATE_VERBOSE
                : prinbee::duplicate_t::DUPLICATE_SILENT));

    prinbee::language::map_by_code_t const prinbee_by_code(languages_by_code(prinbee_languages));

    if(f_verbose
    && icu_by_code.size() != icu_languages.size())
    {
        std::cout
            << "language-manager:info: removed "
            << icu_languages.size() - icu_by_code.size()
            << " duplicates from the ICU list.\n";
    }

    // list of available IDs (in case some languages were removed, we can
    // reuse their old IDs... athough hopefully the database was properly
    // updated first!)
    //
    std::vector<bool> used_ids(65536);
    used_ids[0] = true;
    for(auto const & l : prinbee_languages)
    {
        used_ids[l.second->get_id()] = true;
    }

    // create a list of ICU entries that do not exist in the Prinbee list
    //
    // we want to shuffle that list to add it in a random order; the IDs
    // should not be consecutive for any type of order because later added
    // entries would otherwise not be in order... and the table could look
    // weirder (personal taste, I guess)
    //
    std::vector<prinbee::language::pointer_t> new_languages;
    for(auto const & l : icu_by_code)
    {
        if(prinbee_by_code.find(l.first) == prinbee_by_code.end())
        {
            new_languages.push_back(l.second);
        }
    }

    if(new_languages.empty())
    {
        std::cerr << "language-manager:info: no new languages found; Prinbee list of languages not updated.\n";
        return 0;
    }

    if(f_verbose)
    {
        std::cout
            << "language-manager:info: found "
            << new_languages.size()
            << " new languages.\n";
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(new_languages.begin(), new_languages.end(), g);

    for(auto const & l : new_languages)
    {
        auto it(std::find(used_ids.begin(), used_ids.end(), false));
        prinbee::language_id_t const id(std::distance(used_ids.begin(), it));

        // l is the language from the ICU with the ID we assigned, in order
        // for that purpose, for the prinbee_languages map, we need to create
        // a new language object and assigned the new `id` value as its ID
        //
        prinbee::language::pointer_t lang(std::make_shared<prinbee::language>());

        lang->set_id(id);

        lang->set_country(l->get_country());
        lang->set_language(l->get_language());
        lang->set_country_2_letters(l->get_country_2_letters());
        lang->set_language_2_letters(l->get_language_2_letters());
        lang->set_language_3_letters(l->get_language_3_letters());

        prinbee_languages[id] = lang;

        // this ID was used up
        //
        *it = true;
    }

//std::cout << "table to be saved:\n";
//prinbee::display_languages(prinbee_languages);

    // save to the .ini file
    //
    // we do not want to load the existing file in this case, rename
    // it first then proceed
    //
    if(rename(f_file.c_str(), (f_file + ".bak").c_str()) != 0)
    {
        if(errno != ENOENT)
        {
            std::cerr << "language-manager:error: could not rename existing Prinbee language file \""
                << f_file
                << "\" to \""
                << f_file
                << ".bak\".\n";
            return 1;
        }
    }
    advgetopt::conf_file::reset_conf_files();

    advgetopt::conf_file_setup setup(
          f_file
        , advgetopt::line_continuation_t::line_continuation_unix
        , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
        , advgetopt::COMMENT_SHELL
        , advgetopt::SECTION_OPERATOR_INI_FILE);

    advgetopt::conf_file::pointer_t config(advgetopt::conf_file::get_conf_file(setup));

    for(auto const & l : prinbee_languages)
    {
        std::string const section("l::" + std::to_string(static_cast<int>(l.second->get_id())));
        config->set_parameter(
              section
            , "language"
            , l.second->get_language());

        if(l.second->has_language_2_letters())
        {
            config->set_parameter(
                  section
                , "language_2_letters"
                , l.second->get_language_2_letters());
        }

        config->set_parameter(
              section
            , "language_3_letters"
            , l.second->get_language_3_letters());

        if(l.second->has_country_2_letters())
        {
            config->set_parameter(
                  section
                , "country"
                , l.second->get_country());

            config->set_parameter(
                  section
                , "country_2_letters"
                , l.second->get_country_2_letters());
        }
    }

    if(!config->save_configuration(
          std::string(".double-backup")   // this should not be used
        , false
        , false))
    {
        std::cerr
            << "language-manager:error: could not save configuration file to \""
            << f_file
            << "\" (read-only location?).\n";
        return 1;
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
