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
#include    <prinbee/data/schema.h>
#include    <prinbee/data/structure.h>
#include    <prinbee/exception.h>
#include    <prinbee/network/crc16.h>


// eventdispatcher
//
#include    <eventdispatcher/connection.h>


// advgetopt
//
#include    <advgetopt/utils.h>


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
typedef std::uint32_t           context_id_t;
typedef std::uint32_t           data_version_t;         // used to make sure schema alterations are sequential
typedef std::uint32_t           message_serial_t;


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

constexpr message_name_t        g_message_acknowledge       = create_message_name("ACK");
constexpr message_name_t        g_message_error             = create_message_name("ERR");
constexpr message_name_t        g_message_get_complex_types = create_message_name("GCTP");
constexpr message_name_t        g_message_get_context       = create_message_name("GCTX");
constexpr message_name_t        g_message_get_data          = create_message_name("GET");
constexpr message_name_t        g_message_get_index         = create_message_name("GIDX");
constexpr message_name_t        g_message_get_table         = create_message_name("GTBL");
constexpr message_name_t        g_message_list_contexts     = create_message_name("LCTX");
constexpr message_name_t        g_message_list_indexes      = create_message_name("LIDX");
constexpr message_name_t        g_message_list_tables       = create_message_name("LTBL");
constexpr message_name_t        g_message_lock              = create_message_name("LOCK");
constexpr message_name_t        g_message_ping              = create_message_name("PING");
constexpr message_name_t        g_message_pong              = create_message_name("PONG");
constexpr message_name_t        g_message_register          = create_message_name("REG");
constexpr message_name_t        g_message_set_complex_types = create_message_name("SCTP");
constexpr message_name_t        g_message_set_context       = create_message_name("SCTX");
constexpr message_name_t        g_message_set_data          = create_message_name("SET");
constexpr message_name_t        g_message_set_index         = create_message_name("SIDX");
constexpr message_name_t        g_message_set_table         = create_message_name("STBL");
constexpr message_name_t        g_message_synchronize       = create_message_name("SYNC");
constexpr message_name_t        g_message_transaction       = create_message_name("TRAN");


std::string message_name_to_string(message_name_t name);


// all the binary connections understand the PING and PONG messages;
// when sending a PING, it is expected we receive a PONG as the reply


enum class err_code_t : std::uint32_t
{
    ERR_CODE_NONE = 0,
    ERR_CODE_INVALID_PARAMETERS,
    ERR_CODE_LOCK,
    ERR_CODE_PROTOCOL_UNSUPPORTED,
    ERR_CODE_TIME_DIFFERENCE_TOO_LARGE,
    ERR_CODE_UNEXPECTED_VERSION,
    ERR_CODE_UNKNOWN_PEER,
};


/** \brief The data sent along an ERROR message.
 *
 * Most messages expect a reply. If the destination cannot properly
 * handle the message, it instead sends an error message.
 *
 * The error message includes:
 *
 * \li the name of the message that generated the error;
 * \li the serial number of that message;
 * \li a machine error code; and
 * \li a human message.
 *
 * \note
 * The errors are passed using a bsr buffer, that way it is forward and
 * backward compatible over time. We may add more fields with time and
 * still safely be able to receive and send error messages to older
 * versions of the library.
 */
struct msg_error_t
{
    message_name_t          f_message_name = g_message_unknown;
    message_serial_t        f_serial_number = 0;
    err_code_t              f_code = err_code_t::ERR_CODE_NONE;
    std::string             f_error_message = std::string();
};


/** \brief The data sent along an ACK message.
 *
 * Most messages expect a reply. If the destination was able to handle
 * the message properly, it replies with an acknowledgment. Note that an
 * acknowledgment does not mean everything worked if the message does
 * work asynchronously after sending the acknowledgment (especially
 * INSERT or UPDATE commands, which are acknowledge quickly but may take
 * a long time as the backend needs to then update indexes that are affected
 * by the commands).
 *
 * The message includes the name of the message being acknowledged and
 * the original message serial number so the other side can match the
 * acknowledgment with the original message.
 *
 * Some messages also make use of the phase parameter defining which
 * phase is being acknowledged. For example, the INSERT, SET, and UPDATE
 * commands have many phases to acknowledge the data being journaled
 * locally, by the local proxy, and the prinbee daemon.
 */
struct msg_acknowledge_t
{
    message_name_t          f_message_name = g_message_unknown;
    message_serial_t        f_serial_number = 0;
    std::uint32_t           f_phase = 0;
};


