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

/** \file
 * \brief Messenger for the prinbee daemon.
 *
 * The Prinbee daemon has a normal messenger connection. This is used to
 * find the daemons and connect to them. The clients make use of a
 * direct connection so communication can happen with large binary data
 * (i.e. large files are to be sent to the backends).
 */


// self
//
#include    "worker_pool.h"

//#include    "prinbeed.h"


// prinbee
//
//#include    <prinbee/names.h>


// cluck
//
//#include    <cluck/cluck_status.h>


// eventdispatcher
//
//#include    <eventdispatcher/names.h>


// communicatord
//
//#include    <communicatord/names.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee_daemon
{



/** \class worker_pool
 * \brief Manage a set of worker threads.
 *
 * This class creates a set of worker threads used to run the work the server
 * has to do.
 */



/** \brief The worker_pool initialization.
 *
 * This function initializes the worker pool.
 *
 * \note
 * The FIFO (\p fifo) is used to send payloads from one worker to the next.
 * If a function returns true, it means it updated the payload which is then
 * sent to the next available worker, So in most cases, our worker functions
 * return false since they process the message at once.
 *
 * \param[in] p  The prinbee object we are listening for (i.e. "daemon").
 * \param[in] worker_count  The number of threads to create.
 * \param[in] fifo  The input/output FIFO used to send work loads to the workers.
 */
worker_pool::worker_pool(
          prinbeed * p
        , int worker_count
        , cppthread::fifo<payload_t>::pointer_t fifo)
    : pool(
          "prinbee_pool"
        , worker_count
        , fifo
        , fifo // forward to the same worker pool!
        , p)
{
}



} // namespace prinbee_daemon
// vim: ts=4 sw=4 et
