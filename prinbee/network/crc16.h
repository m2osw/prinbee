// Copyright (c) 2025  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Handling of CRC16 computations.
 *
 * In order to ensure that our binary messages are in good shape, we want
 * to make use of a CRC16. We have a magic number at the start of each
 * message and so if a message has an invalid size or CRC16, we can skip
 * up to the next message. The CRC16 helps us make sure that we do indeed
 * have a valid message whenever we find that header.
 *
 * You are, of course, welcome to use this CRC for other reasons.
 */

// C++
//
#include    <cstdint>



namespace prinbee
{



typedef std::uint16_t       crc16_t;

crc16_t crc16_compute(std::uint8_t const * data, std::size_t size);



} // namespace prinbee
// vim: ts=4 sw=4 et