/** \brief The pong message data.
 *
 * When sending a PING message, we expect to receive a PONG back. The
 * PONG includes the serial number of the PING. That way you can
 * make sure that all the PONG messages you receive back match
 * the PING messages you sent.
 *
 * The Proxy uses the PING / PONG messages to gather stats about the
 * pinged computers. That gives the Proxy a chance to send messages to
 * the least busy computer (when there is a choice of doing so).
 */
struct msg_pong_t
{
    message_serial_t        f_ping_serial_number = 0;
    double                  f_loadavg_1min = 0.0;
};


/** \brief The data sent along the REGISTER message.
 *
 * All the binary connections understand the REGISTER message; the receiver
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
struct msg_register_t
{
    std::string             f_name = std::string();
    std::string             f_protocol_version = std::string();
    timespec                f_now = snapdev::now();
};


/** \brief Structure used to send and receive a LCTX command.
 *
 * At the moment, there is nothing to send. The reply is a list of
 * context names separated by commas.
 */
struct msg_list_contexts_t
{
    advgetopt::string_list_t    f_list = advgetopt::string_list_t();
};


/** \brief Structure used to send and receive a context definition.
 *
 * This structure is used to communicate the equivalent of commands:
 *
 * \li CREATE CONTEXT
 * \li ALTER CONTEXT
 * \li DROP CONTEXT
 *
 * The binary version is handled within the binary_message implementation.
 * It uses a STRUCTURE to send/receive the data through the binary
 * connection.
 *
 * The IF NOT EXISTS of the CREATE CONTEXT is just in pbql; in that case,
 * the pbql implementation first does a GCTX and if it returns a context,
 * just do not do the SCTX. If not specified and the context exists, then
 * the CLI generates an error. In that case, an ALTER CONTEXT is required.
 *
 * When connecting to a context, you first need to send a GCTX to get the
 * context identifier. You later reuse that identifier to apply most of
 * the other functions (get/set tables, indexes, rows, etc.)
 */
struct msg_context_t
{
    std::string             f_context_name = std::string();             // name of the context
    std::string             f_description = std::string();              // description of this context
    schema_version_t        f_schema_version = 0;                       // do not set on a GCTX, increase by 1 on a SCTX
    context_id_t            f_context_id = 0;                           // defined by server when creating a context; unique among all contexts within a cluster
    snapdev::timespec_ex    f_created_on = snapdev::timespec_ex();      // set when creating a context
    snapdev::timespec_ex    f_last_updated_on = snapdev::timespec_ex(); // set when updating a context (ALTER CONTEXT ... -> SCTX)
    std::uint32_t           f_table_count = 0;                          // the number of tables in this context
};

enum phase_context_t : std::uint32_t
{
    //PHASE_CONTEXT_JOURNALED,        // local user journaled -- do we want such?! I think for schema related commands, it either works or doesn't
    //PHASE_CONTEXT_PROXY,            // proxy journaled
    PHASE_CONTEXT_RECEIVED,         // daemon received
    PHASE_CONTEXT_SAVED,            // daemon saved the data successfully
    PHASE_CONTEXT_SYNCING,          // daemon synchronized one or more other nodes
    PHASE_CONTEXT_INCOMPLETE,       // all live daemons are synchronized
    PHASE_CONTEXT_COMPLETE,         // 100% of the daemons are synchronized
};


struct msg_synchronization_table_t
{
    typedef std::list<msg_synchronization_table_t>      list_t;

    std::string             f_table_name = std::string();
    snapdev::timespec_ex    f_table_last_updated_on = snapdev::timespec_ex();
};


struct msg_synchronization_index_t
{
    typedef std::list<msg_synchronization_index_t>      list_t;

    std::string             f_index_name = std::string();
    snapdev::timespec_ex    f_index_last_updated_on = snapdev::timespec_ex();
};


struct msg_synchronization_t
{
    std::string             f_context_name = std::string();
    snapdev::timespec_ex    f_context_last_updated_on = snapdev::timespec_ex();
    snapdev::timespec_ex    f_complex_types_last_updated_on = snapdev::timespec_ex();
    msg_synchronization_table_t::list_t
                            f_tables = msg_synchronization_table_t::list_t();
    msg_synchronization_index_t::list_t
                            f_indexes = msg_synchronization_index_t::list_t();
};


struct msg_complex_types_t
{
    context_id_t    f_context_id = 0;               // the context ID
    buffer_t        f_complex_types = buffer_t();   // the complex types (used defined records)
};


