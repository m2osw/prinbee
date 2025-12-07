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


// self
//
#include    "ping_pong_timer.h"

#include    "proxy.h"


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_proxy
{



/** \class ping_pong_timer
 * \brief A timer used to know when to send the next flop of PING messages.
 *
 * Once in a while, each proxy sends PING messages to the Prinbee daemons
 * to make sure the connection remains alive.
 */



/** \brief The timer initialization.
 *
 * The PING PONG timer wakes up every few seconds to send a PING message to
 * the Prinbee daemons. It gets initialized with that amount of time as
 * defined in the configuration file.
 *
 * \note
 * The amount of time between each call slips.
 *
 * \param[in] p  The proxy server we are listening for.
 * \param[in] interval_us  Amount of time between each wake up.
 */
ping_pong_timer::ping_pong_timer(proxy * p, std::int64_t interval_us)
    : timer(interval_us)
    , f_proxy(p)
{
    set_name("ping_pong_timer");
}


ping_pong_timer::~ping_pong_timer()
{
}


/** \brief Call the send_pings() function.
 *
 * When this function is called, the timer timed out. This means it is
 * time to call the send_pings() function.
 */
void ping_pong_timer::process_timeout()
{
    f_proxy->send_pings();
}



} // namespace prinbee_proxy
// vim: ts=4 sw=4 et
