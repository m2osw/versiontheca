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
 * \brief Trait used to parse and compare unicode versions.
 *
 * The library supports this trait which accepts any character in the string
 * parts of the version, including any unicode character. The only non-allowed
 * characters are control characters.
 *
 * \note
 * At the moment, only ASCII digits (code 0x30 to 0x39) are viewed as numbers.
 * Later, other non-fractional unicode numbers (such as circled numbers) will
 * be viewed as digits (once the libutf8 has support to detect those
 * characers). So make sure not to use those numbers in your strings because
 * later they will not compare properly.
 */

// self
//
#include    <versiontheca/trait.h>


namespace versiontheca
{



class unicode
    : public trait
{
public:
    typedef std::shared_ptr<unicode>       pointer_t;
};



}
// namespace versiontheca
// vim: ts=4 sw=4 et
