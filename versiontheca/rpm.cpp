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
 * \brief Parse and compare RPM compatible versions.
 *
 * RPM versions are very well defined to support a limited set of
 * characters which can be compared with well defined expected results.
 * The functions available here implement the RPM algorithm as defined
 * in the RPM manual. It includes all the features to the letter because
 * we assume that fully supporting the version is of major importance.
 *
 * However, the colon (:) character is not supported in a filename under
 * the MS-Windows file systems. For this reason we have one exception: we
 * support a semi-colon (;) as an exact equivalent of a colon. This is fine
 * because by default the semi-colon is not considered valid in a filename.
 */

// self
//
#include    <versiontheca/rpm.h>

#include    <versiontheca/exception.h>


// C++
//
#include    <iostream>
#include    <type_traits>


// snapdev
//
#include    <snapdev/not_reached.h>


// last include
//
#include    <snapdev/poison.h>



namespace versiontheca
{



namespace
{



#include    <versiontheca/rpm_order_table.ci>

int compare_characters(char32_t a, char32_t b)
{
    // see order in rpm_order.cpp
    //
    int const lidx(static_cast<int>(static_cast<std::uint8_t>(a)));
    int const ridx(static_cast<int>(static_cast<std::uint8_t>(b)));
#ifdef _DEBUG
    if(g_rpm_compare_characters[lidx] == 0
    || g_rpm_compare_characters[ridx] == 0)
    {
        // LCOV_EXCL_START
        throw logic_error("index ("
            + std::to_string(lidx)
            + " and/or "
            + std::to_string(ridx)
            + ") in rpm/compare_characters() hit a 0, which means it represents an invalid character.");
        // LCOV_EXCL_STOP
    }
#endif
    int const r(g_rpm_compare_characters[lidx] - g_rpm_compare_characters[ridx]);
    return r == 0 ? 0 : (r < 0 ? -1 : 1);
}


int compare_strings(std::string const & lhs, std::string const & rhs)
{
    // because of the '~' we have to compare everything ('~' is before
    // '\0'...)
    //
    std::size_t const max(std::max(lhs.length(), rhs.length()));
    std::size_t lidx(0);
    std::size_t ridx(0);
    while(lidx < max || ridx < max)
    {
        char a('\0');
        do
        {
            if(lidx < lhs.length())
            {
                a = lhs[lidx];
            }
            ++lidx;
        }
        while(a == '_' && lidx < lhs.length());
        if(a == '_')
        {
            a = '\0';
        }

        char b('\0');
        do
        {
            if(ridx < rhs.length())
            {
                b = rhs[ridx];
            }
            ++ridx;
        }
        while(b == '_' && ridx < rhs.length());
        if(b == '_')
        {
            b = '\0';
        }

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


/** \brief Parse an RPM version string.
 *
 * A RPM version string is composed of three parts:
 *
 * * Epoch -- a number followed by a colon (:)
 * * Upstream Version -- numbers, letters, and . + - : ~
 * * RPM-revision -- additional parts after the last hyphen
 *
 * The upstream version may include : only if the version includes an
 * epoch. Similarly, it can include a dash if there is an RPM revision.
 * To parse the version we first search the first colon (:) and parse
 * anything before that as the epoch. Then we search the last dash (-)
 * and parse that as the RPM revision. In between, the parse the
 * rest as a standard version string that can include those two special
 * characters.
 *
 * \param[in] v  The version to parse.
 *
 * \return true if the parser succeeded.
 */
bool rpm::parse(std::string const & v)
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
    if(!trait::parse_version(upstream_version, colon == 0 ? U'\0' : U':'))
    {
        return false;
    }

    if(dash < v.length())
    {
        // parse the revision
        //
        std::size_t idx(size());
        if(!trait::parse_version(v.substr(dash + 1), U'-'))
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


bool rpm::is_valid_character(char32_t c) const
{
    return (c >= U'0' && c <= U'9')
        || (c >= U'A' && c <= U'Z')
        || (c >= U'a' && c <= U'z')
        || c == U'~'
        || c == U'^'
        || c == U'_';
}


bool rpm::is_separator(char32_t c) const
{
    return c == U'+'
        || c == U'.';
}


bool rpm::get_upstream_positions(std::size_t & start, std::size_t & end) const
{
    std::size_t const max(size());
    if(max == 0ULL)
    {
        f_last_error = "no parts in this RPM version; cannot compute upstream start/end.";
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
        throw logic_error("no standard parts in this RPM version; cannot compute upstream start/end.");  // LCOV_EXCL_LINE
    }

    return true;
}


bool rpm::next(int pos, trait::pointer_t format)
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


bool rpm::previous(int pos, trait::pointer_t format)
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


std::string rpm::to_string() const
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
    if(at(0).get_type() == ':'
    && !at(0).is_zero())
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


/** \brief Compare two RPM versions against each other.
 *
 * RPM versions are considered to be composed of three parts:
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
 * In our case, we do not compare the '.', '+', and '-' characters. These
 * are viewed as separators. This allows us to compare upstream versions
 * against each other without \em leaking the release elements.
 * However, this is not 100% compatible with the RPM comparator which
 * matches the release version information against the upstream version.
 *
 * As per the sort order definition of the RPM version, the '~' character
 * compares before anything else (including the empty string), then letters
 * (A-Z, a-z), then the `+`, then the `^`. The `_` is ignored while sorting
 * (as if it wasn't there, so compare with or without the `_` return the
 * same result).
 *
 * If a part is an integer and the other a string, then the integer is
 * considered larger than the string ("a" < 1).
 *
 * \note
 * If the right hand side version is not an RPM version, then the default
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
int rpm::compare(trait::pointer_t rhs) const
{
    if(empty() || rhs == nullptr || rhs->empty())
    {
        throw empty_version("one or both of the input versions are empty.");
    }

    pointer_t right(std::dynamic_pointer_cast<rpm>(rhs));
    if(right == nullptr)
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
    if(right->at(0).get_type() == ':')
    {
        repoch = right->at(0).get_integer();
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
        for(;;)
        {
            if((lpos >= size() || at(lpos).get_type() != type)
            && (rpos >= right->size() || right->at(rpos).get_type() != type))
            {
                // we reached a different type on both sides
                // we need to explicitly break
                //
                break;
            }

            int lint(0);
            int rint(0);
            bool linteger(false);
            bool rinteger(false);
            std::string lstr;
            std::string rstr;

            if(lpos < size()
            && at(lpos).get_type() == type)
            {
                linteger = at(lpos).is_integer();
                if(linteger)
                {
                    lint = at(lpos).get_integer();
                }
                else
                {
                    lstr = at(lpos).get_string();
                }
                ++lpos;
            }

            if(rpos < right->size()
            && right->at(rpos).get_type() == type)
            {
                rinteger = right->at(rpos).is_integer();
                if(rinteger)
                {
                    rint = right->at(rpos).get_integer();
                }
                else
                {
                    rstr = right->at(rpos).get_string();
                }
                ++rpos;
            }

            if(linteger == rinteger)
            {
                if(linteger)
                {
                    if(lint < rint)
                    {
                        return -1;
                    }
                    if(lint > rint)
                    {
                        return 1;
                    }
                }
                else
                {
                    int const r(compare_strings(lstr, rstr));
                    if(r != 0)
                    {
                        return r;
                    }
                }
            }
            else
            {
                if(linteger)
                {
                    if(lint != 0 || !rstr.empty())
                    {
                        return 1;
                    }
                }
                else
                {
                    if(rint != 0 || !lstr.empty())
                    {
                        return -1;
                    }
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
