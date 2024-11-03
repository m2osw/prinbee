# Copyright (c) 2023-2024  Made to Order Software Corp.  All Rights Reserved
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
## Tool to check out the events in a Prinbee journal
##
project(prinbee-journal)

add_executable(${PROJECT_NAME}
    prinbee_journal.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ADVGETOPT_INCLUDE_DIRS}
        ${SNAPDEV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    prinbee
    ${ADVGETOPT_LIBRARIES}
)

install(
    TARGETS
        ${PROJECT_NAME}

    DESTINATION
        bin
)

##
## Tool to generate and further manage languages
##
project(language-manager)

add_executable(${PROJECT_NAME}
    language_manager.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ADVGETOPT_INCLUDE_DIRS}
        ${ICU_INCLUDE_DIRS}
        ${LIBUTF8_INCLUDE_DIRS}
        ${SNAPDEV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    prinbee
    ${ADVGETOPT_LIBRARIES}
    ${ICU_LIBRARIES}
    ${LIBUTF8_LIBRARIES}
)

install(
    TARGETS
        ${PROJECT_NAME}

    DESTINATION
        bin
)

# vim: ts=4 sw=4 et
