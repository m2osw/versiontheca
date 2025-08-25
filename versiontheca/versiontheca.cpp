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

/** \file
 * \brief The main class of versiontheca is implemented here.
 *
 * You are expected to create a versiontheca using a trait. This file
 * includes the implementation calling the necessary functions on the
 * trait depending on what you need to do with the version data.
 */

/** \mainpage
 * \brief The versiontheca library is used to \em compile versions of various
 * types in a list of parts.
 *
 * The library supports traits, which are various implementations of the
 * version parser. The default is the Unicode trait. This version parser
 * supports pretty much all the Unicode characters. The period is viewed
 * as a part separator and like most of the other parsers, it views strings
 * of digits as number, opposed to a string, which is important for sorting
 * versions in an ascending order.
 *
 * All the traits are:
 *
 * \li Basic -- allow for numbers separated by periods (.) only
 * \li Debian -- allow for Debian like versions to be parsed (Debian, Ubuntu)
 * \li Decimal -- a basic version with just two numbers, so it can be converted
 * to a floating point number
 * \li roman -- allow for roman numerals to appear in a part
 * \li RPM -- allow RPM like versions to be parsed (Fedora, RedHat)
 * \li Unicode -- allow any character in your version, numbers are still viewed
 * as such and ordered as numbers, not strings
 *
 * \note
 * The implementation for the Debian and RPM version traits do not match
 * the corresponding distribution implementations. However, for most
 * common versions, it should work as expected. Only a very few packages
 * temporarily use versions that may fail.
 *
 * At this time, the source code and support is found on Github.com:
 * https://github.com/m2osw/versiontheca
 */

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


bool versiontheca::set_version(std::string const & v)
{
    f_valid = f_trait->parse(v);
    if(!f_valid)
    {
        f_trait->clear();
    }
    return f_valid;
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


void versiontheca::set_major(part_integer_t value)
{
    part p;
    p.set_integer(value);
    if(f_trait->size() >= 1)
    {
        f_trait->at(0) = p;
    }
    else
    {
        f_trait->push_back(p);
    }
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


void versiontheca::set_minor(part_integer_t value)
{
    part p;
    p.set_integer(value);
    if(f_trait->size() >= 2)
    {
        f_trait->at(1) = p;
    }
    else
    {
        if(f_trait->size() == 0)
        {
            part z;
            f_trait->push_back(z);
        }
        f_trait->push_back(p);
    }
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


void versiontheca::set_patch(part_integer_t value)
{
    part p;
    p.set_integer(value);
    if(f_trait->size() >= 3)
    {
        f_trait->at(2) = p;
    }
    else
    {
        while(f_trait->size() < 2)
        {
            part z;
            f_trait->push_back(z);
        }
        f_trait->push_back(p);
    }
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


void versiontheca::set_build(part_integer_t value)
{
    part p;
    p.set_integer(value);
    if(f_trait->size() >= 4)
    {
        f_trait->at(3) = p;
    }
    else
    {
        while(f_trait->size() < 3)
        {
            part z;
            f_trait->push_back(z);
        }
        f_trait->push_back(p);
    }
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


trait::pointer_t versiontheca::get_trait() const
{
    return f_trait;
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
