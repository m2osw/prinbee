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
//#include    "cui.h"


// eventdispatcher
//
#include    <eventdispatcher/cui_connection.h>


// C
//
#include    <panel.h>



namespace prinbee_cui
{



class cui;


class console_connection
    : public ed::cui_connection
{
public:
    typedef std::shared_ptr<console_connection>     pointer_t;

                        console_connection(cui * c);

                        console_connection(console_connection const & rhs) = delete;
                        console_connection & operator = (console_connection const & rhs) = delete;

    virtual void        ready() override;

    void                set_documentation_path(std::string const & path);
    void                reset_prompt();
    void                set_key_bindings();
    void                help(std::string const & section_name);
    bool                is_status_window_open() const;
    void                open_close_status_window();
    void                update_status();

    virtual void        process_command(std::string const & command) override;
    virtual void        process_quit() override;
    virtual void        process_help() override;

private:
    cui *               f_cui = nullptr;
    std::string         f_documentation_path = std::string();
    WINDOW *            f_win_status = nullptr;
    PANEL *             f_pan_status = nullptr;
};



} // namespace prinbee_cui
// vim: ts=4 sw=4 et
