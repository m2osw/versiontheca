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
 * \brief Functions used to parse and compare debian versions.
 *
 * The library supports ways to specifically handle debian versions in C++.
 *
 * This file describes the necessary functions to parse a version and then
 * compare two versions together.
 */

// self
//
#include    <versiontheca/trait.h>


namespace versiontheca
{



class debian
    : public trait
{
public:
    typedef std::shared_ptr<debian>       pointer_t;

    virtual bool        parse(std::string const & v) override;
    virtual bool        is_valid_character(char32_t c) const override;

    virtual bool        next(int pos, trait::pointer_t format) override;
    virtual bool        previous(int pos, trait::pointer_t format) override;

    virtual std::string to_string() const override;

private:
    enum class accepted_chars_t
    {
        ACCEPTED_CHARS_EPOCH,
        ACCEPTED_CHARS_UPSTREAM,
        ACCEPTED_CHARS_DEBIAN_REVISION,
    };

    bool                prepare_next_previous(std::size_t & start, std::size_t & end);

    accepted_chars_t    f_accepted_chars = accepted_chars_t::ACCEPTED_CHARS_EPOCH;
};



}
// namespace versiontheca
// vim: ts=4 sw=4 et
