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


// cppthread
//
#include    <cppthread/guard.h>
#include    <cppthread/mutex.h>


// snapdev
//
#include    <snapdev/brs.h>
#include    <snapdev/memory_streambuf.h>
#include    <snapdev/safe_assert.h>
#include    <snapdev/to_string_literal.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{


namespace
{



cppthread::mutex    g_message_mutex = cppthread::mutex();
std::uint32_t       g_serial_number = 0;


constexpr prinbee::struct_description_t g_acknowledge_description[] =
{
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_magic)
        , prinbee::FieldType(struct_type_t::STRUCT_TYPE_MAGIC)
        //, prinbee::FieldDefaultValue(prinbee::to_string(msg_message_t::MSG_ACKNOWLEDGE))
    ),
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_structure_version)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(1, 0)
    ),
    prinbee::define_description(
          prinbee::FieldName("message_name") // name of original message
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("serial")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::end_descriptions()
};


// sub-structure used to define an array of names (i.e. list of contexts)
constexpr prinbee::struct_description_t g_name_description[] =
{
    prinbee::define_description(
          prinbee::FieldName("name")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P8STRING)
    ),
    prinbee::end_descriptions()
};


constexpr prinbee::struct_description_t g_list_contexts_description[] =
{
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_magic)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        //, prinbee::FieldDefaultValue(prinbee::to_string(msg_message_t::MSG_LIST_CONTEXT))
    ),
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_structure_version)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(1, 0)
    ),
    prinbee::define_description(
          prinbee::FieldName("names")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_ARRAY16) // limit to 64K contexts in one cluster
        , prinbee::FieldSubDescription(g_name_description)
    ),
    prinbee::end_descriptions()
};


constexpr prinbee::struct_description_t g_pong_description[] =
{
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_magic)
        , prinbee::FieldType(struct_type_t::STRUCT_TYPE_MAGIC)
        //, prinbee::FieldDefaultValue(prinbee::to_string(msg_message_t::MSG_PONG))
    ),
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_structure_version)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(1, 0)
    ),
    prinbee::define_description(
          prinbee::FieldName("ping_serial_number")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::define_description(
          prinbee::FieldName("loadavg_1min")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_FLOAT64)
    ),
    prinbee::end_descriptions()
};


constexpr prinbee::struct_description_t g_set_context_description[] =
{
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_magic)
        , prinbee::FieldType(struct_type_t::STRUCT_TYPE_MAGIC)
        //, prinbee::FieldDefaultValue(prinbee::to_string(msg_message_t::MSG_SET_CONTEXT))
    ),
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_structure_version)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(1, 0)
    ),
    prinbee::define_description(
          prinbee::FieldName("context_name")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P8STRING)
    ),
    prinbee::define_description(
          prinbee::FieldName("description")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    prinbee::define_description(
          prinbee::FieldName("context_id")
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_UINT32)
    ),
    prinbee::end_descriptions()
};



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
    f_header.f_serial_number = get_next_serial_number();
    f_header.f_created_on = snapdev::now().to_nsec();
}


/** \brief Generate the next message serial number.
 *
 * This function generates the next serial number for this message. It is
 * automatically assigned whenever a new binary_message is created.
 *
 * \return The next message serial number.
 */
