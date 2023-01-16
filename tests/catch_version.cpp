// Copyright (c) 2023  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/versiontheca
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

// self
//
#include    "catch_main.h"


// versiontheca
//
#include    <versiontheca/version.h>


// C++
//
//#include    <cstring>
//#include    <algorithm>


// last include
//
#include    <snapdev/poison.h>







CATCH_TEST_CASE("version", "[version]")
{
    CATCH_START_SECTION("version: verify runtime vs compile time versiontheca version numbers")
    {
        CATCH_REQUIRE(versiontheca::get_major_version()   == VERSIONTHECA_VERSION_MAJOR);
        CATCH_REQUIRE(versiontheca::get_release_version() == VERSIONTHECA_VERSION_MINOR);
        CATCH_REQUIRE(versiontheca::get_patch_version()   == VERSIONTHECA_VERSION_PATCH);
        CATCH_REQUIRE(strcmp(versiontheca::get_version_string(), VERSIONTHECA_VERSION_STRING) == 0);
    }
    CATCH_END_SECTION()
}




// vim: ts=4 sw=4 et
