# Copyright (c) 2023  Made to Order Software Corp.  All Rights Reserved
#
# https://snapwebsites.org/project/versiontheca
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
## Debian compare characters
##
project(debian-order)

add_executable(${PROJECT_NAME}
    debian_order.cpp
)

project(debian-order-table)

set(DEBIAN_ORDER_TABLE_CI ${PROJECT_BINARY_DIR}/debian_order_table.ci)

add_custom_command(
    OUTPUT
        ${DEBIAN_ORDER_TABLE_CI}

    COMMAND
        debian-order >${DEBIAN_ORDER_TABLE_CI}

    WORKING_DIRECTORY
        ${PROJECT_BINARY_DIR}

    DEPENDS
        debian-order
)

add_custom_target(${PROJECT_NAME}
    DEPENDS
        ${DEBIAN_ORDER_TABLE_CI}
)

##
## RPM compare characters
##
project(rpm-order)

add_executable(${PROJECT_NAME}
    rpm_order.cpp
)

project(rpm-order-table)

set(RPM_ORDER_TABLE_CI ${PROJECT_BINARY_DIR}/rpm_order_table.ci)

add_custom_command(
    OUTPUT
        ${RPM_ORDER_TABLE_CI}

    COMMAND
        rpm-order >${RPM_ORDER_TABLE_CI}

    WORKING_DIRECTORY
        ${PROJECT_BINARY_DIR}

    DEPENDS
        rpm-order
)

add_custom_target(${PROJECT_NAME}
    DEPENDS
        ${RPM_ORDER_TABLE_CI}
)

##
## versiontheca library
##
project(versiontheca)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

add_library(${PROJECT_NAME} SHARED
    basic.cpp
    debian.cpp
    decimal.cpp
    part.cpp
    roman.cpp
    rpm.cpp
    trait.cpp
    version.cpp
    versiontheca.cpp

    ${DEBIAN_ORDER_TABLE_CI}
    ${RPM_ORDER_TABLE_CI}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${LIBEXCEPT_INCLUDE_DIRS}
        ${LIBUTF8_INCLUDE_DIRS}
        ${SNAPDEV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${LIBEXCEPT_LIBRARIES}
    ${LIBUTF8_LIBRARIES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION
        ${VERSIONTHECA_VERSION_MAJOR}.${VERSIONTHECA_VERSION_MINOR}

    SOVERSION
        ${VERSIONTHECA_VERSION_MAJOR}
)

install(
    TARGETS
        ${PROJECT_NAME}

    LIBRARY DESTINATION
        lib

    COMPONENT
        runtime
)

install(
    FILES
        basic.h
        debian.h
        decimal.h
        exception.h
        part.h
        rpm.h
        trait.h
        unicode.h
        versiontheca.h

    DESTINATION
        include/versiontheca
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/version.h

    DESTINATION
        include/versiontheca
)


# vim: ts=4 sw=4 et