std::uint32_t binary_message::get_next_serial_number()
{
    cppthread::guard lock(g_message_mutex);
    ++g_serial_number;
    if(g_serial_number == 0)
    {
        g_serial_number = 1;
    }
    return g_serial_number;
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


void binary_message::set_acknowledged_by(ed::connection::pointer_t peer, bool success)
{
    f_acknowledged_by = peer;
    f_acknowledged_success = success;
}


bool binary_message::get_acknowledged_success() const
{
    return f_acknowledged_success;
}


ed::connection::pointer_t binary_message::get_acknowledged_by() const
{
    return f_acknowledged_by;
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
    memmove(reinterpret_cast<std::uint8_t *>(&f_header), reinterpret_cast<std::uint8_t *>(&f_header) + 1, sizeof(f_header) - 1);
    reinterpret_cast<std::uint8_t *>(&f_header)[sizeof(f_header) - 1] = data;
}


bool binary_message::is_message_header_valid() const
{
    // header starts with "bm" (magic)
    //
    if(f_header.f_magic[0] != 'b'
    || f_header.f_magic[1] != 'm')
    {
SNAP_LOG_ERROR << "--- no bm ---" << SNAP_LOG_SEND;
        return false;
    }

    // version cannot be 0
    //
    if(f_header.f_version == 0)
    {
SNAP_LOG_ERROR << "--- version is 0 ---" << SNAP_LOG_SEND;
        return false;
    }

    // message name must be defined (cannot be "unknown")
    //
    if(f_header.f_name == g_message_unknown)
    {
SNAP_LOG_ERROR << "--- unknown f_name ---" << SNAP_LOG_SEND;
        return false;
    }

    // crc16 must be valid; read the entire entry in a buffer and compute
    //
    if(crc16_compute(reinterpret_cast<std::uint8_t const *>(&f_header), sizeof(f_header)) != 0)
    {
SNAP_LOG_ERROR << "--- crc16 is wrong? ---" << SNAP_LOG_SEND;
        return false;
    }

    // everything looks good
    //
SNAP_LOG_ERROR << "--- perfect ---" << SNAP_LOG_SEND;
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
#ifdef _DEBUG
    if(sizeof(f_header) != offsetof(header_t, f_header_crc16) + sizeof(f_header.f_header_crc16))
    {
        throw logic_error("the header CRC16 is expected to be the last 2 bytes of the header structure.");
    }
#endif
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
            f_header.f_data_crc16 = crc16_compute(f_buffer.data(), f_header.f_size);
        }
    }
    else
    {
        f_header.f_data_crc16 = 0;
    }

    f_header.f_header_crc16 = crc16_compute(
              reinterpret_cast<std::uint8_t const *>(&f_header)
            , sizeof(f_header) - sizeof(f_header.f_header_crc16));

    return &f_header;
}


/** \brief Get the message serial number.
 *
 * Each message is given a unique serial number. This function returns that
 * number.
 *
 * \return The message serial number.
 */
std::uint32_t binary_message::get_serial_number() const
{
    return f_header.f_serial_number;
}


/** \brief Get the time and date when this message was created.
 *
 * For many messages, the creation time is important since it determines
 * whether one has to be interpreted before the other.
 *
 * \return The date and time when the message was created.
 */
