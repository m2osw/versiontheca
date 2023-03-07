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
                                , std::string const & v = std::string());

                        // note: because of the trait pointer requirement
                        // copy is not possible without a clone() function
                        // instead, you can use:
                        //
                        //   versiontheca copy(<your trait>, rhs.get_version());
                        //
                        versiontheca(versiontheca const &) = delete;
    versiontheca &      operator = (versiontheca const &) = delete;

    void                set_format(versiontheca const & format);
    bool                set_version(std::string const & v);
    bool                next(int pos);
    bool                previous(int pos);

    bool                is_valid() const;
    std::size_t         size() const;
    std::string         get_version() const;
    void                set_major(part_integer_t value);
    part_integer_t      get_major() const;
    void                set_minor(part_integer_t value);
    part_integer_t      get_minor() const;
    void                set_patch(part_integer_t value);
    part_integer_t      get_patch() const;
    void                set_build(part_integer_t value);
    part_integer_t      get_build() const;
    std::string         get_last_error(bool clear = true) const;
    trait::pointer_t    get_trait() const;

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


inline std::ostream & operator << (std::ostream & os, versiontheca const & v)
{
    return os << v.get_version();
}



}
// namespace versiontheca
// vim: ts=4 sw=4 et
