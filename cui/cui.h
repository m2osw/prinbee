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

// self
//
#include    "console_connection.h"
#include    "interrupt.h"
#include    "messenger.h"
#include    "ping_pong_timer.h"
#include    "proxy_connection.h"


// prinbee
//
#include    <prinbee/pbql/parser.h>



namespace prinbee_cui
{



enum msg_reply_t
{
    MSG_REPLY_RECEIVED,         // when we receive a message (i.e. not ACK nor ERR)
    MSG_REPLY_FAILED,           // ERR a message we sent
    MSG_REPLY_SUCCEEDED,        // ACK a message we sent
};



constexpr std::uint32_t const       MAX_PING_PONG_FAILURES = 5;


class cui
{
public:
                                    cui(int argc, char **argv);
    //                            cui(cui const & rhs) = delete;
    //virtual                     ~cui() override;

    //cui &                       operator = (cui const & rhs) = delete;

    int                             run();
    void                            start_binary_connection();
    void                            stop(bool quitting);
    void                            send_ping();
    std::string                     define_prompt();
    void                            execute_commands(std::string const & commands);
    bool                            user_commands(std::string const & command);
    std::string                     get_messenger_status() const;
    std::string                     get_fluid_settings_status() const;
    std::string                     get_proxy_status() const;
    snapdev::timespec_ex            get_last_ping() const;
    std::string                     get_prinbee_status() const;
    std::string                     get_console_status() const;

    void                            msg_prinbee_proxy_current_status(ed::message & msg);
    bool                            msg_process_reply(
                                          prinbee::binary_message::pointer_t msg
                                        , msg_reply_t state);

private:
    bool                            init_connections();
    bool                            init_console_connection();
    bool                            init_file();
    bool                            parse_clear();
    bool                            parse_help();

    advgetopt::getopt               f_opts;
    messenger::pointer_t            f_messenger = messenger::pointer_t();
    ed::communicator::pointer_t     f_communicator = ed::communicator::pointer_t();
    console_connection::pointer_t   f_console_connection = console_connection::pointer_t();
    proxy_connection::pointer_t     f_proxy_connection = proxy_connection::pointer_t();
    interrupt::pointer_t            f_interrupt = interrupt::pointer_t();
    ping_pong_timer::pointer_t      f_ping_pong_timer = ping_pong_timer::pointer_t();
    prinbee::pbql::command::vector_t
                                    f_cmds = prinbee::pbql::command::vector_t();
    std::string                     f_command = std::string();
    std::string                     f_file = std::string();
    std::string                     f_address = std::string();
    bool                            f_interactive = false;
    bool                            f_ready = false; // received the ACK from the REG message
    bool                            f_quit = false; // if true and all the commands were executed, we quit if true
    prinbee::pbql::parser::pointer_t
                                    f_parser = prinbee::pbql::parser::pointer_t();
    prinbee::pbql::lexer::pointer_t f_lexer = prinbee::pbql::lexer::pointer_t();
};



} // namespace prinbee_cui
// vim: ts=4 sw=4 et
