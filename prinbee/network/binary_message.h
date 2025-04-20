// Copyright (c) 2016-2025  Made to Order Software Corp.  All Rights Reserved
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

// eventdispatcher
//
#include    <eventdispatcher/tcp_server_connection.h>



namespace prinbee
{



typedef std::uint32_t           message_name_t;


/** \brief Create a binary message name from a string.
 *
 * This function transforms a string in a message_name_t value.
 *
 * The message names cannot be the empty string.
 *
 * \exception std::invalid_argument
 * This exception is raised if the input string is nullptr, the empty string,
 * or the string is more than 4 characters.
 *
 * \param[in] name  The name of the message as a C-string.
 */
constexpr message_name_t create_message_name(char const * const name)
{
    if(name == nullptr)
    {
        throw std::invalid_argument("name cannot be null.");
    }
    if(name[0] == '\0')
    {
        throw std::invalid_argument("name cannot be empty.");
    }

    message_name_t result(0);

    result |= name[0];
    if(name[1] != '\0')
    {
        result |= name[1] << 8;
        if(name[2] != '\0')
        {
            result |= name[2] << 16;
            if(name[3] != '\0')
            {
                result |= name[3] << 24;
                if(name[4] != '\0')
                {
                    throw std::invalid_argument("name cannot be more than 4 characters.");
                }
            }
        }
    }

    return result;
}



constexpr message_name_t    g_message_unknown = 0;
constexpr message_name_t    g_message_ping = create_message_name("PING");
constexpr message_name_t    g_message_pong = create_message_name("PONG");



constexpr uint8_t const     g_binary_message_version = 1;



class binary_message
{
public:
    typedef std::shared_ptr<binary_message>  pointer_t;

                                binary_message();
                                binary_message(binary_message const &) = delete;
    binary_message &            operator = (binary_message const &) = delete;

    void                        set_name(message_name_t name);
    message_name_t              get_name() const;

    bool                        has_pointer() const;
    void                        set_data_by_pointer(void * data, std::size_t size);
    void *                      get_data_pointer(std::size_t & size) const;
    void                        set_data(void const * data, std::size_t size);
    std::vector<std::uint8_t> const &
                                get_data() const;

private:
    message_name_t              f_name = g_message_unknown;
    void *                      f_data = nullptr;
    std::size_t                 f_size = 0;
    std::vector<std::uint8_t>   f_buffer = std::vector<std::uint8_t>();
};



} // namespace prinbee
// vim: ts=4 sw=4 et
