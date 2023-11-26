// Copyright (c) 2023  Made to Order Software Corp.  All Rights Reserved
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
#include    "catch_main.h"


// prinbee
//
#include    <prinbee/journal/journal.h>


// snapdev
//
#include    <snapdev/mkdir_p.h>
#include    <snapdev/not_used.h>


// advgetopt
//
//#include    <advgetopt/options.h>



namespace
{



void unlink_conf(std::string const & path)
{
    std::string conf_filename(path + "/journal.conf");
    snapdev::NOT_USED(unlink(conf_filename.c_str()));
}



}


CATCH_TEST_CASE("journal_options", "[journal]")
{
    CATCH_START_SECTION("journal_options: default options")
    {
        std::string path(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/journal_options");
        unlink_conf(path);
        CATCH_REQUIRE(snapdev::mkdir_p(path) == 0);

        prinbee::journal j(path);
        CATCH_REQUIRE(j.is_valid());
        CATCH_REQUIRE(j.set_maximum_number_of_files(5));
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
