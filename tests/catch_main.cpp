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

// Tell catch we want it to add the runner code in this file.
#define CATCH_CONFIG_RUNNER

// self
//
#include    "catch_main.h"


// prinbee
//
#include    <prinbee/utils.h>
#include    <prinbee/version.h>


// libexcept
//
#include    <libexcept/exception.h>


// snaplogger
//
#include    <snaplogger/logger.h>


// snapdev
//
#include    <snapdev/mkdir_p.h>
#include    <snapdev/not_used.h>
#include    <snapdev/pathinfo.h>
#include    <snapdev/rm_r.h>


// C++
//
#include    <fstream>


// C
//
#include    <sys/stat.h>
#include    <sys/types.h>


// last include
//
#include    <snapdev/poison.h>



namespace SNAP_CATCH2_NAMESPACE
{





//std::string setup_context(std::string const & sub_path, std::vector<std::string> const & xmls)
//{
//    std::string path(g_tmp_dir() + "/" + sub_path);
//
//    if(mkdir(path.c_str(), 0700) != 0)
//    {
//        if(errno != EEXIST)
//        {
//            CATCH_REQUIRE(!"could not create context path");
//            return std::string();
//        }
//    }
//
//    if(mkdir((path + "/tables").c_str(), 0700) != 0)
//    {
//        if(errno != EEXIST)
//        {
//            CATCH_REQUIRE(!"could not create table path");
//            return std::string();
//        }
//    }
//
//    if(mkdir((path + "/database").c_str(), 0700) != 0)
//    {
//        if(errno != EEXIST)
//        {
//            CATCH_REQUIRE(!"could not create database path");
//            return std::string();
//        }
//    }
//
//    for(auto const & x : xmls)
//    {
//        char const * s(x.c_str());
//        CATCH_REQUIRE((s[0] == '<'
//                    && s[1] == '!'
//                    && s[2] == '-'
//                    && s[3] == '-'
//                    && s[4] == ' '
//                    && s[5] == 'n'
//                    && s[6] == 'a'
//                    && s[7] == 'm'
//                    && s[8] == 'e'
//                    && s[9] == '='));
//        s += 10;
//        char const * e(s);
//        while(*e != ' ')
//        {
//            ++e;
//        }
//        std::string const name(s, e - s);
//
//        std::string const filename(path + "/tables/" + name + ".xml");
//        {
//            std::ofstream o(filename);
//            o << x;
//        }
//
//        // the table.xsd must pass so we can make sure that our tests make
//        // use of up to date XML code and that table.xsd is also up to date
//        //
//        std::string const verify_table("xmllint --noout --nonet --schema prinbee/data/tables.xsd " + filename);
//        std::cout << "running: " << verify_table << std::endl;
//        int const r(system(verify_table.c_str()));
//        CATCH_REQUIRE(r == 0);
//    }
//
//    return path;
//}


void init_callback()
{
    libexcept::set_collect_stack(libexcept::collect_stack_t::COLLECT_STACK_NO);
}


int init_tests(Catch::Session & session)
{
    snapdev::NOT_USED(session);

    snaplogger::setup_catch2_nested_diagnostics();
    snaplogger::mark_ready(); // we do not process options, so we have to explicitly call ready()

    // simulate a /var/lib/prinbee/... under our test temporary directory
    //
    // note: snapcatch2 deletes that temporary folder and
    //       everything under it on startup
    //
    std::string path(g_tmp_dir() + "/var/lib/prinbee");
    snapdev::mkdir_p(path);
    std::string errmsg;
    path = snapdev::pathinfo::realpath(path, errmsg); // make it an absolute path
    prinbee::set_prinbee_path(path);

    //snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
    //l->add_console_appender();
    //l->set_severity(snaplogger::severity_t::SEVERITY_ALL);

    // to test that the logger works as expected
    //SNAP_LOG_ERROR
    //    << "This is an error through the logger..."
    //    << SNAP_LOG_SEND;

    return 0;
}


}



int main(int argc, char * argv[])
{
    return SNAP_CATCH2_NAMESPACE::snap_catch2_main(
              "prinbee"
            , PRINBEE_VERSION_STRING
            , argc
            , argv
            , SNAP_CATCH2_NAMESPACE::init_callback
            , nullptr
            , SNAP_CATCH2_NAMESPACE::init_tests
        );
}


// vim: ts=4 sw=4 et
