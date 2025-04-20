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
#pragma once


/** \file
 * \brief Language object.
 *
 * The Prinbee database uses languages to distinguish content for different
 * regions. A key, say the URI of a page, can have multiple versions and
 * languages. This is managed by Prinbee. Inside the database files, the
 * language is just a 16 bit number. This is matched against a file
 * managed within the source code. The file is updated each time the
 * Unicode library is updated.
 */


// C++
//
#include    <cstdint>
#include    <map>
#include    <memory>



namespace prinbee
{



// this type is what gets saved in each row that support a language
//
typedef std::uint16_t                   language_id_t;


class language
{
public:
    typedef std::shared_ptr<language>           pointer_t;
    typedef std::map<language_id_t, pointer_t>  map_t;
    typedef std::map<std::string, pointer_t>    map_by_code_t; // key is the language + country code (i.e. "en_US") as available

    void                    set_id(language_id_t id);
    language_id_t           get_id() const;
    std::string             get_key() const;

    void                    set_country(std::string const & country);
    std::string             get_country() const;

    void                    set_language(std::string const & l);
    std::string             get_language() const;

    bool                    has_country_2_letters() const;
    void                    set_country_2_letters(std::string const & country);
    std::string             get_country_2_letters() const;

    bool                    has_language_2_letters() const;
    void                    set_language_2_letters(std::string const & l);
    std::string             get_language_2_letters() const;

    void                    set_language_3_letters(std::string const & l);
    std::string             get_language_3_letters() const;

private:
    std::string             f_country = std::string();
    std::string             f_language = std::string();
    language_id_t           f_id = 0;
    char                    f_country_2_letters[2] = { '?', '?' };
    char                    f_language_2_letters[2] = { '?', '?' };
    char                    f_language_3_letters[3] = { '?', '?', '?' };
};


char const *            get_language_filename();
void                    load_languages(std::string const & filename);
language::map_t         get_all_languages();
void                    display_languages(prinbee::language::map_t const & languages);

enum class duplicate_t
{
    DUPLICATE_FORBIDDEN,
    DUPLICATE_SILENT,
    DUPLICATE_VERBOSE,
};

language::map_by_code_t languages_by_code(
          language::map_t const & languages
        , duplicate_t duplicates_handling = duplicate_t::DUPLICATE_FORBIDDEN);



} // namespace prinbee
// vim: ts=4 sw=4 et
