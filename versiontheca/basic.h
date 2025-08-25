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
#pragma once

/** \file
 * \brief Trait used to parse and compare basic versions.
 *
 * The library supports ways to specifically handle very basic versions in
 * C++.
 *
 * A basic version is a version exclusively composed of numbers separated
 * by period (.) characters. By default, the numbers are any value from
 * 0 to 2^32-1 (4294967295). You can use the format to limit the values
 * to a smaller number. Comparing versions against each other is done
 * numerically only. A larger number represents a newer version.
 */

// self
//
#include    <versiontheca/trait.h>


namespace versiontheca
{



class basic
    : public trait
{
public:
    typedef std::shared_ptr<basic>       pointer_t;

    virtual bool        parse(std::string const & v) override;
};



}
// namespace versiontheca
// vim: ts=4 sw=4 et