snapdev::timespec_ex binary_message::get_creation_date() const
{
    return f_header.f_created_on;
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


void const * binary_message::get_either_pointer(std::size_t & size) const
{
    if(f_data != nullptr)
    {
        size = f_header.f_size;
        return f_data;
    }
    else
    {
        size = f_buffer.size();
        return f_buffer.data();
    }
}


void binary_message::create_acknowledge_message(pointer_t original_message, std::uint32_t phase)
{
    set_name(g_message_acknowledge);

    structure::pointer_t description(std::make_shared<structure>(g_acknowledge_description));

    description->set_integer("message_name", original_message->f_header.f_name);
    description->set_integer("serial_number", original_message->f_header.f_serial_number);
    description->set_integer("phase", phase);

    reference_t start_offset(0);
    virtual_buffer::pointer_t buffer(description->get_virtual_buffer(start_offset));
    f_buffer.resize(buffer->size());
    buffer->pread(
          f_buffer.data()
        , f_buffer.size()
        , 0);
}


bool binary_message::deserialize_acknowledge_message(msg_acknowledge_t & acknowledge)
{
    std::size_t size(0L);
    void const * data(get_either_pointer(size));

    virtual_buffer::pointer_t buffer(std::make_shared<virtual_buffer>());
    buffer->pwrite(data, size, 0, true);

    structure::pointer_t description(std::make_shared<structure>(g_acknowledge_description));
    description->set_virtual_buffer(buffer, 0);

    acknowledge.f_message_name = description->get_integer("message_name");
    acknowledge.f_serial_number = description->get_integer("serial_number");
    acknowledge.f_phase = description->get_integer("phase");

    return true;
}


void binary_message::create_error_message(pointer_t original_message, err_code_t code, std::string const & error_message)
{
    set_name(g_message_error);

    std::stringstream buffer;
    snapdev::serializer out(buffer);
    out.add_value("name", original_message->f_header.f_name);
    out.add_value("sn", original_message->f_header.f_serial_number);
    out.add_value("code", code);
    out.add_value("msg", error_message);
    std::string const data(buffer.str());
    set_data(data.data(), data.size());
}


bool binary_message::deserialize_error_message(msg_error_t & err)
{
    auto callback([&err](snapdev::deserializer<std::iostream> & s, snapdev::field_t const & f)
    {
        if(f.f_name == "name")
        {
            if(!s.read_data(err.f_message_name))
            {
                err.f_message_name = g_message_unknown;
            }
        }
        else if(f.f_name == "sn")
        {
            if(!s.read_data(err.f_serial_number))
            {
                err.f_serial_number = 0;
            }
        }
        else if(f.f_name == "code")
        {
            if(!s.read_data(err.f_code))
            {
                err.f_code = err_code_t::ERR_CODE_NONE;
            }
        }
        else if(f.f_name == "msg")
        {
            if(!s.read_data(err.f_error_message))
            {
                err.f_error_message.clear();
            }
        }

        return true;
    });

    std::size_t size(0L);
    void const * data(get_either_pointer(size));
    snapdev::memory_streambuf<char> buffer(data, size);
    std::iostream in(&buffer);
    snapdev::deserializer d(in);
    d.deserialize(callback);

    return true;
}


void binary_message::create_register_message(std::string const & name, std::string const & protocol_version)
{
    set_name(g_message_register);

    timespec const now(snapdev::now());

    std::stringstream buffer;
    snapdev::serializer out(buffer);
    out.add_value("name", name);
    out.add_value("protocol_version", protocol_version);
    out.add_value("now", now);
    std::string const data(buffer.str());
    set_data(data.data(), data.size());
}


bool binary_message::deserialize_register_message(msg_register_t & r)
{
    auto callback([&r](snapdev::deserializer<std::iostream> & s, snapdev::field_t const & f)
    {
        if(f.f_name == "protocol")
        {
            if(s.read_data(r.f_protocol_version))
            {
                r.f_protocol_version.clear();
            }
        }
        else if(f.f_name == "name")
        {
            if(!s.read_data(r.f_name))
            {
                r.f_name.clear();
            }
        }
        else if(f.f_name == "now")
        {
            if(!s.read_data(r.f_now))
            {
                r.f_now = {};
            }
        }

        return true;
    });

    std::size_t size(0L);
    void const * data(get_either_pointer(size));
    snapdev::memory_streambuf<char> buffer(data, size);
    std::iostream in(&buffer);
    snapdev::deserializer d(in);
    d.deserialize(callback);

    return true;
}


void binary_message::create_ping_message()
{
    set_name(g_message_ping);
}


void binary_message::create_pong_message(binary_message::pointer_t ping_message)
{
    snapdev::SAFE_ASSERT(ping_message->get_name() == g_message_ping);

    set_name(g_message_pong);

    structure::pointer_t description(std::make_shared<structure>(g_pong_description));

    // 'this' message is expected to be the PING message so we use its serial number
    //
    description->set_integer("ping_serial_number", ping_message->f_header.f_serial_number);
    double loadavg(-1.0);
    if(getloadavg(&loadavg, 1) != 1)
    {
        loadavg = -1.0;
    }
    description->set_float64("loadavg_1min", loadavg);

    reference_t start_offset(0);
    virtual_buffer::pointer_t buffer(description->get_virtual_buffer(start_offset));
    f_buffer.resize(buffer->size());
    buffer->pread(
          f_buffer.data()
        , f_buffer.size()
        , 0);
}


bool binary_message::deserialize_pong_message(msg_pong_t & pong)
{
    std::size_t size(0L);
    void const * data(get_either_pointer(size));

    virtual_buffer::pointer_t buffer(std::make_shared<virtual_buffer>());
    buffer->pwrite(data, size, 0, true);

    structure::pointer_t description(std::make_shared<structure>(g_pong_description));
    description->set_virtual_buffer(buffer, 0);

    pong.f_ping_serial_number = description->get_integer("ping_serial_number");
    pong.f_loadavg_1min = description->get_float64("loadavg_1min");

    return true;
}


void binary_message::create_list_contexts_message(advgetopt::string_list_t const & list)
{
    set_name(g_message_list_contexts);

    structure::pointer_t description(std::make_shared<structure>(g_list_contexts_description));

    for(auto const & n : list)
    {
        structure::pointer_t name(description->new_array_item("names"));
        name->set_string("name", n);
    }

    reference_t start_offset(0);
    virtual_buffer::pointer_t buffer(description->get_virtual_buffer(start_offset));
    f_buffer.resize(buffer->size());
    buffer->pread(
          f_buffer.data()
        , f_buffer.size()
        , 0);
}


bool binary_message::deserialize_list_contexts_message(msg_list_contexts_t & context_list)
{
    std::size_t size(0L);
    void const * data(get_either_pointer(size));

    virtual_buffer::pointer_t buffer(std::make_shared<virtual_buffer>());
    buffer->pwrite(data, size, 0, true);

    structure::pointer_t description(std::make_shared<structure>(g_list_contexts_description));
    description->set_virtual_buffer(buffer, 0);

    structure::vector_t list(description->get_array("names"));
    std::size_t const max(list.size());
    for(std::size_t idx(0); idx < max; ++idx)
    {
        context_list.f_list.push_back(list[idx]->get_string("name"));
    }

    return true;
}


void binary_message::create_context_message(msg_context_t const & context)
{
    set_name(g_message_set_context);

    structure::pointer_t description(std::make_shared<structure>(g_set_context_description));

    description->set_string("context_name", context.f_context_name);
    description->set_string("description", context.f_description);
    description->set_integer("schema_version", context.f_schema_version);
    description->set_integer("context_id", context.f_context_id);
    description->set_integer("created_on", context.f_created_on);
    description->set_integer("last_updated_on", context.f_last_updated_on);
    description->set_integer("table_count", context.f_table_count);

    reference_t start_offset(0);
    virtual_buffer::pointer_t buffer(description->get_virtual_buffer(start_offset));
    f_buffer.resize(buffer->size());
    buffer->pread(
          f_buffer.data()
        , f_buffer.size()
        , 0);
}


bool binary_message::deserialize_context_message(msg_context_t & context)
{
    std::size_t size(0L);
    void const * data(get_either_pointer(size));

    virtual_buffer::pointer_t buffer(std::make_shared<virtual_buffer>());
    buffer->pwrite(data, size, 0, true);

    structure::pointer_t description(std::make_shared<structure>(g_set_context_description));
    description->set_virtual_buffer(buffer, 0);

    context.f_context_name = description->get_string("context_name");
    context.f_description = description->get_string("description");
    context.f_context_id = description->get_integer("context_id");

    return true;
}


std::string message_name_to_string(message_name_t name)
{
    // handle special case
    //
    if(name == g_message_unknown)
    {
        return "<unknown>";
    }

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

    std::string result;

    result += name >> s1;
    char c(static_cast<char>(name >> s2));
    if(c != '\0')
    {
        result += c;
        c = static_cast<char>(name >> s3);
        if(c != '\0')
        {
            result += c;
            c = static_cast<char>(name >> s4);
            if(c != '\0')
            {
                result += c;
            }
        }
    }

    return result;
}



} // namespace prinbee
// vim: ts=4 sw=4 et
