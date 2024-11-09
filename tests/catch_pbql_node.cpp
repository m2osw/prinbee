// Copyright (c) 2019-2024  Made to Order Software Corp.  All Rights Reserved
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

// prinbee
//
#include    <prinbee/pbql/node.h>

#include    <prinbee/exception.h>


// self
//
#include    "catch_main.h"


// snaplogger
//
#include    <snaplogger/message.h>


// last include
//
#include    <snapdev/poison.h>



namespace
{





} // no name namespace



CATCH_TEST_CASE("node", "[node] [pbql]")
{
    CATCH_START_SECTION("node: verify defaults")
    {
        prinbee::pbql::location l;
        std::string const filename(SNAP_CATCH2_NAMESPACE::random_string(1, 25));
        l.set_filename(filename);

        unsigned int line(0);
        SNAP_CATCH2_NAMESPACE::random(line);
        line %= 100;
        for(unsigned int idx(0); idx < line; ++idx)
        {
            l.next_line();
        }
        ++line;

        unsigned int column(0);
        SNAP_CATCH2_NAMESPACE::random(column);
        column %= 90;
        for(unsigned int idx(0); idx < column; ++idx)
        {
            l.next_column();
        }
        ++column;

        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));

        CATCH_REQUIRE(n->get_token() == prinbee::pbql::token_t::TOKEN_IDENTIFIER);

        prinbee::pbql::location const & loc(n->get_location());
        CATCH_REQUIRE(loc.get_filename() == filename);
        CATCH_REQUIRE(loc.get_column() == static_cast<int>(column));
        CATCH_REQUIRE(loc.get_line() == static_cast<int>(line));

        CATCH_REQUIRE(n->get_string() == std::string());
        CATCH_REQUIRE(n->get_integer() == prinbee::uint512_t());
        CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(n->get_floating_point(), 0.0L, 0.0L));
        CATCH_REQUIRE(n->get_parent() == nullptr);
        CATCH_REQUIRE(n->get_children_size() == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: verify string")
    {
        prinbee::pbql::location l;
        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));

        for(int i(0); i < 10; ++i)
        {
            std::string const identifier(SNAP_CATCH2_NAMESPACE::random_string(1, 25));
            n->set_string(identifier);
            CATCH_REQUIRE(n->get_string() == identifier);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("location: verify integer")
    {
        prinbee::pbql::location l;
        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));

        for(int i(0); i < 10; ++i)
        {
            prinbee::uint512_t a;
            SNAP_CATCH2_NAMESPACE::rand512(a);
            n->set_integer(a);
            CATCH_REQUIRE(n->get_integer() == a);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("location: verify floating point")
    {
        prinbee::pbql::location l;
        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));

        for(int i(0); i < 10; ++i)
        {
            double const a(drand48());
            n->set_floating_point(a);
            CATCH_REQUIRE(SNAP_CATCH2_NAMESPACE::nearly_equal(n->get_floating_point(), static_cast<long double>(a), 0.0L));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("location: verify tree")
    {
        prinbee::pbql::location l;

        // prepare nodes
        //
        prinbee::pbql::node::pointer_t root(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
        prinbee::pbql::node::pointer_t i32(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
        i32->set_integer(32);
        prinbee::pbql::node::pointer_t i54(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_INTEGER, l));
        i54->set_integer(54);
        prinbee::pbql::node::pointer_t plus(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_PLUS, l));

        // build tree
        //
        root->insert_child(-1, plus);
        plus->insert_child(-1, i54);
        plus->insert_child(0, i32);    // using the index, we can insert before/after another item

        CATCH_REQUIRE(root->get_children_size() == 1);
        CATCH_REQUIRE(root->get_child(0) == plus);
        CATCH_REQUIRE(plus->get_child(0) == i32);
        CATCH_REQUIRE(plus->get_child(0)->get_integer() == 32UL);
        CATCH_REQUIRE(plus->get_child(1) == i54);
        CATCH_REQUIRE(plus->get_child(1)->get_integer() == 54UL);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("node_error", "[node] [pbql] [error]")
{
    CATCH_START_SECTION("node: invalid token")
    {
        prinbee::pbql::token_t const invalid_token[] = {
            prinbee::pbql::token_t::TOKEN_UNKNOWN,
            prinbee::pbql::token_t::TOKEN_other,
            prinbee::pbql::token_t::TOKEN_max,
        };

        prinbee::pbql::location l;
        for(std::size_t idx(0); idx < std::size(invalid_token); ++idx)
        {
            CATCH_REQUIRE_THROWS_MATCHES(
                      std::make_shared<prinbee::pbql::node>(invalid_token[idx], l)
                    , prinbee::invalid_token
                    , Catch::Matchers::ExceptionMessage(
                              "prinbee_exception: node created with an invalid token ("
                            + std::to_string(static_cast<int>(invalid_token[idx]))
                            + ")."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: child not found")
    {
        prinbee::pbql::location l;
        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
        CATCH_REQUIRE_THROWS_MATCHES(
                  n->get_child(0)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                          "out_of_range: child 0 does not exist."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("node: insert child at wrong position")
    {
        prinbee::pbql::location l;
        prinbee::pbql::node::pointer_t n(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
        prinbee::pbql::node::pointer_t c(std::make_shared<prinbee::pbql::node>(prinbee::pbql::token_t::TOKEN_IDENTIFIER, l));
        CATCH_REQUIRE_THROWS_MATCHES(
                  n->insert_child(3, c)
                , prinbee::out_of_range
                , Catch::Matchers::ExceptionMessage(
                          "out_of_range: child 3 does not exist."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
