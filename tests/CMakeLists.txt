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

if(SnapCatch2_FOUND)

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
            ${PROJECT_SOURCE_DIR}
            ${SNAPCATCH2_INCLUDE_DIRS}
            ${LIBEXCEPT_INCLUDE_DIRS}
            ${LIBUTF8_INCLUDE_DIRS}
    )

    target_link_libraries(${PROJECT_NAME}
        prinbee
        ${SNAPCATCH2_LIBRARIES}
        ${LIBUTF8_LIBRARIES}
    )

else(SnapCatch2_FOUND)

    message("snapcatch2 not found... no unit test will be built.")

endif(SnapCatch2_FOUND)

add_subdirectory(bloomfilter)

if(SnapCatch2_FOUND)

    find_package(SnapTestRunner)
    AddUnitTestsTarget(
        PROJECT_NAME
            rununittests
    )

endif(SnapCatch2_FOUND)


# vim: ts=4 sw=4 et
