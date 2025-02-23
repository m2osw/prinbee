# Copyright (c) 2013-2024  Made to Order Software Corp.  All Rights Reserved.
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
## build the backend daemon
##
project(prinbee-daemon)

add_executable(${PROJECT_NAME}
    main.cpp

    interrupt.cpp
    messenger.cpp
    prinbeed.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ADVGETOPT_INCLUDE_DIRS}
        ${COMMUNICATORD_INCLUDE_DIRS}
        #${CPPPROCESS_INCLUDE_DIRS}
        ${EVENTDISPATCHER_INCLUDE_DIRS}
        ${FLUIDSETTINGS_INCLUDE_DIRS}
        ${LIBADDR_INCLUDE_DIRS}
        ${LIBEXCEPT_INCLUDE_DIRS}
        ${SNAPLOGGER_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    prinbee
    ${ADVGETOPT_LIBRARIES}
    ${COMMUNICATORD_LIBRARIES}
    #${CPPPROCESS_LIBRARIES}
    ${EVENTDISPATCHER_LIBRARIES}
    ${FLUIDSETTINGS_LIBRARIES}
    ${LIBADDR_LIBRARIES}
    ${LIBEXCEPT_LIBRARIES}
    ${SNAPLOGGER_LIBRARIES}
)

install(
    TARGETS
        ${PROJECT_NAME}

    RUNTIME DESTINATION
        sbin

    COMPONENT
        runtime
)


# vim: ts=4 sw=4 et nocindent
