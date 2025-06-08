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

/** \file
 * \brief Client connection handling binary messages between prinbee components.
 *
 * The Prinbee accepts binary connections from clients and daemons from
 * proxies. This implements such connections.
 */


// self
//
#include    "prinbee/network/binary_client.h"



//// prinbee
////
//#include    <prinbee/names.h>


// snaplogger
//
#include    <snaplogger/message.h>


//// eventdispatcher
////
//#include    <eventdispatcher/names.h>
//
//
//// communicatord
////
//#include    <communicatord/names.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{


namespace
{


constexpr std::size_t const     PRINBEE_NETWORK_PAGE_SIZE = 4096;


} // no name namespace


/** \class binary_client
 * \brief Handle messages from clients, proxies, Prinbee daemons.
 *
 * This class is an implementation of the event dispatcher TCP server
 * connection used to connect to the Proxy or Prinbee Daemon.
 */



/** \brief A binary connection to communicate with Prinbee.
 *
 * This connection is used to communicate between clients, proxies, and
 * daemons using binary messages which are way more compact than the
 * communicator daemon messages that use text.
 *
 * The socket is automatically made non-blocking.
 *
 * \note
 * At the moment, there is no limit to the size of a message. However,
 * many really large messages are likely to cause memory issues in the
 * long run. For example, one may want to manage large files and transfer
 * such in one large message (say 250Mb). It works, but it breaks the
 * memory by allocating one such large buffer (and the class does not
 * free that buffer until the whole client is destructed). It will
 * not in itself fragment the memory, but it will also use a very long
 * time to transfer that one single message not allowing any other
 * messages from being transferred in between. Our current strategy
 * will be to limit messages to 64Kb. That way, other intersperse
 * messages can happen quickly, memory management is much better,
 * and we can make use of a journal to get the entire message saved
 * on the other side before processing it.
 *
 * \todo
 * Consider using UDP since with a TCP connection, we get congestions
 * when one thing fails to go through or is really large, which can
 * prevent out of bounds communication.
 *
 * \param[in] a  The address to connect to.
 */
binary_client::binary_client(addr::addr const & a)
    : tcp_client_connection(a)
{
    set_name("binary_client");
    non_blocking();

    // to a minimum we need a buffer which is sufficient to read the header
    // size and we round that up to the next page size
    //
    std::size_t const size((binary_message::get_message_header_size() + PRINBEE_NETWORK_PAGE_SIZE - 1ULL) & -PRINBEE_NETWORK_PAGE_SIZE);
    f_data.resize(size);
}


binary_client::~binary_client()
{
}


void binary_client::send_message(binary_message & msg)
{
    write(msg.get_header(), binary_message::get_message_header_size());
    if(msg.has_data())
    {
        if(msg.has_pointer())
        {
            std::size_t size(0);
            void const * data(msg.get_data_pointer(size));
            write(data, size);
        }
        else
        {
            std::vector<std::uint8_t> const & data(msg.get_data());
            write(data.data(), data.size());
        }
    }
}


ssize_t binary_client::write(void const * buf, std::size_t count)
{
    if(!valid_socket())
    {
        errno = EBADF;
        return -1;
    }
SNAP_LOG_WARNING << "binary client write() called: " << buf << " + " << count << SNAP_LOG_SEND;

    if(buf != nullptr && count > 0)
    {
        char const * d(reinterpret_cast<char const *>(buf));
        std::size_t l(count);

        if(f_output.empty())
        {
            // attempt an immediate write() to the socket, this way we may
            // be able to avoid caching anything
            //
            errno = 0;
            ssize_t const r(tcp_client_connection::write(d, l));
            if(r > 0)
            {
                l -= r;
                if(l == 0)
                {
                    // no buffer needed!
                    //
SNAP_LOG_WARNING << "binary client write() -- instant write worked!" << SNAP_LOG_SEND;
                    return count;
                }

                // could not write the entire buffer, cache the rest
                //
                d += r;
            }
            // TODO: handle error cases -- the process_write() will do that
            //       but we're going to cache the data, etc. which is a waste
        }

SNAP_LOG_WARNING << "binary client write() -- caching data for later" << SNAP_LOG_SEND;
        f_output.insert(f_output.end(), d, d + l);
        return count;
    }

    return 0;
}


bool binary_client::is_writer() const
{
    return valid_socket() && !f_output.empty();
}


/** \brief Read incoming data.
 *
 * This function reads the binary message. This function maintains a state
 * to know whether it is reading the header, trying to re-sync. in case
 * we discovered an invalid header, or reading the data attached to a
 * message.
 *
 * The function tries to respect the maximum number of events to process
 * (5 by default) and the time limit imposed (500ms by default).
 *
 * \sa set_event_limit()
 * \sa set_processing_time_limit()
 */
