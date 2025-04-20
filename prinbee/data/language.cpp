// Copyright (c) 2019-2025  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Database file implementation.
 *
 * Each table uses one or more files. Each file is handled by a dbfile
 * object and a corresponding set of blocks.
 *
 * \todo
 * The `version` field is not going to be cross instance compatible.
 * Any new instance of a database file gets a schema with version 1.0.
 * That version increases as modifications to the schema are being applied
 * (for example, as you add a new plugin to the Snap! environment of a
 * website, the content table is likely to be updated and get a newer
 * version).
 * \todo
 * The problem with this mechanism is that the exact same schema on two
 * different nodes will not always have the same version. If you create
 * a new node when another has a schema version 1.15, then the new node
 * gets the same schema, but the version is set to 1.0.
 * \todo
 * On day to day matters, this has no bearing, but it could be really
 * confusing to administrators. There are two possible solutions: have
 * the version assigned using communication and use the latest version
 * for that table, latest version across your entire set of nodes.
 * The other, which is much easier as it requires no inter-node
 * communication, is to calculate an MD5 sum of the schema. As long as
 * that calculation doesn't change across versions of Snap! Database
 * then we're all good (but I don't think we can ever guarantee such
 * a thing, so that solution becomes complicated in that sense).
 */

// self
//
#include    "prinbee/data/language.h"

#include    "prinbee/exception.h"


// advgetopt
//
#include    <advgetopt/advgetopt.h>
#include    <advgetopt/validator_integer.h>


// snaplogger
//
#include    <snaplogger/message.h>


// libutf8
//
#include    <libutf8/libutf8.h>


// snapdev
//
#include    <snapdev/to_upper.h>
#include    <snapdev/to_lower.h>


// C++
//
#include    <iomanip>
#include    <iostream>
//#include    <type_traits>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



namespace
{



language::map_t     g_languages;



}
// no name namespace




void language::set_id(language_id_t id)
{
    if(f_id != 0)
    {
        throw logic_error("language ID cannot be changed once set the first time.");
    }
    if(id == 0)
    {
        throw invalid_number("language ID cannot be set to 0.");
    }

    f_id = id;
}


language_id_t language::get_id() const
{
    return f_id;
}


/** \brief Compute the key of a Prinbee language.
 *
 * The language makes use of the standard language key which in general
 * is defined as "2 letter language abbreviation", an underscore, and
 * the "2 letter country abbreviation".
 *
 * The 2 letter language abbreviation is not always available. In that
 * case we make use of the 3 letter language abbreviation.
 *
 * Further, the country may not be defined either. i.e. a form of
 * language that is not tightly bound to a region. This is generally
 * the default for a language (i.e. French, in general, is the same in
 * countries where it is spoken; the region is important if you want
 * to use specifics to that region which may not be understandable by
 * people from another region).
 *
 * As a result, they function generates one of these:
 *
 * \code
 *     <2 letter language> + '_' + <2 letter country>
 *     <3 letter language> + '_' + <2 letter country>
 *     <2 letter language>
 *     <3 letter language>
 * \endcode
 *
 * The language manager used to generate our list of Prinbee supported
 * languages makes sure that all the languages have a unique key.
 *
 * \return The key for this language (i.e. "en_US", "fr_CA", etc.)
 */
std::string language::get_key() const
{
    std::string key;

    if(has_language_2_letters())
    {
        key += get_language_2_letters();
    }
    else
    {
        key += get_language_3_letters();
    }

    if(has_country_2_letters())
    {
        key += '_';
        key += get_country_2_letters();
    }

    return key;
}


void language::set_country(std::string const & country)
{
    f_country = country;
}


std::string language::get_country() const
{
    return f_country;
}


void language::set_language(std::string const & l)
{
    f_language = l;
}


std::string language::get_language() const
{
    return f_language;
}


bool language::has_country_2_letters() const
{
    return f_country_2_letters[0] != '?' && f_country_2_letters[1] != '?';
}


void language::set_country_2_letters(std::string const & country)
{
    if(country.length() != 2)
    {
        throw invalid_size("the 2 letters country code must be exactly 2 letters.");
    }

    std::string const upper(snapdev::to_upper(country));
    f_country_2_letters[0] = upper[0];
    f_country_2_letters[1] = upper[1];
}


std::string language::get_country_2_letters() const
{
    return std::string(f_country_2_letters, 2);
}


bool language::has_language_2_letters() const
{
    return f_language_2_letters[0] != '?' && f_language_2_letters[1] != '?';
}


void language::set_language_2_letters(std::string const & l)
{
    if(l.length() != 2)
    {
        throw invalid_size("the 2 letters language code must be exactly 2 letters.");
    }

    std::string const lower(snapdev::to_lower(l));
    f_language_2_letters[0] = lower[0];
    f_language_2_letters[1] = lower[1];
}


std::string language::get_language_2_letters() const
{
    return std::string(f_language_2_letters, 2);
}


