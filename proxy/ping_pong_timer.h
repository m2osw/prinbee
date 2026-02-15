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
#include    <eventdispatcher/timer.h>


// C
//
#include    <signal.h>



namespace prinbee_proxy
{



class proxy;



class ping_pong_timer
    : public ed::timer
{
public:
    typedef std::shared_ptr<ping_pong_timer>  pointer_t;

                                ping_pong_timer(proxy * p, std::int64_t interval_us);
                                ping_pong_timer(ping_pong_timer const & rhs) = delete;
    virtual                     ~ping_pong_timer() override;

    ping_pong_timer &           operator = (ping_pong_timer const & rhs) = delete;

    // ed::connection implementation
    virtual void                process_timeout() override;

private:
    proxy *                     f_proxy = nullptr;
    std::int64_t                f_interval = 0;
};



} // namespace prinbee_proxy
// vim: ts=4 sw=4 et
