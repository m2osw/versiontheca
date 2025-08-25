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
 * \brief Parse and compare Debian compatible versions.
 *
 * Debian versions are very well defined to support a limited set of
 * characters which can be compared with well defined expected results.
 * The functions available here implement the Debian algorithm as defined
 * in the Debian manual. It includes all the features to the letter because
 * we assume that fully supporting the version is of major importance.
 *
 * However, the colon (:) character is not supported in a filename under
 * the MS-Windows file systems. For this reason we have one exception: we
 * support a semi-colon (;) as an exact equivalent of a colon. This is fine
 * because by default the semi-colon is not considered valid in a filename.
 */

// self
//
#include    <versiontheca/debian.h>

#include    <versiontheca/exception.h>


// libutf8
//
#include    <libutf8/libutf8.h>


// C++
//
#include    <iostream>
#include    <type_traits>


// last include
//
#include    <snapdev/poison.h>



namespace versiontheca
{



namespace
{



#include    <versiontheca/debian_order_table.ci>

int compare_characters(char a, char b)
{
    // see order in debian_order.cpp
    //
    int const lidx(static_cast<int>(static_cast<std::uint8_t>(a)));
    int const ridx(static_cast<int>(static_cast<std::uint8_t>(b)));
#ifdef _DEBUG
    if(g_debian_compare_characters[lidx] == 0
    || g_debian_compare_characters[ridx] == 0)
    {
        // LCOV_EXCL_START
        throw logic_error("index ("
            + std::to_string(lidx)
            + " and/or "
            + std::to_string(ridx)
            + ") in debian/compare_characters() hit a 0, which means it represents an invalid character.");
        // LCOV_EXCL_STOP
    }
#endif
    int const r(g_debian_compare_characters[lidx] - g_debian_compare_characters[ridx]);
    return r == 0 ? 0 : (r < 0 ? -1 : 1);
}


int compare_strings(std::string const & lhs, std::string const & rhs)
{
    // because of the '~' we have to compare everything ('~' is before
    // '\0'...)
    //
    std::size_t const max(std::max(lhs.length(), rhs.length()));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        char const a(idx >= lhs.length() ? '\0' : lhs[idx]);
        char const b(idx >= rhs.length() ? '\0' : rhs[idx]);
        int const r(compare_characters(a, b));
        if(r != 0)
        {
            return r;
        }
    }
    return 0;
}



}
// no name namespace


/** \brief Parse a Debian version string.
 *
 * A Debian version string is composed of three parts:
 *
 * * Epoch -- a number followed by a colon (:)
 * * Upstream Version -- numbers, letters, and . + - : ~
 * * Debian-revision -- additional parts after the last hyphen
 *
 * The upstream version may include : only if the version includes an
 * epoch. Similarly, it can include a dash if there is a Debian revision.
 * To parse the version we first search the first colon (:) and parse
 * anything before that as the epoch. Then we search the last dash (-)
 * and parse that as the Debian revision. In between, the parse the
 * rest as a standard version string that can include those two special
 * characters.
 *
 * \param[in] v  The version to parse.
 *
 * \return true if the parser succeeded.
 */
