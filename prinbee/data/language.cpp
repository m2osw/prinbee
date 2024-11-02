// Copyright (c) 2019-2024  Made to Order Software Corp.  All Rights Reserved
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


// snapdev
//
#include    <snapdev/to_upper.h>
#include    <snapdev/to_lower.h>


// C++
//
//#include    <iostream>
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
        , advgetopt::SECTION_OPERATOR_INI_FILE);

    advgetopt::conf_file::pointer_t config(advgetopt::conf_file::get_conf_file(setup));

    advgetopt::conf_file::sections_t sections(config->get_sections());
    for(auto s : sections)
    {
        if(!s.starts_with("l"))
        {
            // ??? -- should we error on such?
            continue;
        }

        std::int64_t id(0);
        if(!advgetopt::validator_integer::convert_string(s.c_str() + 1, id))
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
        l->set_country_2_letters(config->get_parameter(s + "::country_2_letters"));

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



} // namespace prinbee
// vim: ts=4 sw=4 et
