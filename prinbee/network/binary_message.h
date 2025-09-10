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

// prinbee
//
#include    <prinbee/exception.h>
#include    <prinbee/network/crc16.h>


// snapdev
//
#include    <snapdev/callback_manager.h>
#include    <snapdev/timespec_ex.h>


// C++
//
#include    <cstdint>
#include    <memory>
#include    <vector>



namespace prinbee
{



constexpr std::size_t const     PRINBEE_NETWORK_PAGE_SIZE = 4096;


typedef std::uint8_t            message_version_t;
typedef std::uint32_t           message_name_t;


constexpr message_version_t     g_binary_message_version = 1;


/** \brief Create a binary message name from a string.
 *
 * This function transforms a string in a message_name_t value.
 *
 * The message names cannot be the empty string or more than 4 characters.
 *
 * \note
 * The output is not affected by endianess.
 *
 * \exception prinbee::invalid_parameter
 * This exception is raised if the input string is nullptr, the empty string,
 * or the string is more than 4 characters.
 *
 * \param[in] name  The name of the message as a C-string.
 */
constexpr message_name_t create_message_name(char const * const name)
{
    if(name == nullptr)
    {
        throw prinbee::invalid_parameter("name cannot be null.");
    }
    if(name[0] == '\0')
    {
        throw prinbee::invalid_parameter("name cannot be empty.");
    }

    message_name_t result(0);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    constexpr int const s1 = 24;
    constexpr int const s2 = 16;
    constexpr int const s3 = 8;
    constexpr int const s4 = 0;
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    constexpr int const s1 = 0;
    constexpr int const s2 = 8;
    constexpr int const s3 = 16;
    constexpr int const s4 = 24;
#else
#error "Unsupported endianess"
#endif
    result |= name[0] << s1;
    if(name[1] != '\0')
    {
        result |= name[1] << s2;
        if(name[2] != '\0')
        {
            result |= name[2] << s3;
            if(name[3] != '\0')
            {
                result |= name[3] << s4;
                if(name[4] != '\0')
                {
                    throw prinbee::invalid_parameter("name cannot be more than 4 characters.");
                }
            }
        }
    }

    return result;
}


constexpr message_name_t        g_message_unknown = 0;
constexpr message_name_t        g_message_error = create_message_name("ERR");
constexpr message_name_t        g_message_ping = create_message_name("PING");
constexpr message_name_t        g_message_pong = create_message_name("PONG");
constexpr message_name_t        g_message_register = create_message_name("REG");
constexpr message_name_t        g_message_registered = create_message_name("REGD");


std::string message_name_to_string(message_name_t name);


// all the binary connections understand the PING and PONG messages;
// when sending a PING, it is expected we receive a PONG as the reply


/** \brief The data sent along the REGISTER message.
 *
 * Each binary connection understands the REGISTER message; the receiver
 * has to verify the protocol version and the time has to be close (because
 * the date of all the involved computers are used everywhere and they must
 * "match").
 *
 * The receiver then replies with either REGISTERED or ERROR; on an error
 * the connection is dropped and the error message includes the reason
 * for the refusal (protocol not compatible or time mismatch).
 *
 * Note: the REGISTER message is sent using a bsr buffer for the data;
 *       this means it can change over time without the need for the
 *       structure to be compatible and yet still communicate the pertinent
 *       data as expected (i.e. the bsr serializer uses names for fields
 *       which is forward and backward compatible).
 */
struct register_t
{
    std::string             f_name = std::string();
    std::string             f_protocol_version = std::string();
    timespec                f_now = snapdev::now();
};



class binary_message
{
public:
    typedef std::shared_ptr<binary_message>                 pointer_t;

    // for our cheap dispatcher like code
    //
    typedef std::function<bool(binary_message & msg)>       callback_t;
    typedef snapdev::callback_manager<callback_t>           callback_manager_t;
    typedef std::map<message_name_t, callback_manager_t>    callback_map_t;

                                binary_message();
                                binary_message(binary_message const &) = delete;
    binary_message &            operator = (binary_message const &) = delete;

    void                        set_name(message_name_t name);
    message_name_t              get_name() const;

    static std::size_t          get_message_header_size();
    void                        set_message_header_data(void const * data, std::size_t size);
    void                        add_message_header_byte(std::uint8_t data);
    bool                        is_message_header_valid() const;
    std::size_t                 get_data_size() const;
    void const *                get_header();

    bool                        has_data() const;
    bool                        has_pointer() const;
    void                        set_data_by_pointer(void * data, std::size_t size);
    void *                      get_data_pointer(std::size_t & size) const;
    void                        set_data(void const * data, std::size_t size);
    std::vector<std::uint8_t> const &
                                get_data() const;
    void const *                get_either_pointer(std::size_t & size) const;

    void                        create_register_message(std::string const & name, std::string const & protocol_version);
    bool                        deserialize_register_message(register_t & pp);

private:
    struct header_t
    {
        char                f_magic[2] = { 'b', 'm' };
        message_version_t   f_version = g_binary_message_version;   // 1 to 255
        std::uint8_t        f_flags = 0;
        message_name_t      f_name = g_message_unknown;
        std::uint32_t       f_size = 0;                             // size of following data
        crc16_t             f_data_crc16 = 0;                       // following data CRC16
        crc16_t             f_header_crc16 = 0;                     // CRC16 of this header
    };

    header_t                    f_header = header_t();
    void *                      f_data = nullptr;
    std::vector<std::uint8_t>   f_buffer = std::vector<std::uint8_t>();
};



} // namespace prinbee
// vim: ts=4 sw=4 et
