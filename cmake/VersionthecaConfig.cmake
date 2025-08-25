# - Find Versiontheca
#
# VERSIONTHECA_FOUND        - System has Versiontheca
# VERSIONTHECA_INCLUDE_DIRS - The Versiontheca include directories
# VERSIONTHECA_LIBRARIES    - The libraries needed to use Versiontheca
# VERSIONTHECA_DEFINITIONS  - Compiler switches required for using Versiontheca
#
# License:
#
# Copyright (c) 2023-2025  Made to Order Software Corp.  All Rights Reserved
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

find_path(
    VERSIONTHECA_INCLUDE_DIR
        versiontheca/version.h

    PATHS
        ENV VERSIONTHECA_INCLUDE_DIR
)

find_library(
    VERSIONTHECA_LIBRARY
        versiontheca

    PATHS
        ${VERSIONTHECA_LIBRARY_DIR}
        ENV VERSIONTHECA_LIBRARY
)

mark_as_advanced(
    VERSIONTHECA_INCLUDE_DIR
    VERSIONTHECA_LIBRARY
)

set(VERSIONTHECA_INCLUDE_DIRS ${VERSIONTHECA_INCLUDE_DIR})
set(VERSIONTHECA_LIBRARIES    ${VERSIONTHECA_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Versiontheca
    REQUIRED_VARS
        VERSIONTHECA_INCLUDE_DIR
        VERSIONTHECA_LIBRARY
)

# vim: ts=4 sw=4 et
