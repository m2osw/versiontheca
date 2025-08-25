// Copyright (c) 2023-2025  Made to Order Software Corp.  All Rights Reserved
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
#define CATCH_CONFIG_RUNNER
#include    "catch_main.h"



// versiontheca
//
#include    <versiontheca/version.h>


// libexcept
//
//#include    <libexcept/exception.h>


// snapdev
//
//#include    <snapdev/not_used.h>
//#include    <snapdev/mkdir_p.h>


// C
//
//#include    <sys/stat.h>


// last include
//
#include    <snapdev/poison.h>




namespace SNAP_CATCH2_NAMESPACE
{





} // namespace SNAP_CATCH2_NAMESPACE






int main(int argc, char * argv[])
{
    return SNAP_CATCH2_NAMESPACE::snap_catch2_main(
              "versiontheca"
            , VERSIONTHECA_VERSION_STRING
            , argc
            , argv
            , nullptr
            , nullptr
            , nullptr
            , nullptr
        );
}


// vim: ts=4 sw=4 et