struct msg_table_t
{
    context_id_t    f_context_id = 0;               // the context ID
    buffer_t        f_schemata = buffer_t();        // the table schemata
};


//enum class msg_message_t : std::uint32_t
//{
//    MSG_CREATE_CONTEXT              = DBTYPE_NAME("CTXT"),
//};
//
//
//constexpr char const * to_string(msg_message_t name)
//{
//    switch(name)
//    {
//    case msg_message_t::MSG_CREATE_CONTEXT:
//        return "CTXT";
//
//    }
//
//    return "";
//}


class binary_message
{
public:
    typedef std::shared_ptr<binary_message>                 pointer_t;
    typedef std::map<message_serial_t, pointer_t>           map_t;

    // for our cheap dispatcher like code
    //
    typedef std::function<bool(pointer_t msg)>              callback_t;
    typedef snapdev::callback_manager<callback_t>           callback_manager_t;
    typedef std::map<message_name_t, callback_manager_t>    callback_map_t;

                                binary_message();
                                binary_message(binary_message const & rhs);
    binary_message &            operator = (binary_message const & rhs) = delete;

    message_serial_t            get_next_serial_number();

    void                        set_name(message_name_t name);
    message_name_t              get_name() const;
    void                        set_acknowledged_by(ed::connection::pointer_t peer, bool success);
    ed::connection::pointer_t   get_acknowledged_by() const;
    bool                        get_acknowledged_success() const;

    static std::size_t          get_message_header_size();
    void                        set_message_header_data(void const * data, std::size_t size);
    void                        add_message_header_byte(std::uint8_t data);
    bool                        is_message_header_valid() const;
    std::size_t                 get_data_size() const;
    void const *                get_header();
    message_serial_t            get_serial_number() const;
    snapdev::timespec_ex        get_creation_date() const;

    bool                        has_data() const;
    bool                        has_pointer() const;
    void                        set_data_by_pointer(void * data, std::size_t size);
    void *                      get_data_pointer(std::size_t & size) const;
    void                        make_data_safe();
    void                        set_data(void const * data, std::size_t size);
    std::vector<std::uint8_t> const &
                                get_data() const;
    void const *                get_either_pointer(std::size_t & size) const;

    void                        create_acknowledge_message(pointer_t original_message, std::uint32_t phase);
    bool                        deserialize_acknowledge_message(msg_acknowledge_t & ack);
    void                        create_error_message(pointer_t original_message, err_code_t code, std::string const & error_message);
    bool                        deserialize_error_message(msg_error_t & err);
    void                        create_ping_message();
    void                        create_pong_message(binary_message::pointer_t ping_message);
    bool                        deserialize_pong_message(msg_pong_t & pong);
    void                        create_register_message(std::string const & name, std::string const & protocol_version);
    bool                        deserialize_register_message(msg_register_t & r);
    void                        create_list_contexts_message(advgetopt::string_list_t const & list);
    bool                        deserialize_list_contexts_message(msg_list_contexts_t & context_list);
    void                        create_context_message(msg_context_t const & context);
    bool                        deserialize_context_message(msg_context_t & context);

private:
    struct header_t
    {
        char                    f_magic[2] = { 'b', 'm' };
        message_version_t       f_version = g_binary_message_version;   // 1 to 255
        std::uint8_t            f_flags = 0;
        message_name_t          f_name = g_message_unknown;
        std::int64_t            f_created_on = 0;                       // this is a timespec_ex converted to an int64_t (i.e. timespec_ex cannot be in a bare struct we can copy with memcpy)
        message_serial_t        f_serial_number = 0;                    // to match a reply to the original message (i.e. ACK and ERR especially)
        std::uint32_t           f_size = 0;                             // size of following data
        std::uint32_t           f_padding = 0;                          // for now, we have another 32 bits available / unused
        crc16_t                 f_data_crc16 = 0;                       // following data CRC16
        crc16_t                 f_header_crc16 = 0;                     // CRC16 of this header
    };
    static_assert(
          sizeof(header_t) == offsetof(header_t, f_header_crc16) + sizeof(header_t::f_header_crc16)
        , "the f_header_crc16 field is expected to be the last 2 bytes of the header_t structure.");

    header_t                    f_header = header_t();
    void *                      f_data = nullptr;
    std::vector<std::uint8_t>   f_buffer = std::vector<std::uint8_t>();
    ed::connection::pointer_t   f_acknowledged_by = ed::connection::pointer_t();
    bool                        f_acknowledged_success = false;
};



} // namespace prinbee
// vim: ts=4 sw=4 et
