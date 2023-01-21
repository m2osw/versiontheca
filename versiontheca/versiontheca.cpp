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
#include    <versiontheca/versiontheca.h>

#include    <versiontheca/basic.h>
#include    <versiontheca/exception.h>


// libutf8
//
#include    <libutf8/iterator.h>


// snapdev
//
#include    <snapdev/hexadecimal_string.h>
#include    <snapdev/tokenize_string.h>


// C++
//
#include    <iostream>


// last include
//
#include    <snapdev/poison.h>



namespace versiontheca
{



versiontheca::versiontheca(
          trait::pointer_t const & t
        , std::string const & v)
    : f_trait(t == nullptr ? std::make_shared<basic>() : t)
{
    if(!v.empty())
    {
        set_version(v);
    }
}


void versiontheca::set_format(versiontheca const & format)
{
    f_format = format.f_trait;
}


void versiontheca::set_version(std::string const & v)
{
    f_valid = f_trait->parse(v);
    if(!f_valid)
    {
        f_trait->clear();
    }
}


bool versiontheca::next(int pos)
{
    f_valid = f_trait->next(pos, f_format);
    if(!f_valid)
    {
        f_trait->clear();
    }

    return f_valid;
}


bool versiontheca::previous(int pos)
{
    f_valid = f_trait->previous(pos, f_format);
    if(!f_valid)
    {
        f_trait->clear();
    }

    return f_valid;
}


bool versiontheca::is_valid() const
{
    return f_valid;
}


std::size_t versiontheca::size() const
{
    return f_trait->size();
}


std::string versiontheca::get_version() const
{
    return f_trait->to_string();
}


part_integer_t versiontheca::get_major() const
{
    if(f_trait->empty())
    {
        return 0;
    }
    part const & p(f_trait->at(0));
    if(!p.is_integer())
    {
        return 0;
    }
    return p.get_integer();
}


part_integer_t versiontheca::get_minor() const
{
    if(f_trait->size() < 2)
    {
        return 0;
    }
    part const & p(f_trait->at(1));
    if(!p.is_integer())
    {
        return 0;
    }
    return p.get_integer();
}


part_integer_t versiontheca::get_patch() const
{
    if(f_trait->size() < 3)
    {
        return 0;
    }
    part const & p(f_trait->at(2));
    if(!p.is_integer())
    {
        return 0;
    }
    return p.get_integer();
}


part_integer_t versiontheca::get_build() const
{
    if(f_trait->size() < 4)
    {
        return 0;
    }
    part const & p(f_trait->at(3));
    if(!p.is_integer())
    {
        return 0;
    }
    return p.get_integer();
}


std::string versiontheca::get_last_error(bool clear) const
{
    return f_trait->get_last_error(clear);
}


int versiontheca::compare(versiontheca const & rhs) const
{
    if(!f_valid || !rhs.f_valid)
    {
        throw invalid_version("one or both of the input versions are not valid.");
    }

    return f_trait->compare(rhs.f_trait);
}


bool versiontheca::operator == (versiontheca const & rhs) const
{
    return compare(rhs) == 0;
}


bool versiontheca::operator != (versiontheca const & rhs) const
{
    return compare(rhs) != 0;
}


bool versiontheca::operator < (versiontheca const & rhs) const
{
    return compare(rhs) < 0;
}


bool versiontheca::operator <= (versiontheca const & rhs) const
{
    return compare(rhs) <= 0;
}


bool versiontheca::operator > (versiontheca const & rhs) const
{
    return compare(rhs) > 0;
}


bool versiontheca::operator >= (versiontheca const & rhs) const
{
    return compare(rhs) >= 0;
}



}
// namespace versiontheca
// vim: ts=4 sw=4 et
