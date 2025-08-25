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

// C++
//
#include    <cstdint>
#include    <string>
#include    <vector>



namespace versiontheca
{



constexpr std::size_t const     MAX_PARTS = 25;
constexpr char32_t const        NO_SEPARATOR = U'\0';


typedef std::uint32_t   part_integer_t;


class part
{
public:
    typedef std::vector<part>       vector_t;

    void                set_separator(char32_t separator);
    void                set_width(std::uint8_t width);
    void                set_type(char type);

    bool                set_value(std::string const & value);
    void                set_string(std::string const & s);
    void                set_integer(part_integer_t const i);
    void                set_to_max_string(std::size_t len = 1);
    void                set_to_max_integer();

    bool                next();
    bool                previous();

    char32_t            get_separator() const;
    std::uint8_t        get_width() const;
    char                get_type() const;

    bool                is_integer() const;
    std::string         get_string() const;
    part_integer_t      get_integer() const;
    std::string         to_string() const;
    std::string         get_last_error(bool clear = true) const;

    bool                is_zero() const;
    int                 compare(part const & rhs) const;    // change to <=> once available

protected:
    char32_t            f_separator = NO_SEPARATOR;
    std::uint8_t        f_width = 0;
    char                f_type = '\0';
    bool                f_is_integer = true;
    part_integer_t      f_integer = 0;
    std::string         f_string = std::string();
    mutable std::string f_last_error = std::string();
};



}
// namespace versiontheca
// vim: ts=4 sw=4 et
