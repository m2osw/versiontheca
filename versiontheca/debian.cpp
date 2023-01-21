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
#include  <libutf8/iterator.h>
#include  <libutf8/libutf8.h>


// C++
//
#include  <iostream>
#include  <type_traits>


// C
//
//#include    <errno.h>
//#include    <string.h>
//#include    <stdio.h>


// last include
//
#include    <snapdev/poison.h>



namespace versiontheca
{



namespace
{



int compare_characters(char32_t a, char32_t b)
{
    if(a == b)
    {
        return 0;
    }
    if(a == '\0')
    {
        return b == '~' ? 1 : -1;
    }
    if(b == '\0')
    {
        return a == '~' ? -1 : 1;
    }
    if((a >= 'a' && a <= 'z')
    || (a >= 'A' && a <= 'Z'))
    {
        if((b >= 'a' && b <= 'z')
        && (b >= 'A' && b <= 'Z'))
        {
            if(a < b)
            {
                return -1;
            }
            if(a > b)
            {
                return 1;
            }
            return 0;
        }
        // letters are earlier than non-letters except '~'
        return b == '~' ? 1 : -1;
    }
    if((b >= 'a' && b <= 'z')
    || (b >= 'A' && b <= 'Z'))
    {
        return a == '~' ? -1 : 1;
    }
    if(a == '~')
    {
        return -1;
    }
    if(b == '~')
    {
        return 1;
    }
    if(a < b)
    {
        return -1;
    }
    if(a > b)
    {
        return 1;
    }
    return 0;
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


/** \brief Parse a debian version string.
 *
 * A debian version string is composed of three parts:
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
              "invalid ':' and/or '-' positions in \""
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
              "a debian version must always start with a number \""
            + v
            + "\".";
        return false;
    }

    if(dash < v.length())
    {
        // parse the debian revision
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


bool debian::prepare_next_previous(std::size_t & start, std::size_t & end)
{
    std::size_t const max(size());
    if(max == 0ULL)
    {
        f_last_error = "no parts in this Debian version; cannot computer next/previous.";
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
        throw logic_error("no standard parts in this Debian version; cannot computer next/previous.");  // LCOV_EXCL_LINE
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

    if(!prepare_next_previous(start, end))
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
    for(;; --pos)
    {
        if(at(pos) == get_format_part(format, pos, at(pos).is_integer()))
        {
            if(static_cast<std::size_t>(pos - 1) <= start)
            {
                f_last_error = "maximum limit reached; cannot increment version any further.";
                return false;
            }
            erase(pos);
            --end;
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

    if(!prepare_next_previous(start, end))
    {
        return false;
    }

    pos += start;
    if(end <= static_cast<std::size_t>(pos))
    {
        part zero;
        zero.set_integer(0U);
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
    // ignore all .0 at the end except for the minor version
    // (i.e. "1.0" keep that zero)
    //
    std::size_t max(size());
    std::size_t limit(1);
    if(max > 0
    && at(0).get_type() == ':')
    {
        ++limit;
    }
    while(max > limit && at(max - 1).is_zero())
    {
        --max;
    }
    if(max == 0)
    {
        f_last_error = "no parts to output.";
        return std::string();
    }
    std::string result;
    max = std::max(limit + 1, max);
    for(std::size_t idx(0); idx < max; ++idx)
    {
        char const sep(at(idx).get_separator());
        if(sep != '\0')
        {
#ifdef _DEBUG
            if(idx == 0)
            {
                throw logic_error("the very first part should not have a separator defined (it is not supported)."); // LCOV_EXCL_LINE
            }
#endif
            result += sep;
        }
        result += at(idx).to_string();
    }
    return result;
}


/** \brief Compare two debian versions against each other.
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
 * If the right hand side version is not a debian version, then the default
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
    if(empty() || rhs->empty())
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
            int lint(0);
            std::string lstr;
            int rint(0);
            std::string rstr;

            if(lpos >= size())
            {
                if(rpos >= size())
                {
                    // we've reached the end of both lists
                    //
                    break;
                }
            }
            else if(at(lpos).get_type() == type)
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



// struct debian_version_part_t
// {
//     debian_version_part_t()
//         : f_val(-1)
//     {
//     }
// 
//     static int cmp(char a, char b)
//     {
//         if(a == b) 
//         {
//             return 0;
//         }
//         if(a == '\0') 
//         {
//             return b == '~' ? 1 : -1;
//         }
//         if(b == '\0') 
//         {
//             return a == '~' ? -1 : 1;
//         }
//         if(a >= 'a' && a <= 'z') 
//         {
//             if(b >= 'a' && b <= 'z') 
//             {
//                 if(a < b) 
//                 {
//                     return -1;
//                 }
//                 if(a > b) 
//                 {
//                     return 1;
//                 }
//                 return 0;
//             }
//             // letters are earlier than non-letters except '~'
//             return b == '~' ? 1 : -1;
//         }
//         if(b >= 'a' && b <= 'z') 
//         {
//             return a == '~' ? -1 : 1;
//         }
//         if(a == '~') 
//         {
//             return -1;
//         }
//         if(b == '~') 
//         {
//             return 1;
//         }
//         if(a < b) 
//         {
//             return -1;
//         }
//         if(a > b) 
//         {
//             return 1;
//         }
//         return 0;
//     }
// 
//     int compare(const debian_version_part_t& rhs) const
//     {
//         // value comparison?
//         if(f_val != -1) 
//         {
//             if(rhs.f_val == -1) 
//             {
//                 // this is a bug
//                 throw std::logic_error("comparing parts that are not of the same type (1)");
//             }
//             if(f_val < rhs.f_val) 
//             {
//                 return -1;
//             }
//             if(f_val > rhs.f_val) 
//             {
//                 return 1;
//             }
//             return 0;
//         }
//         if(rhs.f_val != -1) 
//         {
//             // this is a bug
//             throw std::logic_error("comparing parts that are not of the same type (2)");
//         }
// 
//         // string comparison is complicated!
//         const char *a(f_str.c_str());
//         const char *b(rhs.f_str.c_str());
//         while(*a != '\0' && *b != '\0') 
//         {
//             int r(cmp(*a, *b));
//             if(r != 0) 
//             {
//                 return r;
//             }
//             ++a;
//             ++b;
//         }
//         // one or the other or both are '\0' which needs
//         // to be compared properly against '~'
//         return cmp(*a, *b);
//     }
// 
//     bool is_zero() const
//     {
//         // ".0" is also viewed as zero
//         return f_val == -1 ? f_str == "." || f_str == "" : f_val == 0;
//     }
// 
//     std::string f_str;
//     /// If -1, then f_str is the ASCII data, otherwise f_val is the decimal value
//     int         f_val;
// };
// 
// 
// namespace
// {
// int fix_version(int c)
// {
//     if(c >= 'A' && c <= 'Z')
//     {
//         return c | 0x020;
//     }
//     if(c == ';')
//     {
//         return ':';
//     }
//     return c;
// }
// }
// 
// 
// /// Private declaration of the Debian version object
// struct debian_version_t
// {
//     /// Initializes the Debian version object
//     debian_version_t(std::string version, std::string& error_msg)
//         : f_epoch(0)
//         //, f_version_parts -- auto-init
//         //, f_revision_parts -- auto-init
//     {
//         // Note: using uppercase for all letters is NOT Debian compatible
//         //       however, MS-Windows makes use of case insensitive filenames
//         //       so it is viewed as safer to have such a test of versions
//         std::transform(version.begin(), version.end(), version.begin(), fix_version);
// 
//         // first ":" separate the epoch from the rest
//         std::string::size_type p(version.find_first_of(":"));
//         if(p != std::string::npos)
//         {
//             if(p == 0)
//             {
//                 error_msg = "empty epoch";
//                 return;
//             }
//             std::string epoch(version.substr(0, p));
//             // remove the epoch
//             version = version.substr(p + 1);
//             p = epoch.find_first_not_of("0123456789");
//             if(p != std::string::npos)
//             {
//                 error_msg = "non-decimal epoch";
//                 return;
//             }
//             f_epoch = 0;
//             for(const char *e(epoch.c_str()); *e != '\0'; ++e)
//             {
//                 // note: we already checked whether all characters were digits
//                 const int old(f_epoch);
//                 f_epoch = f_epoch * 10 + *e - '0';
//                 if(f_epoch < old)
//                 {
//                     // this happens on overflows
//                     error_msg = "invalid decimal epoch";
//                     return;
//                 }
//             }
//         }
// 
//         // now check for a revision
//         p = version.find_last_of("-");
//         if(p != std::string::npos)
//         {
//             if(p + 1 == version.length())
//             {
//                 error_msg = "empty revision";
//                 return;
//             }
//             std::string revision(version.substr(p + 1));
//             // This is not actually true for revisions even though it says so
//             // however they want to support things such as -alpha, -beta, -rc1
//             //if(revision[0] < '0' || revision[0] > '9')
//             //{
//             //    error_msg = "invalid revision, digit expected as first character";
//             //    return;
//             //}
//             version = version.substr(0, p);
//             // revisions do not support colons, use '-' which cannot
//             // be included in that string
//             if(string_to_parts(revision, f_revision_parts, '-') != 0)
//             {
//                 error_msg = "invalid character in revision";
//                 return;
//             }
//         }
// 
//         // now transform the version into parts
//         if(version[0] < '0' || version[0] > '9')
//         {
//             error_msg = "invalid version, digit expected as first character";
//             return;
//         }
//         if(string_to_parts(version, f_version_parts) != 0)
//         {
//             error_msg = "invalid character in version";
//             return;
//         }
//     }
// 
//     /// Break up a string into version parts (number, string, number, ...)
//     int string_to_parts(const std::string& v, std::vector<debian_version_part_t>& parts, char colon = ':')
//     {
//         for(const char *s = v.c_str(); *s != '\0';)
//         {
//             // read the string
//             const char *start(s);
//             for(; *s != '\0' && (*s < '0' || *s > '9'); ++s)
//             {
//                 // digits will not be included here so no need to test them
//                 // characters have been transformed to lowercase
//                 if((*s < 'a' || *s > 'z')
//                 && *s != '-' && *s != '.' && *s != '~' && *s != '+' && *s != colon)
//                 {
//                     return -1;
//                 }
//             }
//             debian_version_part_t ps;
//             ps.f_str.assign(start, s - start);
//             parts.push_back(ps);
//             if(*s == '\0')
//             {
//                 break;
//             }
// 
//             // read the number
//             debian_version_part_t pv;
//             pv.f_val = 0;
//             while(*s >= '0' && *s <= '9')
//             {
//                 pv.f_val = pv.f_val * 10 + *s - '0';
//                 ++s;
//             }
//             parts.push_back(pv);
//         }
//         return 0;
//     }
// 
//     /// compare two versions against each others
//     int compare(const debian_version_t& rhs) const
//     {
//         int    r;
// 
//         // compare the epoch first
//         r = f_epoch - rhs.f_epoch;
//         if(r != 0)
//         {
//             return r < 0 ? -1 : 1;
//         }
// 
//         // compare version sub-parts
//         r = compare_parts(f_version_parts, rhs.f_version_parts);
//         if(r != 0)
//         {
//             return r;
//         }
// 
//         // now compare revision sub-parts
//         return compare_parts(f_revision_parts, rhs.f_revision_parts);
//     }
// 
//     static int compare_parts(const std::vector<debian_version_part_t>& a, const std::vector<debian_version_part_t>& b)
//     {
//         std::vector<debian_version_part_t>::size_type idx;
//         for(idx = 0; idx < a.size() && idx < b.size(); ++idx)
//         {
//             int r(a[idx].compare(b[idx]));
//             if(r != 0)
//             {
//                 return r;
//             }
//         }
//         // test a's if a is longer
//         for(; idx < a.size(); ++idx)
//         {
//             if(!a[idx].is_zero())
//             {
//                 return 1;
//             }
//         }
//         // test b's if b is longer
//         for(; idx < b.size(); ++idx)
//         {
//             if(!b[idx].is_zero())
//             {
//                 return -1;
//             }
//         }
//         return 0;  // equal!
//     }
// 
//     std::string to_string() const
//     {
//         std::stringstream str;
//         std::stringstream ver;
// 
//         // version parts, mandatory
//         parts_to_string(ver, f_version_parts);
// 
//         // put the epoch if there is one (i.e. not "0:")
//         if(f_epoch > 0 || ver.str().find(":") != std::string::npos)
//         {
//             str << f_epoch << ":";
//         }
// 
//         str << ver.str();
// 
//         // revision parts, optional
//         if(f_revision_parts.size() > 0)
//         {
//             // avoid "-0" which is the default
//             if(f_revision_parts.size() != 2
//             || !f_revision_parts[0].is_zero()
//             || !f_revision_parts[1].is_zero())
//             {
//                 str << "-";
//                 parts_to_string(str, f_revision_parts);
//             }
//         }
// 
//         // results
//         return str.str();
//     }
// 
//     static void parts_to_string(std::stringstream& str, const std::vector<debian_version_part_t>& parts)
//     {
//         // remove the .0 at the end (i.e. 1.0.0 -> 1.0)
//         std::vector<debian_version_part_t>::size_type count(parts.size());
//         while(count > 2)
//         {
//             if(parts[count - 1].is_zero())
//             {
//                 --count;
//             }
//             else
//             {
//                 break;
//             }
//         }
//         // concatenate all the parts except ending zeroes
//         for(std::vector<debian_version_part_t>::size_type i = 0; i < count; ++i)
//         {
//             if(parts[i].f_val == -1)
//             {
//                 str << parts[i].f_str;
//             }
//             else
//             {
//                 str << parts[i].f_val;
//             }
//         }
//     }
// 
// private:
//     int                                 f_epoch;
//     std::vector<debian_version_part_t>  f_version_parts;
//     std::vector<debian_version_part_t>  f_revision_parts;
// };
// 
// 
// 
// 
// /** \brief Validates a string as a Debian version.
//  *
//  * This function attempts to transform a string into a Debian
//  * version object. If it works, the function returns 1 (true)
//  * and if it fails, the function returns 0.
//  *
//  * For more information about a Debian version string,
//  * please check out the string_to_debian_version() function.
//  *
//  * \param[in]  string         The string version to validate
//  * \param[out] error_string   A pointer to a buffer where an error string is copied
//  * \param[in]  error_size     The size of the error_string buffer in char
//  *
//  * \return 1 if the string is a valid Debian string; 0 if it is invalid
//  */
// int validate_debian_version(const char *string, char *error_string, size_t error_size)
// {
//     std::string error_msg;
//     const char *errmsg;
// 
//     try
//     {
//         debian_version_t version(string, error_msg);
//         if(error_msg.empty())
//         {
//             // it is valid!
//             return 1;
//         }
//         // an error occurred
//         errmsg = error_msg.c_str();
//     }
//     catch(const std::bad_alloc&)
//     {
//         // this is a C function not expecting a throw
//         errmsg = "out of memory";
//     }
// 
//     // copy error in user's buffer
//     if(error_string != 0 && error_size > 0)
//     {
//         --error_size;
//         strncpy(error_string, errmsg, static_cast<int>(error_size));
//         error_string[error_size] = '\0';
//     }
// 
//     return 0;
// }
// 
// 
// /** \brief Initializes a Debian version object from a string.
//  *
//  * This function transforms a string in a Debian version object.
//  * Note, however, that the versions supported by these functions
//  * are not 100% compatible with Debian versions. For example,
//  * the letters are all tested in a case insensitive manner because
//  * the MS-Windows file system is case insensitive.
//  *
//  * The handle of the object is returned. You can use the handle
//  * to compare versions together and regenerate the string.
//  *
//  * A Debian version is defined as:
//  *
//  *    debian_version: [ epoch ':' ] version [ '-' release ]
//  *
//  *    epoch: '[0-9]+' ':'
//  *
//  *    version: '[0-9]' '[-:.+~0-9a-zA-Z]*'
//  *
//  *    release: '-' '[0-9]' '[.+~0-9a-zA-Z]*'
//  *
//  * The default epoch is 0. Any positive number is valid. The order
//  * is the expected order for a decimal number.
//  *
//  * The default release is 0. All the possible values available in a
//  * version are possible except the dash and colon characters. The
//  * release is tested last if the remainder of the version is equal.
//  *
//  * The version is mandatory, there is no default. Any value is valid.
//  * A version can include letters in lower or upper case. Characters
//  * are compared as case insensitive strings. Digits are taken as
//  * decimal numbers and compared as such. When digits appear in one
//  * version when letters appear in the other, the digits are viewed
//  * as an empty string that is smaller than the letters.
//  *
//  * This function returns NULL whenever the input includes an
//  * invalid character.
//  *
//  * An empty string is not valid because the version part is mandatory.
//  *
//  * Any one part cannot be empty. So version strings such as ":1.3",
//  * "0:", and "1.4-" are invalid. However, a version such as "1.3." is
//  * acceptable. In case of a period, "1.3." is equal to "1.3". Other
//  * characters are not removed however so "1.3+" is larger than "1.3".
//  *
//  * Debian reference:
//  * http://www.debian.org/doc/debian-policy/ch-controlfields.html#s-f-Version
//  *
//  * \param[in]  string         The string version to convert to a Debian version object
//  * \param[out] error_string   A pointer to a buffer where an error string is copied
//  * \param[in]  error_size     The size of the error_string buffer in char
//  *
//  * \return NULL if an error occurs (and the error in error_string if supplied);
//  *       the pointer of a newly initialized Debian version object
//  */
// debian_version_handle_t string_to_debian_version(const char *string, char *error_string, size_t error_size)
// {
//     std::string error_msg;
//     debian_version_handle_t version;
//     const char *errmsg;
// 
//     try
//     {
//         version = new debian_version_t(string, error_msg);
//         if(error_msg.empty())
//         {
//             return version;
//         }
//         // an error occurred, get rid of the version
//         delete version;
// 
//         errmsg = error_msg.c_str();
//     }
//     catch(const std::bad_alloc&)
//     {
//         // this is a C function not expecting a throw
//         errmsg = "out of memory";
//     }
// 
//     if(error_string != 0 && error_size > 0)
//     {
//         --error_size;
//         strncpy(error_string, errmsg, error_size);
//         error_string[error_size] = '\0';
//     }
// 
//     return 0;
// }
// 
// 
// /** \brief Delete a debian object.
//  *
//  * This function deletes a debian object created by the
//  * string_to_debian_version() function.
//  *
//  * \param[in] debian_version    The Debian version to delete
//  */
// void delete_debian_version(debian_version_handle_t debian_version)
// {
//     delete debian_version;
// }
// 
// 
// 
// /** \brief Function used to convert a Debian version object to a string
//  *
//  * This function converts a Debian version object to a string.
//  *
//  * You can call the function once with string set to NULL and string_size
//  * to zero to query the necessary size for your string.
//  *
//  * The result is the size of the string or -1 if an error occurs.
//  *
//  * errno is set to EINVAL if debian_version is a NULL pointer.
//  *
//  * errno is set to ENOMEM if your string buffer is too small to copy the
//  * entire version string.
//  *
//  * \note
//  * This function is a good way to canonicalize a Debian version: (1) you
//  * convert a version string to a Debian version object and (2) you convert
//  * the object back to a string.
//  *
//  * \param[in]  debian_version   The debian version object to convert to a string
//  * \param[out] string           A pointer to a string buffer to get the result
//  * \param[in]  string_size      The size of the string buffer
//  *
//  * \return The size of the string or -1 if an error occurred
//  */
// int debian_version_to_string(const debian_version_handle_t debian_version, char *string, size_t string_size)
// {
//     if(debian_version == 0) 
//     {
//         // no version object
//         errno = EINVAL;
//         return -1;
//     }
// 
//     std::string str = debian_version->to_string();
// 
//     // requesting the size only
//     if(string == 0 || string_size == 0) 
//     {
//         return static_cast<int>(str.size());
//     }
// 
//     if(str.size() >= string_size) 
//     {
//         // buffer to small
//         errno = ENOMEM;
//         return -1;
//     }
// 
//     memcpy(string, str.c_str(), str.size() + 1);
// 
//     return static_cast<int>(str.size());
// }
// 
// 
// 
// /** \brief Function used to compare two Debian versions.
//  *
//  * This function is used to compare two Debian versions against each others.
//  *
//  * \param[in]  left            The left version
//  * \param[in]  right           The right version
//  *
//  * \return -1, 0 or 1 whether left is smaller, equal or larger than right
//  */
// int debian_versions_compare(const debian_version_handle_t left, const debian_version_handle_t right)
// {
//     if(left == 0 || right == 0) 
//     {
//         // no version object
//         errno = EINVAL;
//         return -1;
//     }
// 
//     return left->compare(*right);
// }



}
// namespace versiontheca
// vim: ts=4 sw=4 et