void language::set_language_3_letters(std::string const & l)
{
    if(l.length() != 3)
    {
        throw invalid_size("the 3 letters language code must be exactly 3 letters.");
    }

    std::string const lower(snapdev::to_lower(l));
    f_language_3_letters[0] = lower[0];
    f_language_3_letters[1] = lower[1];
    f_language_3_letters[2] = lower[2];
}


std::string language::get_language_3_letters() const
{
    return std::string(f_language_3_letters, 3);
}





char const * get_language_filename()
{
    return "/usr/share/prinbee/languages.ini";
}


void load_languages(std::string const & filename)
{
    // load the .ini file now
    //
    advgetopt::conf_file_setup setup(
          filename
        , advgetopt::line_continuation_t::line_continuation_unix
        , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
        , advgetopt::COMMENT_SHELL
        , advgetopt::SECTION_OPERATOR_INI_FILE | advgetopt::SECTION_OPERATOR_CPP);

    advgetopt::conf_file::pointer_t config(advgetopt::conf_file::get_conf_file(setup));

    g_languages.clear();

    advgetopt::conf_file::sections_t sections(config->get_sections());
    for(auto s : sections)
    {
        if(!s.starts_with("l::"))
        {
            // ??? -- should we error on such?
            continue;
        }

        std::int64_t id(0);
        if(!advgetopt::validator_integer::convert_string(s.c_str() + 3, id))
        {
            throw invalid_number(
                      "invalid language identifier \""
                    + s
                    + "\".");
        }
        if(id < 1 || id > 65535)
        {
            throw invalid_number(
                      "language identifier too small or too large in \""
                    + s
                    + "\".");
        }

        language::pointer_t l(std::make_shared<language>());
        l->set_id(id);

        l->set_country(config->get_parameter(s + "::country"));
        l->set_language(config->get_parameter(s + "::language"));

        std::string const country2(config->get_parameter(s + "::country_2_letters"));
        if(!country2.empty())
        {
            l->set_country_2_letters(country2);
        }

        std::string const lang2(config->get_parameter(s + "::language_2_letters"));
        if(!lang2.empty())
        {
            l->set_language_2_letters(lang2);
        }

        std::string const lang3(config->get_parameter(s + "::language_3_letters"));
        if(!lang3.empty())
        {
            l->set_language_3_letters(lang3);
        }

        g_languages[id] = l;
    }
}


language::map_t get_all_languages()
{
    return g_languages;
}


void display_languages(prinbee::language::map_t const & languages)
{
    if(languages.empty())
    {
        std::cout << "warning: no languages availables.\n";
        return;
    }

    std::size_t max_country_length(7);
    std::size_t max_language_length(8);
    for(auto const & l : languages)
    {
        max_country_length = std::max(max_country_length, libutf8::u8length(l.second->get_country()));
        max_language_length = std::max(max_language_length, libutf8::u8length(l.second->get_language()));
    }
    std::stringstream ss;
    ss  << "+-------+---------" << std::string(max_country_length - 7, '-')
                          << "+----------" << std::string(max_language_length - 8, '-')
                                     << "+----+----+-----+--------+\n";
    std::cout << ss.str()
        << "| ID    | Country " << std::string(max_country_length - 7, ' ')
                          << "| Language " << std::string(max_language_length - 8, ' ')
                                     << "| C2 | L2 | L3  | Key    |\n"
              << ss.str();
    for(auto const & l : languages)
    {
        std::cout
            << "| "
            << std::setw(5) << std::right << l.second->get_id() << std::left
            << " | "
            // there is a "bug" in the setw() code as it does not take UTF-8 in account
            << std::setw(max_country_length + l.second->get_country().length() - libutf8::u8length(l.second->get_country())) << l.second->get_country()
            << " | "
            << std::setw(max_language_length + l.second->get_language().length() - libutf8::u8length(l.second->get_language())) << l.second->get_language()
            << " | "
            << l.second->get_country_2_letters()
            << " | "
            << l.second->get_language_2_letters()
            << " | "
            << l.second->get_language_3_letters()
            << " | "
            << std::setw(6) << l.second->get_key()
            << " |\n";
    }
    std::cout << ss.str();
}


language::map_by_code_t languages_by_code(language::map_t const & languages, duplicate_t duplicates_handling)
{
    language::map_by_code_t result;
    language::map_t duplicates;
    std::size_t count_duplicates(0);
    for(auto const & l : languages)
    {
        std::string const key(l.second->get_key());
        auto it(result.find(key));
        if(it != result.end())
        {
            switch(duplicates_handling)
            {
            case duplicate_t::DUPLICATE_FORBIDDEN:
                throw invalid_entity("the input languages includes duplicates.");

            case duplicate_t::DUPLICATE_SILENT:
                break;

            case duplicate_t::DUPLICATE_VERBOSE:
                ++count_duplicates;
                duplicates[l.second->get_id()] = l.second;
                duplicates[it->second->get_id()] = it->second;
                break;

            }
        }
        else
        {
            result[key] = l.second;
        }
    }

    if(!duplicates.empty())
    {
        std::cout
            << "prinbee: found "
            << count_duplicates
            << " duplicated languages by key:\n";
        display_languages(duplicates);
    }

    return result;
}



} // namespace prinbee
// vim: ts=4 sw=4 et
