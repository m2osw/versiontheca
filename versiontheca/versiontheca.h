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

// self
//
#include    <versiontheca/part.h>
#include    <versiontheca/trait.h>


// C++
//
#include    <memory>



namespace versiontheca
{



class versiontheca
{
public:
    typedef std::shared_ptr<versiontheca>       pointer_t;

                        versiontheca(
                                  trait::pointer_t const & t
                                , std::string const & v);

    void                set_format(versiontheca const & format);
    void                set_version(std::string const & v);
    bool                next(int pos);
    bool                previous(int pos);

    bool                is_valid() const;
    std::size_t         size() const;
    std::string         get_version() const;
    part_integer_t      get_major() const;
    part_integer_t      get_minor() const;
    part_integer_t      get_patch() const;
    part_integer_t      get_build() const;
    std::string         get_last_error(bool clear = true) const;

    int                 compare(versiontheca const & rhs) const;
    bool                operator == (versiontheca const & rhs) const;
    bool                operator != (versiontheca const & rhs) const;
    bool                operator <  (versiontheca const & rhs) const;
    bool                operator <= (versiontheca const & rhs) const;
    bool                operator >  (versiontheca const & rhs) const;
    bool                operator >= (versiontheca const & rhs) const;

private:
    trait::pointer_t    f_trait = trait::pointer_t();
    bool                f_valid = false;
    trait::pointer_t    f_format = trait::pointer_t();
};



}
// namespace versiontheca
// vim: ts=4 sw=4 et
