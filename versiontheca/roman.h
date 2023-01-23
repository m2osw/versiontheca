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
#pragma once

/** \file
 * \brief Trait used to parse and compare roman versions.
 *
 * The trait allows the version numbers to be written in roman numerals.
 *
 * \li I -- represents 1
 * \li V -- represents 5
 * \li X -- represents 10
 * \li L -- represents 50
 * \li C -- represents 100
 * \li D -- represents 500
 * \li M -- represents 1000
 *
 * The parser understands many syntaxes, but in most cases you want to use
 * the "subtract one" version instead of the "four of this" syntax. So to
 * write the roman numeral 4, you want to use IV instead of IIII.
 *
 * The parser ignores case, so ix and IX both represent 9.
 */

// self
//
#include    <versiontheca/trait.h>


namespace versiontheca
{



part_integer_t  from_roman_number(std::string const & value);
std::string     to_roman_number(part_integer_t value);



class roman
    : public trait
{
public:
    typedef std::shared_ptr<roman>       pointer_t;

    virtual bool        parse(std::string const & v) override;
    virtual std::string to_string() const override;
};



}
// namespace versiontheca
// vim: ts=4 sw=4 et
