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
 * \brief Interface defining a version trait.
 *
 * The library supports multiple types of versions. Traits define how the
 * versions get parsed, canonicalized, compared, and a way to compute the
 * next and/or previous version.
 *
 * This file describes the necessary functions to parse a version and then
 * compare two versions together.
 */

// self
//
#include    <versiontheca/part.h>


// C++
//
#include    <memory>



namespace versiontheca
{



class trait
{
public:
    typedef std::shared_ptr<trait>       pointer_t;

    virtual             ~trait();

    void                clear();
    part &              at(int index);
    part const &        at(int index) const;
    part &              operator [] (int index);
    part const &        operator [] (int index) const;
    void                push_back(part const & p);
    void                insert(int index, part const & p);
    void                erase(int index);
    bool                empty() const;
    std::size_t         size() const;
    void                resize(std::size_t sz);

    virtual bool        parse(std::string const & v);
    virtual bool        is_valid_character(char32_t c) const;
    virtual int         compare(trait::pointer_t rhs) const;

    virtual bool        next(int pos, pointer_t format);
    virtual bool        previous(int pos, pointer_t format);

    virtual std::string to_string() const;

    std::string         get_last_error(bool clear = true) const;

protected:
    bool                parse_version(std::string const & v, char32_t sep);
    bool                parse_value(std::string const & value, char32_t sep);
    part                get_format_part(pointer_t format, int pos, bool integer);

    mutable std::string f_last_error = std::string();

private:
    part::vector_t      f_parts = part::vector_t();
};



}
// namespace versiontheca
// vim: ts=4 sw=4 et
