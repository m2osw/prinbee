# Copyright (c) 2019-2025  Made to Order Software Corp.  All Rights Reserved
#
# https://snapwebsites.org/project/prinbee
# contact@m2osw.com
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

project(plugin_database)

find_package( Qt5Core REQUIRED )
find_package( Qt5Xml  REQUIRED )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(
    SYSTEM
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${Qt5Script_INCLUDE_DIRS}
        ${Qt5Sql_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS}
        ${Qt5XmlPatterns_INCLUDE_DIRS}
)

qt5_add_resources(SNAP_MANAGER_RESOURCES_RCC
    database.qrc
)

add_library(database SHARED
    database.cpp
    ${SNAP_MANAGER_MOC_CXX}
    ${SNAP_MANAGER_RESOURCES_RCC}
)

#message("AS2JS_LIBRARIES = " ${AS2JS_LIBRARIES})

target_link_libraries(database
    ${QT_LIBRARIES}
)

install(
    TARGETS
        database

    LIBRARY DESTINATION
        ${PLUGIN_INSTALL_DIR}
)

install(
    DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/

    DESTINATION
        include/snapmanager/plugins/database

    FILES_MATCHING PATTERN
        "*.h"
)

# vim: ts=4 sw=4 et
