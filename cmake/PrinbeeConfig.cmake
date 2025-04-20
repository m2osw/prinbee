# Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
#
# https://snapwebsites.org/project/prinbee
# contact@m2osw.com
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# - Try to find Prinbee
#
# Once done this will define
#
# PRINBEE_FOUND        - System has Prinbee
# PRINBEE_INCLUDE_DIRS - The Prinbee include directories
# PRINBEE_LIBRARIES    - The libraries needed to use Prinbee (none)
# PRINBEE_DEFINITIONS  - Compiler switches required for using Prinbee (none)
#

find_path(
    PRINBEE_INCLUDE_DIR
        # TBD: we should have a prinbee.h at some point?
        prinbee/version.h

    PATHS
        ENV PRINBEE_INCLUDE_DIR
)

find_library(
    PRINBEE_LIBRARY
        prinbee

    PATHS
        ${PRINBEE_LIBRARY_DIR}
        ENV PRINBEE_LIBRARY
)

mark_as_advanced(
    PRINBEE_INCLUDE_DIR
    PRINBEE_LIBRARY
)

set(PRINBEE_INCLUDE_DIRS ${PRINBEE_INCLUDE_DIR})
set(PRINBEE_LIBRARIES    ${PRINBEE_LIBRARY}    )

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set SNAPLOGGER_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(
    Prinbee
    DEFAULT_MSG
    PRINBEE_INCLUDE_DIR
    PRINBEE_LIBRARY
)

# vim: ts=4 sw=4 et
