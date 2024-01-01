# Copyright (c) 2013-2024  Made to Order Software Corp.  All Rights Reserved
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

AtomicNames(names.an)

##
## prinbee library
##
project(prinbee CXX ASM)

# Put the version in the header file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

add_library(${PROJECT_NAME} SHARED
    bigint/add.s
    bigint/add128.s
    bigint/add256.s
    bigint/add512.s
    bigint/sub.s
    bigint/sub128.s
    bigint/sub256.s
    bigint/sub512.s

    bigint/int512.cpp
    bigint/uint512.cpp

    block/block_blob.cpp
    block/block.cpp
    block/block_data.cpp
    block/block_entry_index.cpp
    block/block_free_block.cpp
    block/block_free_space.cpp
    block/block_index_pointers.cpp
    block/block_indirect_index.cpp
    block/block_primary_index.cpp
    block/block_schema.cpp
    block/block_schema_list.cpp
    block/block_secondary_index.cpp
    block/block_top_index.cpp
    block/block_top_indirect_index.cpp

    file/file_bloom_filter.cpp
    file/file_external_index.cpp
    file/file_snap_database_table.cpp
    file/hash.cpp

    journal/journal.cpp

    database/conditions.cpp
    database/context.cpp
    database/cursor.cpp
    database/table.cpp
    database/row.cpp
    database/cell.cpp

    data/dbfile.cpp
    data/convert.cpp
    data/schema.cpp
    #data/script.cpp
    data/structure.cpp
    data/virtual_buffer.cpp

    error.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/names.cpp
    utils.cpp
    version.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ADVGETOPT_INCLUDE_DIRS}
        ${BASIC_XML_DIRS}
        ${CPPTHREAD_INCLUDE_DIRS}
        ${LIBEXCEPT_INCLUDE_DIRS}
        ${LIBUFTF8_INCLUDE_DIRS}
        ${MURMUR3_INCLUDE_DIRS}
        ${SNAPLOGGER_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${ADVGETOPT_LIBRARIES}
    ${BASIC_XML_LIBRARIES}
    ${CPPTHREAD_LIBRARIES}
    ${LIBEXCEPT_LIBRARIES}
    ${LIBUFTF8_LIBRARIES}
    ${MURMUR3_LIBRARIES}
    ${SNAPLOGGER_LIBRARIES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION
        ${PRINBEE_VERSION_MAJOR}.${PRINBEE_VERSION_MINOR}

    SOVERSION
        ${PRINBEE_VERSION_MAJOR}
)

install(
    TARGETS
        ${PROJECT_NAME}

    RUNTIME DESTINATION
        bin

    LIBRARY DESTINATION
        lib

    ARCHIVE DESTINATION
        lib
)

install(
    FILES
        bigint/add_sub.h
        bigint/int512.h
        bigint/uint512.h

    DESTINATION
        include/prinbee/bigint
)

install(
    FILES
        block/block_blob.h
        block/block_data.h
        block/block_entry_index.h
        block/block_free_block.h
        block/block_free_space.h
        block/block.h
        block/block_index_pointers.h
        block/block_indirect_index.h
        block/block_schema.h
        block/block_secondary_index.h
        block/block_top_index.h

    DESTINATION
        include/prinbee/block
)

install(
    FILES
        data/dbfile.h
        data/dbtype.h
        data/convert.h
        data/schema.h
        #data/script.h
        data/structure.h
        data/virtual_buffer.h

    DESTINATION
        include/prinbee/data
)

install(
    FILES
        journal/journal.h

    DESTINATION
        include/prinbee/journal
)

install(
    FILES
        database/cell.h
        database/context.h
        database/row.h
        database/table.h

    DESTINATION
        include/prinbee/database
)

install(
    FILES
        file/file_bloom_filter.h
        file/file_external_index.h
        file/file_snap_database_table.h
        file/hash.h

    DESTINATION
        include/prinbee/file
)

install(
    FILES
        network/consistency.h

    DESTINATION
        include/prinbee/network
)

install(
    FILES
        exception.h
        error.h
        ${CMAKE_CURRENT_BINARY_DIR}/names.h
        utils.h
        ${CMAKE_CURRENT_BINARY_DIR}/version.h

    DESTINATION
        include/prinbee
)


# vim: ts=4 sw=4 et