bool debian::parse(std::string const & v)
{
    std::string::size_type colon(v.find(':'));
    std::string::size_type dash(v.rfind('-'));
    if((colon != std::string::npos && dash != std::string::npos && colon >= dash)
    || colon == 0ULL
    || dash == 0ULL)
    {
        // if there is a ':' then there has to be an epoch and a dash
        // cannot appear in the epoch
        //
        f_last_error =
              "position of ':' and/or '-' is invalid in \""
            + v
            + "\".";
        return false;
    }

    // if there is a colon we must have an epoch (there may be more colons
    // later in the version in which case "0:..." is required in that case)
    //
    if(colon != std::string::npos)
    {
        part p;
        f_accepted_chars = accepted_chars_t::ACCEPTED_CHARS_EPOCH;
        if(!p.set_value(v.substr(0, colon)))
        {
            f_last_error = p.get_last_error();
            return false;
        }
        if(!p.is_integer())
        {
            f_last_error = "epoch must be a valid integer.";
            return false;
        }
        p.set_type(':');
        push_back(p);
    }
    ++colon;

    if(dash == std::string::npos)
    {
        dash = v.length();
    }

    // the upstream can be parsed as is with parts separated by periods
    //
    std::string const upstream_version(v.substr(colon, dash - colon));
    f_accepted_chars = accepted_chars_t::ACCEPTED_CHARS_UPSTREAM;
    if(!trait::parse_version(upstream_version, colon == 0 ? U'\0' : U':'))
    {
        return false;
    }

    if(!at(colon == 0 ? 0 : 1).is_integer())
    {
        f_last_error =
              "a Debian version must always start with a number \""
            + v
            + "\".";
        return false;
    }

    if(dash < v.length())
    {
        // parse the revision
        //
        std::size_t idx(size());
        f_accepted_chars = accepted_chars_t::ACCEPTED_CHARS_DEBIAN_REVISION;
        if(!trait::parse_value(v.substr(dash + 1), U'-'))
        {
            return false;
        }

        // mark all of these as revision info
        //
        std::size_t const max(size());
        for(; idx < max; ++idx)
        {
            at(idx).set_type('-');
        }
    }

    return true;
}


bool debian::is_valid_character(char32_t c) const
{
    if(c >= U'0' && c <= U'9')
    {
        // this is not hit because:
        //
        // 1. epoch: we directly call the part::set_value()
        //    which doesn't use this callback, instead we check whether
        //    the resulting part is an INTEGER
        //
        // 2. others: we call this function only for characters other
        //    than digits (since we know digits are fine)
        //
        return true; // LCOV_EXCL_LINE
    }

    if(f_accepted_chars == accepted_chars_t::ACCEPTED_CHARS_EPOCH)
    {
        // this is not hit since this function is never called when the
        // accepted characters are for the epoch (as mentioned above,
        // we directly call the part::set_value())
        //
        return false; // LCOV_EXCL_LINE
    }

    if((c >= U'A' && c <= U'Z')
    || (c >= U'a' && c <= U'z')
    || c == U'+'
    || c == U'.'
    || c == U'~')
    {
        return true;
    }

    if(f_accepted_chars == accepted_chars_t::ACCEPTED_CHARS_DEBIAN_REVISION)
    {
        return false;
    }

    return c == U'-'
        || c == U':';
}


bool debian::get_upstream_positions(std::size_t & start, std::size_t & end) const
{
    std::size_t const max(size());
    if(max == 0ULL)
    {
        f_last_error = "no parts in this Debian version; cannot compute upstream start/end.";
        return false;
    }

    start = 0;
    end = size();
    for(std::size_t idx(0); idx < size(); ++idx)
    {
        if(at(idx).get_type() == ':' && start <= 0)
        {
            start = idx + 1;
        }
        else if(at(idx).get_type() == '-' && end > idx)
        {
            end = idx;
            break;
        }
    }

    // this cannot happen since the standard part is required
    // (i.e. 123:-blah is not a valid version, it has an empty version in
    // between and the parser detects that before we can get here)
    //
    if(static_cast<std::make_signed_t<std::size_t>>(end - start) <= 0)
    {
        throw logic_error("no standard parts in this Debian version; cannot compute upstream start/end.");  // LCOV_EXCL_LINE
    }

    return true;
}


