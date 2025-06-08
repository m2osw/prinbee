// Copyright (c) 2024-2025  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Binary message definition.
 *
 * Prinbee primarily communicates using binary messages. This is much more
 * efficient than using the communicator daemon. This file implements the
 * binary message header used by the low level binary interface.
 */


// self
//
#include    "prinbee/network/binary_message.h"

//#include    "prinbeed.h"
#include    "prinbee/data/structure.h"
#include    "prinbee/names.h"


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/to_string_literal.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{


namespace
{








} // no name namespace


/** \class binary_message
 * \brief Handle the binary message header.
 *
 * This class is an implementation of the structure used to define the
 * message header used by the binary communication interface.
 */





/** \brief Initialize a binary_message object.
 *
 * The constructor initializes a default binary message. This means there
 * is no data and the name is set to g_message_unknown.
 *
 * To send a message, you must at least set the message name. If the message
 * comes with a body, then you must also set its data.
 *
 * To receive a message, you should not define anything more since the
 * receiving end will read the message from the connection and save it
 * in this message object.
 */
binary_message::binary_message()
{
}


/** \brief Set the name of the message.
 *
 * This function saves the name of the message in this object. The name
 * is a 1 to 4 Latin 1 letter string that was converted to a message_name_t
 * type (an std::uint32_t integer).
 *
 * By default, the name is set to "unknown" (g_message_unknown). That
 * special value cannot be used to send a message. That also means you
 * cannot receive an "unknown" message.
 *
 * \param[in] name  The name of the message.
 */
void binary_message::set_name(message_name_t name)
{
    f_header.f_name = name;
}


/** \brief Retrieve the name of the message.
 *
 * Messages are given a 1 to 4 letter name. This function returns that name
 * as a message_name_t type (an std::uint32_t integer).
 *
 * The special value 0 represents the "unknown" message. This message
 * cannot be sent or received. It generally means that the set_name()
 * was not yet called on a binary_message object.
 *
 * \return The binary message name.
 */
message_name_t binary_message::get_name() const
{
    return f_header.f_name;
}


/** \brief Return the size of the message header.
 *
 * This function parses the message header and then computes the static
 * size. If the static size computation fails, there is an invalid field
 * in the header and the whole thing crashes.
 *
 * \return The size of the message header.
 */
std::size_t binary_message::get_message_header_size()
{
    return sizeof(header_t);
}


void binary_message::set_message_header_data(void const * data, std::size_t size)
{
#ifdef _DEBUG
    if(sizeof(header_t) != size)
    {
        throw invalid_size("incorrect size for the binary message header; expected "
            + std::to_string(sizeof(header_t))
            + ", called with "
            + std::to_string(size)
            + " instead.");
    }
#endif
    memcpy(&f_header, data, size);
}


void binary_message::add_message_header_byte(std::uint8_t data)
{
    memmove(reinterpret_cast<char *>(&f_header), reinterpret_cast<char *>(&f_header) + 1, sizeof(f_header) - 1);
    reinterpret_cast<char *>(&f_header)[sizeof(f_header) - 1] = data;
}


bool binary_message::is_message_header_valid() const
{
    // header starts with "bm" (magic)
    //
    if(f_header.f_magic[0] != 'b'
    || f_header.f_magic[1] != 'm')
    {
        return false;
    }

    // version cannot be 0
    //
    if(f_header.f_version != 0)
    {
        return false;
    }

    // message name must be defined (cannot be "unknown")
    //
    if(f_header.f_name == g_message_unknown)
    {
        return false;
    }

    // crc16 must be valid; read the entire entry in a buffer and compute
    //
    if(crc16_compute(reinterpret_cast<std::uint8_t const *>(&f_header), sizeof(f_header)) != 0)
    {
        return false;
    }

    // everything looks good
    //
    return true;
}


/** \brief Get the size of the following buffer.
 *
 * This function returns the size of the buffer after the header. This is
 * the size as saved in the header. You must make sure that the header
 * is valid before calling this function. The function may zero.
 *
 * If you are creating a message to send to the other side, then this
 * function won't work properly. Instead, you want to get the data or
 * data pointer.
 *
 * \return Size of the data following the header in a binary message.
 *
 * \sa is_message_header_valid()
 */
std::size_t binary_message::get_data_size() const
{
    return f_header.f_size;
}


/** \brief Get a pointer to the message header.
 *
 * This function returns a pointer to the header structure. This pointer
 * can be used to send the message header over the network or make a
 * copy for later use. The header is stable as long as not setter is
 * called and the binary_message object is not destroyed.
 *
 * The function returns a const buffer. You are not expected to update
 * the buffer. You may parse it in another binary_message using the
 * set_message_header_data() function.
 *
 * \note
 * The function is not const because it may need to re-evaluate the
 * CRC16's.
 *
 * \return A pointer to the message header.
 */
void const * binary_message::get_header()
{
    // at this time, I think it's fine to (re-)compute the CRC16's because
    // we call this function only once when we send the message over the
    // network
    //
    f_header.f_header_crc16 = crc16_compute(
              reinterpret_cast<std::uint8_t const *>(&f_header)
            , sizeof(f_header) - sizeof(f_header.f_header_crc16));

    if(has_data())
    {
        if(has_pointer())
        {
            f_header.f_data_crc16 = crc16_compute(
                      reinterpret_cast<std::uint8_t const *>(f_data)
                    , f_header.f_size);
        }
        else
        {
#ifdef _DEBUG
            if(f_buffer.size() != f_header.f_size)
            {
                throw invalid_size("the buffer size ("
                    + std::to_string(f_buffer.size())
                    + ") does not match the size saved in the message header ("
                    + std::to_string(f_header.f_size)
                    + ").");
            }
#endif
            f_header.f_header_crc16 = crc16_compute(f_buffer.data(), f_header.f_size);
        }
    }

    return &f_header;
}


/** \brief Check whether the message has data attached to it.
 *
 * The has_data() function checks whether the f_data or f_buffer holds
 * some data (i.e. are not empty buffers).
 *
 * To clear the data of a message, do:
 *
 * \code
 *     message.set_data_by_pointer(nullptr, 0);
 * \endcode
 *
 * \return true if at least one of the buffers is not empty.
 */
bool binary_message::has_data() const
{
    return f_header.f_size != 0;
}


/** \brief Check whether the set_data_by_pointer() was used or not.
 *
 * The has_pointer() function checks whether the f_data field is defined
 * or not. If not, the binary message is supposed to be defined in its
 * f_buffer (or not defined at all--however, the buffer may be empty and
 * considered defined--for example, the PING and PONG messages are sent
 * with an empty buffer).
 *
 * \return true if the set_data_by_pointer() was called with a non-null pointer.
 */
bool binary_message::has_pointer() const
{
    return f_data != nullptr;
}


/** \brief Set the message data to the specified \p data pointer.
 *
 * This function sets the message data to the pointer defined in \p data.
 * You must be very careful since this binary message must be used to
 * send the data before you delete your buffer or garbage will be sent
 * to the other end.
 *
 * \note
 * This function has the side effect of clearing the buffer (in case
 * the set_data() was called on this function).
 *
 * \warning
 * The input data pointer is not const on purpose. Although the binary
 * message does not change the buffer, it is your buffer and you could
 * end up modifying it at the wrong time. This is a reminder that tells
 * you not to do so.
 *
 * \param[in] data  A pointer to a buffer of data.
 * \param[in] size  The size of the buffer in bytes.
 *
 * \sa has_pointer()
 * \sa set_data()
 * \sa get_data_pointer()
 */
void binary_message::set_data_by_pointer(void * data, std::size_t size)
{
    f_data = data;
    f_header.f_size = size;
    f_buffer.clear();
}


/** \brief Retrieve the data pointer.
 *
 * This function returns the data pointer and sets the \p size parameter to
 * the number of bytes defined in that buffer.
 *
 * To know whether the data pointer is defined, use the has_pointer()
 * function.
 *
 * \param[out] size  The size of the buffer.
 *
 * \return The pointer to the data in this binary message. It may be null.
 *
 * \sa has_pointer()
 */
void * binary_message::get_data_pointer(std::size_t & size) const
{
    size = f_header.f_size;
    return f_data;
}


/** \brief Set the data of the message.
 *
 * This function saves the data at \p data in this binary message buffer.
 * The input data can be discarded at any time after this call. The
 * function makes a copy of it.
 *
 * \param[in] data  The data to save in the binary message buffer.
 * \param[in] size  The size of the \p data buffer in bytes.
 */
void binary_message::set_data(void const * data, std::size_t size)
{
    f_data = nullptr;
    f_header.f_size = size;
    f_buffer.resize(size);
    memcpy(f_buffer.data(), data, size);
}


/** \brief Get a reference to the binary message buffer.
 *
 * This function returns a reference to the binary message buffer. If the
 * has_pointer() function returns true, this function should not be used
 * since this buffer is not the one used by the binary message implementation.
 *
 * Note that the buffer may be empty.
 *
 * \return A constant reference to the binary message buffer.
 */
std::vector<std::uint8_t> const & binary_message::get_data() const
{
    return f_buffer;
}



} // namespace prinbee
// vim: ts=4 sw=4 et
