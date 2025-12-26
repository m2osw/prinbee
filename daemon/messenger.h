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

// fluid-settings
//
#include    <fluid-settings/fluid_settings_connection.h>



namespace prinbee_daemon
{



class prinbeed;


class messenger
    : public fluid_settings::fluid_settings_connection
{
public:
    typedef std::shared_ptr<messenger>     pointer_t;

                                messenger(prinbeed * c, advgetopt::getopt & opts);
                                messenger(messenger const &) = delete;
    virtual                     ~messenger() override;

    messenger &                 operator = (messenger const &) = delete;

    void                        finish_parsing();

    // ed::connection_with_send_message implementation
    //
    virtual void                ready(ed::message & msg);
    virtual void                stop(bool quitting);

    // fluid_settings::fluid_settings_connection implementation
    //
    virtual void                fluid_settings_changed(
                                      fluid_settings::fluid_settings_status_t status
                                    , std::string const & name
                                    , std::string const & value) override;

private:
    void                        msg_clock_stable(ed::message & msg);
    void                        msg_clock_unstable(ed::message & msg);
    void                        msg_iplock_current_status(ed::message & msg);
    void                        msg_lock_status(ed::message & msg);
    void                        msg_prinbee_current_status(ed::message & msg);
    void                        msg_prinbee_get_status(ed::message & msg);

    prinbeed *                  f_prinbeed = nullptr;
    ed::dispatcher::pointer_t   f_dispatcher = ed::dispatcher::pointer_t();
};



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
