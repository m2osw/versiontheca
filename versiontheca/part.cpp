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
#include    <versiontheca/part.h>

#include    <versiontheca/exception.h>


// libutf8
//
#include    <libutf8/libutf8.h>


// snapdev
//
//#include    <snapdev/not_reached.h>


// C++
//
//#include    <cstdint>
//#include    <string>
//#include    <vector>


// last include
//
#include    <snapdev/poison.h>



namespace versiontheca
{



/** \brief Define the separator.
 *
 * By default, the part separator is set to '\\0' (i.e. no separator). You
 * can call this function to change this value to any other character.
 * The most common character is the period (.). Debian also makes use of
 * the colon (:) for the epoch, the dash (-) for the revision, and the
 * tilde (~) for the release.
 *
 * \note
 * In a vector of parts, the first part is not expected to have a
 * separator (i.e. keep U'\\0').
 *
 * \exception invalid_parameter
 * The separator cannot be set to a control character other than U'\\0'.
 * If that happens, then this exception is raised. Also invalid UTF-32
 * characters are rejected (i.e. surrogates).
 *
 * \param[in] separator  The separator appearing before this part. If U'\\0'
 * then there was no separator (i.e. in "rc1", the "1" part receives U'\\0'
 * as its separator).
 *
 * \sa get_separator()
 */
void part::set_separator(char32_t separator)
{
    if(separator != L'\0'
    && !libutf8::is_valid_unicode(separator, false))
    {
        throw invalid_parameter("separator cannot be a control other than U'\\0' or a surrogate.");
    }

    f_separator = separator;
}


/** \brief Define the number of significant zeroes.
 *
 * In a version such as 1.001, the number of significant zereos is 2. This
 * is useful for versions that view their numbers as a decimal number. In
 * other words, when 1.1 > 1.001 because the first version represents
 * 1.100 and 100 > 001.
 *
 * \param[in] significant_zeroes  The number of zeroes left of the first
 * non-zero digit in this number.
 */
void part::set_significant_zeroes(std::uint8_t significant_zeroes)
{
    f_significant_zeroes = significant_zeroes;
}


/** \brief Type the part.
 *
 * By default a part type is '\0'. You may change the type to a different
 * type using this function. For example, the Debian version system uses
 * 'E' for the Epoch part (the very first part, if defined).
 *
 * \param[in] type  The type to save in this part.
 */
void part::set_type(char type)
{
    f_type = type;
}


/** \brief Set the value as either a string or an integer.
 *
 * If the input string, \p value, is only composed of digits (0-9), then
 * save the value as an integer. In all other cases, save the value as
 * a string. Note that means you can insert a string starting with
 * digits.
 *
 * \exception overflow
 * This overflow exception is raised if \p value is composed of a number
 * larger than part_integer_t can hold (2^32-1). To avoid this error, you
 * can try the set_string_value() instead.
 *
 * \param[in] value  The value to save in this part.
 *
 * \return true if the value is considered valid and the part was properly
 * defined from it.
 */
bool part::set_value(std::string const & value)
{
    part_integer_t integer(0);
    for(char const * s(value.c_str()); *s != '\0'; ++s)
    {
        if(*s >= '0' && *s <= '9')
        {
            part_integer_t const old(integer);
            integer *= 10;
            integer += *s - '0';
            if(integer < old)
            {
                // note: if you want to accept really large numbers as strings
                //       then make sure to use the set_string_value() instead
                //
                f_last_error = "integer too large for a valid version.";
                return false;
            }
        }
        else
        {
            set_string(value);
            return true;
        }
    }
    set_integer(integer);
    return true;
}


void part::set_string(std::string const & value)
{
    f_is_integer = false;
    f_integer = 0;
    f_string = value;
}


void part::set_to_max_string(std::size_t len)
{
    set_string(std::string(len, 'z'));
}


void part::set_integer(part_integer_t const value)
{
    f_is_integer = true;
    f_integer = value;
    f_string.clear();
}


void part::set_to_max_integer()
{
    set_integer(std::numeric_limits<decltype(f_integer)>::max());
}


/** \brief Compute the next version of this part.
 *
 * This function calculates the next version for this specific part.
 *
 * If the part is an integer, it simply gets incremented by one. The
 * function prevents the increment if the integer is already at its
 * maximum (no wrap around allowed).
 *
 * If the part is a string, then lowercase letters get \em increment.
 * That means 'a' becomes 'b', 'b' becomes 'c', etc. and 'z' becomes
 * 'a' and the next letter to the left is incremented. If there are
 * no more letters to the left, then nothing happens and the function
 * returns false.
 *
 * \return false if the increment could not be computed, true if the
 * next() function succeeded.
 */
bool part::next()
{
    if(f_is_integer)
    {
        if(f_integer >= std::numeric_limits<decltype(f_integer)>::max())
        {
            return false;
        }
        ++f_integer;
        return true;
    }
    else
    {
        std::string s(f_string);
        std::size_t pos(s.length());
        while(pos > 0)
        {
            --pos;
            if(s[pos] >= 'a' && s[pos] < 'z')
            {
                ++s[pos];
                break;
            }
            if(s[pos] == 'z')
            {
                s[pos] = 'a';
            }
        }
        if(s <= f_string)
        {
            // nothing good happened
            //
            return false;
        }
        f_string = s;
        return true;
    }
}


bool part::previous()
{
    if(f_is_integer)
    {
        if(f_integer <= std::numeric_limits<decltype(f_integer)>::min())
        {
            return false;
        }
        --f_integer;
        return true;
    }
    else
    {
        std::string s(f_string);
        std::size_t pos(s.length());
        while(pos > 0)
        {
            --pos;
            if(s[pos] > 'a' && s[pos] <= 'z')
            {
                --s[pos];
                break;
            }
            if(s[pos] == 'a')
            {
                s[pos] = 'z';
            }
        }
        if(s >= f_string)
        {
            // nothing good happened
            //
            return false;
        }
        f_string = s;
        return true;
    }
}


char32_t part::get_separator() const
{
    return f_separator;
}


std::uint8_t part::get_significant_zeroes() const
{
    return f_significant_zeroes;
}


char part::get_type() const
{
    return f_type;
}


bool part::is_integer() const
{
    return f_is_integer;
}


std::string part::get_string() const
{
    if(f_is_integer)
    {
        throw wrong_type("this part is not a string.");
    }

    return f_string;
}


part_integer_t part::get_integer() const
{
    if(!f_is_integer)
    {
        throw wrong_type("this part is not an integer.");
    }

    return f_integer;
}


std::string part::to_string() const
{
    if(f_is_integer)
    {
        return std::to_string(f_integer);
    }
    else
    {
        return f_string;
    }
}


std::string part::get_last_error(bool clear) const
{
    std::string last_error(f_last_error);
    if(clear)
    {
        f_last_error.clear();
    }
    return last_error;
}


bool part::is_zero() const
{
    if(f_is_integer)
    {
        return f_integer == 0;
    }
    else
    {
        for(auto const & c : f_string)
        {
            if(c != 'a')
            {
                return false;
            }
        }
        return true;
    }
}


int part::compare(part const & rhs) const
{
    // if both are integers, compare as integers
    //
    if(f_is_integer && rhs.f_is_integer)
    {
        if(f_integer < rhs.f_integer)
        {
            return -1;
        }
        if(f_integer > rhs.f_integer)
        {
            return 1;
        }
        return 0;
    }

    // otherwise fallback to comparing as strings
    //
    std::string ls(to_string());
    std::string rs(rhs.to_string());
    if(ls < rs)
    {
        return -1;
    }
    if(ls > rs)
    {
        return 1;
    }
    return 0;
}


bool part::operator == (part const & rhs) const
{
    return compare(rhs) == 0;
}


bool part::operator != (part const & rhs) const
{
    return compare(rhs) != 0;
}


bool part::operator < (part const & rhs) const
{
    return compare(rhs) < 0;
}


bool part::operator <= (part const & rhs) const
{
    return compare(rhs) <= 0;
}


bool part::operator > (part const & rhs) const
{
    return compare(rhs) > 0;
}


bool part::operator >= (part const & rhs) const
{
    return compare(rhs) >= 0;
}



}
// namespace versiontheca
// vim: ts=4 sw=4 et
