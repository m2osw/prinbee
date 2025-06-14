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

##
## prinbee unittest
##
project(unittest)

find_package(SnapCatch2)
find_package(Reporter)

if(SnapCatch2_FOUND AND Reporter_FOUND)

    add_executable(${PROJECT_NAME}
        catch_main.cpp

        catch_bigint.cpp
        catch_context.cpp
        catch_convert.cpp
        catch_dbfile.cpp
        catch_hash.cpp
        catch_journal.cpp
        catch_network.cpp
        catch_pbql_expression.cpp
        catch_pbql_input.cpp
        catch_pbql_lexer.cpp
        catch_pbql_location.cpp
        catch_pbql_node.cpp
        catch_pbql_parser.cpp
        catch_structure.cpp
        catch_utils.cpp
        catch_version.cpp
        catch_virtual_buffer.cpp
    )

    target_include_directories(${PROJECT_NAME}
        PUBLIC
            ${CMAKE_BINARY_DIR}
            ${EVENTDISPATCHER_INCLUDE_DIRS}
            ${LIBEXCEPT_INCLUDE_DIRS}
            ${LIBUTF8_INCLUDE_DIRS}
            ${PROJECT_SOURCE_DIR}
            ${REPORTER_INCLUDE_DIRS}
            ${SNAPCATCH2_INCLUDE_DIRS}
    )

        message("event dispatcher + report = " ${EVENTDISPATCHER_LIBRARIES} " -- " ${REPORTER_LIBRARIES})
    target_link_libraries(${PROJECT_NAME}
        prinbee
        ${ADVGETOPT_LIBRARIES}
        ${EVENTDISPATCHER_LIBRARIES}
        ${LIBADDR_LIBRARIES}
        ${LIBEXCEPT_LIBRARIES}
        ${LIBUTF8_LIBRARIES}
        ${REPORTER_LIBRARIES}
        ${SNAPCATCH2_LIBRARIES}
    )

else(SnapCatch2_FOUND AND Reporter_FOUND)

    message("snapcatch2 not found... no unit test will be built.")

endif(SnapCatch2_FOUND AND Reporter_FOUND)

add_subdirectory(bloomfilter)

if(SnapCatch2_FOUND AND Reporter_FOUND)

    find_package(SnapTestRunner)
    AddUnitTestsTarget(
        PROJECT_NAME
            rununittests
    )

endif(SnapCatch2_FOUND AND Reporter_FOUND)


# vim: ts=4 sw=4 et