bool debian::next(int pos, trait::pointer_t format)
{
    if(pos < 0)
    {
        throw invalid_parameter("position calling next() cannot be a negative number.");
    }
    if(static_cast<std::size_t>(pos) >= MAX_PARTS)
    {
        throw invalid_parameter(
              "position calling next() cannot be more than "
            + std::to_string(MAX_PARTS)
            + ".");
    }

    std::size_t start(0ULL);
    std::size_t end(0ULL);

    if(!get_upstream_positions(start, end))
    {
        return false;
    }

    pos += start;
    if(end <= static_cast<std::size_t>(pos))
    {
        part zero;
        part alpha;
        do
        {
            part const f(get_format_part(format, end, true));
            if(f.is_integer())
            {
                zero.set_separator(f.get_separator());
                insert(end, zero);
            }
            else
            {
                alpha.set_string(std::string(f.get_string().length(), 'A'));
                alpha.set_separator(f.get_separator());
                insert(end, alpha);
            }
            ++end;
        }
        while(end <= static_cast<std::size_t>(pos));
    }
    for(;;)
    {
        if(at(pos).compare(get_format_part(format, pos, at(pos).is_integer())) == 0)
        {
            if(static_cast<std::size_t>(pos - 1) <= start)
            {
                f_last_error = "maximum limit reached; cannot increment version any further.";
                return false;
            }
            erase(pos);
            --end;
            --pos;
        }
        else
        {
            at(pos).next();
            break;
        }
    }

    // keep part one if it is an integer and the last incremented part
    // was part 0
    //
    if(pos == 0
    && size() >= 2
    && at(1).is_integer())
    {
        at(1).set_integer(0U);
        ++pos;
    }
    ++pos;
    while(static_cast<std::size_t>(pos) < end)
    {
        --end;
        erase(end);
    }

    return true;
}


bool debian::previous(int pos, trait::pointer_t format)
{
    if(pos < 0)
    {
        throw invalid_parameter("position calling previous() cannot be a negative number.");
    }
    if(static_cast<std::size_t>(pos) >= MAX_PARTS)
    {
        throw invalid_parameter(
              "position calling previous() cannot be more than "
            + std::to_string(MAX_PARTS)
            + ".");
    }

    std::size_t start(0);
    std::size_t end(0);

    if(!get_upstream_positions(start, end))
    {
        return false;
    }

    pos += start;
    if(end <= static_cast<std::size_t>(pos))
    {
        part zero;
        zero.set_separator(U'.');
        do
        {
            insert(end, zero);
            ++end;
        }
        while(end <= static_cast<std::size_t>(pos));
    }

    bool result(false);
    for(;;)
    {
        if(at(pos).is_zero())
        {
            if(static_cast<std::size_t>(pos) <= start)
            {
                f_last_error = "minimum limit reached; cannot decrement version any further.";
                return false;
            }
            result = false;
            part const p(get_format_part(format, pos, at(pos).is_integer()));
            if(p.is_integer())
            {
                at(pos).set_integer(p.get_integer());
            }
            else
            {
                at(pos).set_string(p.get_string());
            }
            at(pos).set_separator(p.get_separator());
            --pos;
        }
        else
        {
            at(pos).previous();

            while(at(pos).is_zero()
               && static_cast<std::size_t>(pos + 1) == end)
            {
                erase(pos);
                --end;
                --pos;
            }

            break;
        }
    }
    return true;
}


std::string debian::to_string() const
{
    if(empty())
    {
        f_last_error = "no parts to output.";
        return std::string();
    }

    std::size_t start(0);
    std::size_t end(0);

    get_upstream_positions(start, end);

    std::size_t max(end);
    while(max > start + 2 && at(max - 1).is_zero())
    {
        --max;
    }
    std::string result;
    char32_t sep(U'\0');
    if(is_epoch_required())
    {
        result += at(0).to_string();
        sep = U':';
    }
    for(std::size_t idx(start); idx < max; ++idx)
    {
        if(idx != start)
        {
            sep = at(idx).get_separator();
        }
        if(sep != U'\0')
        {
            result += sep;
        }
        result += at(idx).to_string();
    }
    if(max - start == 1)
    {
        result += ".0";
    }

    // there can also be a release
    //
    for(; end < size(); ++end)
    {
        sep = at(end).get_separator();
        if(sep != '\0')
        {
            result += sep;
        }
        result += at(end).to_string();
    }

    return result;
}


bool debian::is_epoch_required() const
{
    // any parts?
    //
    if(empty())
    {
        return false; // LCOV_EXCL_LINE -- caller returns early if empty()
    }

    // is first part an epoch?
    //
    if(at(0).get_type() != ':')
    {
        return false;
    }

    // is epoch 0?
    //
    if(at(0).get_integer() != 0)
    {
        return true;
    }

    // when 0, we can skip on it only if the upstream version does not include
    // a colon; otherwise it is required to distinguish the upstream colon
    // from an epoch
    //
    std::size_t const max(size());
    for(std::size_t idx(1); idx < max; ++idx)
    {
        if(at(idx).get_type() == '-')
        {
            // release version parts cannot include a ':' so no need to
            // test that section
            //
            break;
        }

        if(!at(idx).is_integer())
        {
            std::string const s(at(idx).get_string());
            if(s.find(':') != std::string::npos)
            {
                // it is required
                //
                return true;
            }
        }
    }

    return false;
}