void binary_client::process_read()
{
    if(valid_socket())
    {
        int count_messages(0);
        std::int64_t const date_limit(ed::get_current_date() + get_processing_time_limit());
        for(;;)
        {
            ssize_t r(0);
            switch(f_read_state)
            {
            case read_state_t::READ_STATE_HEADER:
                r = read(&f_data[f_data_size], binary_message::get_message_header_size() - f_data_size);
                if(r > 0)
                {
                    f_data_size += r;
                }
                break;

            case read_state_t::READ_STATE_HEADER_ADJUST:
                // this is necessary if a message was not sent properly and
                // we need to re-sync; in this case we do not want to change
                // f_data_size and adding the data to the message is
                // different
                //
                r = read(&f_data[0], 1);
                if(r > 0)
                {
                    f_binary_message.add_message_header_byte(f_data[0]);
                }
                break;

            case read_state_t::READ_STATE_DATA:
                r = read(&f_data[f_data_size], f_binary_message.get_data_size() - f_data_size);
                if(r > 0)
                {
                    f_data_size += r;
                }
                break;

            }

            if(r > 0)
            {
                switch(f_read_state)
                {
                case read_state_t::READ_STATE_HEADER:
                    if(f_data_size < binary_message::get_message_header_size())
                    {
                        break;
                    }
                    // the whole header was received
                    //
                    f_binary_message.set_message_header_data(f_data.data(), binary_message::get_message_header_size());
                    [[fallthrough]];
                case read_state_t::READ_STATE_HEADER_ADJUST:
#ifdef _DEBUG
                    if(f_data_size > binary_message::get_message_header_size())
                    {
                        throw invalid_size("the binary message header size is larger than the exact header size?!");
                    }
#endif
                    if(f_binary_message.is_message_header_valid())
                    {
                        f_data_size = 0;

                        if(f_binary_message.get_data_size() == 0)
                        {
                            // there is no data attached to that message,
                            // we can directly process it
                            //
                            f_binary_message.set_data_by_pointer(nullptr, 0);
                            process_message(f_binary_message);
                            ++count_messages;

                            // the state could be READ_STATE_HEADER_ADJUST
                            // so make sure it gets reset
                            //
                            f_read_state = read_state_t::READ_STATE_HEADER;
                        }
                        else
                        {
                            // make sure the buffer is large enough
                            //
                            std::size_t const min_size((f_binary_message.get_data_size() + PRINBEE_NETWORK_PAGE_SIZE - 1ULL) & -PRINBEE_NETWORK_PAGE_SIZE);
                            if(f_data.size() < min_size)
                            {
                                // we do not need to do a realloc() which is
                                // more likely to fragment memory
                                //
                                f_data.clear();
                                f_data.resize(min_size);
                            }

                            f_read_state = read_state_t::READ_STATE_DATA;
                        }
                    }
                    else
                    {
                        // adjust until we're properly re-synced
                        //
                        f_read_state = read_state_t::READ_STATE_HEADER_ADJUST;
                    }
                    break;

                case read_state_t::READ_STATE_DATA:
                    if(f_data_size >= f_binary_message.get_data_size())
                    {
#ifdef _DEBUG
                        if(f_data_size > f_binary_message.get_data_size())
                        {
                            throw invalid_size("the binary message data size is larger than the exact data size?!");
                        }
#endif
                        // there is no data attached to that message,
                        // we can directly process it
                        //
                        f_binary_message.set_data_by_pointer(f_data.data(), f_data_size);
                        process_message(f_binary_message);
                        ++count_messages;

                        f_read_state = read_state_t::READ_STATE_HEADER;
                        f_data_size = 0;
                    }
                    break;

                }

                if(count_messages >= get_event_limit()
                || ed::get_current_date() >= date_limit)
                {
                    // we reach one or both limits, stop processing so
                    // the other events have a chance to run
                    //
                    break;
                }
            }
            else if(r == 0 || errno == 0 || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            else // if(r < 0)
            {
                // TODO: do something more when an error occurs?
                //
                int const e(errno);
                SNAP_LOG_ERROR
                    << "an error occurred while reading from binary socket (errno: "
                    << e
                    << " -- "
                    << strerror(e)
                    << ")."
                    << SNAP_LOG_SEND;
                process_error();
                return;
            }
        }
    }

    // process next level too
    //
    tcp_client_connection::process_read();
}


void binary_client::process_write()
{
    if(valid_socket())
    {
        errno = 0;
        ssize_t const r(tcp_client_connection::write(&f_output[f_position], f_output.size() - f_position));
        if(r > 0)
        {
            // some data was written
            //
            f_position += r;
            if(f_position >= f_output.size())
            {
                f_output.clear();
                f_position = 0;
                process_empty_buffer();
            }
        }
        else if(r < 0 && errno != 0 && errno != EAGAIN && errno != EWOULDBLOCK)
        {
            // connection is considered bad, generate an error
            //
            int const e(errno);
            SNAP_LOG_ERROR
                << "an error occurred while writing to socket of \""
                << get_name()
                << "\" (errno: "
                << e
                << " -- "
                << strerror(e)
                << ")."
                << SNAP_LOG_SEND;
            process_error();
            return;
        }
    }

    // process next level too
    //
    tcp_client_connection::process_write();
}


void binary_client::process_hup()
{
    // this connection is dead...
    //
    close();

    // process next level too
    //
    tcp_client_connection::process_hup();
}



/** \func void process_message(binary_message & msg);
 * \brief Function called whenever a message is received.
 *
 * Whenever the process_read() function completes the receipt of a
 * binary message, it calls the process_message() function with
 * said message.
 *
 * If the message has data, then the \p msg parameter will include
 * that data as a pointer to a temporary buffer. When the function
 * returns that temporary buffer will be reused for the next
 * message. So the function that processes the message must make
 * a copy of the data as required if that is important.
 *
 * \param[in] msg  The binary message including the name and a pointer
 * to the data if any was attached to that message.
 */


} // namespace prinbee
// vim: ts=4 sw=4 et
