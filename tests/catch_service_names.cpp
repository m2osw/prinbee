// Copyright (c) 2019-2025  Made to Order Software Corp.  All Rights Reserved
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


// communicator
//
#include    <communicator/snapcatch2.hpp>


// last include
//
#include    <snapdev/poison.h>



CATCH_TEST_CASE("service_name", "[service]")
{
    CATCH_START_SECTION("service_name: verify the prinbee daemon service name")
    {
        std::string const source_dir(SNAP_CATCH2_NAMESPACE::g_source_dir());
        CATCH_REQUIRE(communicator::verify_service_name(source_dir));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("service_name: verify the prinbee proxy daemon service name")
    {
        std::string const source_dir(SNAP_CATCH2_NAMESPACE::g_source_dir());
        CATCH_REQUIRE(communicator::verify_service_name(source_dir, "proxy/messenger.cpp"));
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
