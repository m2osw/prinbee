// Copyright (c) 2019-2022  Made to Order Software Corp.  All Rights Reserved
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


/** \file
 * \brief Consistency definitions.
 *
 * Whenever accessing the data you can specify the type of consistency is
 * important for your application.
 *
 * By default, prinbee uses QUORUM, which means the acknowledgement is
 * received only once the data was sent to at least (N / 2 + 1) nodes.
 * With the QUORUM consistency, anything you write is then always available
 * from any other server (assuming the writer and reader both use QUORUM).
 *
 * Data Trail:
 *
 * * From within your application, the data is saved in a journal. If somehow
 *   your application cannot connect to the Prinbee proxy, then that copy is
 *   still relevant and can be used to replay the events at any point later.
 * * Once in the proxy, it is considered to be at level 0. The database
 *   system is not in control of your data and it will eventually be saved
 *   in a table.
 * * The proxy then attempts to send the data to all the nodes in the
 *   corresponding partition (as computed using the key).
 * * When at least one database server has the data, but it is not a partition
 *   server for that data, the consistency is ANY. This is done by clients
 *   that somehow cannot connect to a proper partition server.
 * * When at least one database server has the data, you are at level 1.
 * * When at least two database servers have the data, you are at level 2.
 * * When at least three database servers have the data, you are at level 3.
 * * When at least (N / 2 + 1) servers have the data, you have a QUORUM.
 * * When at all N servers have the data, you have a level of ALL.
 *
 * If you use more than 3 servers for replication, then the level can go
 * over 3, but we do not consider that as important so we do not offer a
 * special consistency level beyond 3. There is the ALL level, though, which
 * means that all the destination servers received the data.
 *
 * If you have multiple data centers, you can check the QUORUM consistency
 * as:
 *
 * * LOCAL -- the quorum has been reached in your local network (i.e. another
 *   computer at, say 10.0.2.77)
 * * ANY -- the quorum has been reached within one of your data centers,
 *   whether local or remote
 * * EACH -- the quorum was reached on every single data center.
 *
 * The ultimate implementation will allow you to send data immediately by
 * saving it in your local application journal and return. Later you can
 * check the status of that request and see whether the EACH QUORUM state
 * was ever reached.
 */

// C++
//
#include    <stdint>



namespace prinbee
{



enum class consistency_t : std::int8_t
{
    CONSISTENCY_DEFAULT = -2,       // use current default, on startup it is CONSISTENCY_QUORUM
    CONSISTENCY_INVALID = -1,

    CONSISTENCY_ZERO = 0,           // it works when only the client has a copy
    CONSISTENCY_ONE = 1,            // at least one database server has a copy
    CONSISTENCY_TWO = 2,            // at least two database servers have a copy
    CONSISTENCY_THREE = 3,          // at least three database servers have a copy
    CONSISTENCY_QUORUM = 4,         // at least a QUORUM (N / 2 + 1) of correct partition servers have a copy (local or not)
    CONSISTENCY_ANY_QUORUM = 5,     // at least a QUORUM (N / 2 + 1) of any servers have a copy
    CONSISTENCY_LOCAL_QUORUM = 6,   // QUORUM in local database with correct partition
    CONSISTENCY_EACH_QUORUM = 7,    // QUORUM in each data center with correct partition
    CONSISTENCY_ANY = 8,            // any one database server available (may not be in the correct partition)
    CONSISTENCY_ALL = 9,            // all the servers in the partition have a copy
};



} // namespace prinbee
// vim: ts=4 sw=4 et
