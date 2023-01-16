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

/** \file
 * \brief Parse and compare Debian compatible versions.
 *
 * Debian versions are very well defined to support a limited set of
 * characters which can be compared with well defined expected results.
 * The functions available here implement the Debian algorithm as defined
 * in the Debian manual. It includes all the features to the letter because
 * we assume that fully supporting the version is of major importance.
 *
 * However, the colon (:) character is not supported in a filename under
 * the MS-Windows file systems. For this reason we have one exception: we
 * support a semi-colon (;) as an exact equivalent of a colon. This is fine
 * because by default the semi-colon is not considered valid in a filename.
 */

// self
//
#include    <versiontheca/basic.h>



// C++
//
//#include    <string>
//#include    <memory>
////#include  <iostream> // used for debug purposes
//#include    <sstream>
#include    <algorithm>
//#include    <vector>
//#include    <stdexcept>


// C
//
//#include    <errno.h>
//#include    <string.h>
//#include    <stdio.h>


// last include
//
#include    <snapdev/poison.h>



namespace versiontheca
{



bool basic::parse(std::string const & v)
{
    if(!trait::parse(v))
    {
        return false;
    }

    // all must be numbers in a basic version (1.2.3)
    //
    std::size_t const max(size());
    for(std::size_t idx(0); idx < max; ++idx)
    {
        if(!at(idx).is_integer())
        {
            return false;
        }
    }
    return true;
}



}
// namespace versiontheca
// vim: ts=4 sw=4 et
