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



bool debian::parse(std::string const & v)
{
std::cerr << "--- do we have a problem here?! debian::parse() called!!!\n";
    char32_t sep(U'\0');
    libutf8::utf8_iterator it(v);
    char32_t c(U'\0');
    std::string epoch;
    for(;; ++it)
    {
        c = *it;
        if(c < U'0' || c > U'9')
        {
            break;
        }
        epoch += c;
    }
    if(c == ':')
    {
        // we've got an epoch
        //
        part p;
        if(!p.set_value(epoch))
        {
            f_last_error = p.get_last_error();
            return false;
        }
        p.set_type(':');
        push_back(p);

        // skip the ':'
        //
        ++it;
        sep = ':';
    }
    else
    {
        // no epoch, restore anything we've parsed so far
        //
        it.rewind();
        c = *it;
    }

    std::string value;
    for(;; ++it)
    {
        c = *it;
        if(c == libutf8::EOS || c == U'-' || c == U'~' || c == U'.')
        {
            if(value.empty())
            {
                // this happens if you have two periods one after the other
                //
                f_last_error =
                      "found an empty part in \""
                    + v
                    + "\".";
                return false;
            }

            if(!parse_value(value, sep))
            {
                return false;
            }
            value.clear();

            if(c != U'.')
            {
                break;
            }

            sep = U'.';
        }
        else
        {
            value += libutf8::to_u8string(c);
        }
    }

    if(c == U'-')
    {
        for(++it;; ++it)
        {
            c = *it;
            if(c == libutf8::EOS || c == U'~')
            {
                break;
            }
            value += libutf8::to_u8string(c);
        }
        if(value.empty())
        {
            // this happens if you have two periods one after the other
            //
            f_last_error =
                  "found an empty part near '-' in \""
                + v
                + "\".";
            return false;
        }
        std::size_t pos(size());
        if(!parse_value(value, U'-'))
        {
            return false;
        }
        for(; pos < size(); ++pos)
        {
            at(pos).set_type('-'); // revision
        }
        value.clear();
    }

    if(c == U'~')
    {
        bool is_integer(true);
        for(++it;; ++it)
        {
            c = *it;
            if(c == libutf8::EOS)
            {
                break;
            }
            if(c < '0' || c > '9')
            {
                is_integer = false;
            }
            value += libutf8::to_u8string(c);
        }
        if(value.empty())
        {
            // this happens if you have two periods one after the other
            //
            f_last_error =
                  "found an empty part near '-' in \""
                + v
                + "\".";
            return false;
        }

        // TBD: we may still need to call parse_value() here?
        //
        part p;
        if(is_integer)
        {
            // set_value() will convert the integer to a number and
            // verify overflows
            //
            p.set_value(value);
        }
        else
        {
            p.set_string(value);
        }
        p.set_type('~');
        p.set_separator(U'~');
        push_back(p);
    }

    if(c != libutf8::EOS)
    {
        f_last_error =
              "found unexpected characters in \""
            + v
            + "\".";
        return false;
    }

    return true;
}


bool debian::is_valid_character(char32_t c) const
{
    // in debian the acceptable characters are very limited
    //
    return (c >= U'0' && c <= U'9')
        || (c >= U'a' && c <= U'z')
        || c == U'+'
        || c == U'.';       // release and revision support periods within their strings
}


bool debian::prepare_next_previous(std::size_t & start, std::size_t & end)
{
    std::size_t const max(size());
    if(max == 0ULL)
    {
        f_last_error = "no parts in this Debian version. Cannot computer next().";
        return false;
    }

    start = 0;
    end = size();
    for(std::size_t idx(0); idx < size(); ++idx)
    {
        if(at(idx).get_type() == ':')
        {
            start = idx + 1;
        }
        else if(at(idx).get_type() == '-'
             || at(idx).get_type() == '~')
        {
            end = idx;
            break;
        }
    }

    if(static_cast<std::make_signed_t<std::size_t>>(end - start) <= 0)
    {
        f_last_error = "no standard parts in this Debian version. Cannot computer next().";
        return false;
    }

    return true;
}


bool debian::next(int pos, trait::pointer_t format)
{
    if(pos < 0)
    {
        throw invalid_parameter("position calling next_version() cannot be a negative number.");
    }
    if(static_cast<std::size_t>(pos) >= MAX_PARTS)
    {
        throw invalid_parameter(
              "position calling next_version() cannot be more than "
            + std::to_string(MAX_PARTS)
            + ".");
    }

    std::size_t start(0ULL);
    std::size_t end(0ULL);

std::cerr << "--- we've go in debian next() " << size() << "!\n";
    if(!prepare_next_previous(start, end))
    {
std::cerr << "--- previous/end fails?\n";
        return false;
    }

std::cerr << "--- try some zero insert\n";
    part zero;
    zero.set_separator(U'.');
    while(end <= static_cast<std::size_t>(pos))
    {
        insert(end, zero);
        ++end;
    }
std::cerr << "--- added some entries? " << pos << " < " << size() << "\n";
    for(;; --pos)
    {
std::cerr << "--- ready...\n";
std::cerr << "--- at(pos) -> " << at(pos).is_integer() << "...\n";
        if(at(pos) == get_format_part(format, pos, at(pos).is_integer()))
        {
            if(static_cast<std::size_t>(pos) <= start)
            {
std::cerr << "--- max. reached?! " << size() << "!\n";
                f_last_error = "maximum limit reached; cannot increment version any further.";
                return false;
            }
            erase(pos);
            // "recursive"
        }
        else
        {
std::cerr << "--- do ++ but is that const?! " << at(pos).to_string() << "!\n";
            at(pos).next();
            break;
        }
    }

std::cerr << "--- finsihing up\n";
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
    resize(pos + 1);

    return true;
}


bool debian::previous(int pos, trait::pointer_t format)
{
    if(pos < 0)
    {
        throw overflow("position in previous_version() cannot be a negative number.");
    }
    if(static_cast<std::size_t>(pos) >= MAX_PARTS)
    {
        throw overflow(
              "position in previous_version() cannot be more than "
            + std::to_string(pos)
            + ".");
    }

    std::size_t start(0);
    std::size_t end(0);

    if(!prepare_next_previous(start, end))
    {
        return false;
    }

    bool result(false);
    for(;;)
    {
        while(static_cast<std::size_t>(pos) > end)
        {
            part zero;
            zero.set_integer(0U);
            zero.set_separator(U'.');
            insert(end, zero);
        }
        if(static_cast<std::size_t>(pos) == size())
        {
            result = true;
std::cerr << "--- got format? "
<< std::boolalpha << (format != nullptr) << " -> "
<< (format == nullptr ? std::string("<null>") : format->to_string()) << "\n";
            part p(get_format_part(format, pos, true));
            p.set_separator(U'.');
            push_back(p);
            --pos;
        }
        else if(at(pos).is_zero())
        {
            if(pos == 0)
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
                throw logic_error("the very first part should not have a separator defined (it is not supported).");
            }
#endif
            result += sep;
        }
        result += at(idx).to_string();
    }
    return result;
}



// /// Structure used to hold all the sub-parts of a part (numbers/alpha)
// struct debian_version_part_t
// {
//     debian_version_part_t()
//         //: f_str -- auto-init
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