/** \brief Compare two Debian versions against each other.
 *
 * Debian versions are considered to be composed of three parts:
 *
 * \li epoch
 * \li upstream version
 * \li release version
 *
 * The epoch is 0 by default (if not defined).
 *
 * The upstream version is compared as a list of numbers and strings. It
 * cannot be empty. The upstream version must start with a number.
 *
 * The release is optional and it can include a list of numbers and strings.
 * The release can start with a string or a number.
 *
 * In our case, we do not compare the ':', '.', and '-' characters. These
 * are viewed as separators. This allows us to compare upstream versions
 * against each other without \em leaking the epoch or release elements.
 * However, this is not 100% compatible with the Debian comparator which
 * may match the release version information against the upstream version.
 *
 * As per the sort order definition of the Debian version, strings that
 * contain the '~' character compares before anything else (including the
 * empty string), then letters (A-Z, a-z), then everything else is sorted
 * as per their ASCII code.
 *
 * \note
 * If the right hand side version is not a Debian version, then the default
 * trait compare gets used.
 *
 * \param[in] rhs  The right hand side.
 *
 * \return 0 if both versions are considered equal; -1 if the left
 * hand side is considered smaller and 1 if the left hand side is considered
 * larger.
 *
 * \sa compare_characters()
 */
int debian::compare(trait::pointer_t rhs) const
{
    if(empty() || rhs == nullptr || rhs->empty())
    {
        throw empty_version("one or both of the input versions are empty.");
    }

    pointer_t deb(std::dynamic_pointer_cast<debian>(rhs));
    if(deb == nullptr)
    {
        // mixed versions, use the default compare() function instead
        //
        return trait::compare(rhs);
    }

    std::size_t lpos(0);
    std::size_t rpos(0);

    // compare epoch
    //
    int lepoch(0);
    int repoch(0);
    if(at(0).get_type() == ':')
    {
        lepoch = at(0).get_integer();
        lpos = 1;
    }
    if(deb->at(0).get_type() == ':')
    {
        repoch = deb->at(0).get_integer();
        rpos = 1;
    }
    if(lepoch != repoch)
    {
        return lepoch < repoch ? -1 : 1;
    }

    // compare upstream version, then switch to the release version
    //
    char type('\0');
    for(;;)
    {
        for(bool handle_strings(true);; handle_strings = !handle_strings)
        {
            if((lpos >= size() || at(lpos).get_type() != type)
            && (rpos >= deb->size() || deb->at(rpos).get_type() != type))
            {
                // we reached a different type on both sides
                // we need to explicitly break
                //
                break;
            }

            int lint(0);
            std::string lstr;
            int rint(0);
            std::string rstr;

            if(lpos < size()
            && at(lpos).get_type() == type)
            {
                if(handle_strings ^ at(lpos).is_integer())
                {
                    if(handle_strings)
                    {
                        lstr = at(lpos).get_string();
                    }
                    else
                    {
                        lint = at(lpos).get_integer();
                    }
                    ++lpos;
                }
            }

            if(rpos < deb->size()
            && deb->at(rpos).get_type() == type)
            {
                if(handle_strings ^ deb->at(rpos).is_integer())
                {
                    if(handle_strings)
                    {
                        rstr = deb->at(rpos).get_string();
                    }
                    else
                    {
                        rint = deb->at(rpos).get_integer();
                    }
                    ++rpos;
                }
            }

            if(handle_strings)
            {
                int const r(compare_strings(lstr, rstr));
                if(r != 0)
                {
                    return r;
                }
            }
            else
            {
                if(lint != rint)
                {
                    return lint < rint ? -1 : 1;
                }
            }
        }
        if(type == '-')
        {
            // we are done
            //
            return 0;
        }
        type = '-';
    }
}



}
// namespace versiontheca
// vim: ts=4 sw=4 et
