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
 * \brief Parse and compare versions using Roman numerals.
 *
 * This trait understands Roman numerals and converts them to integers.
 */

// self
//
#include    <versiontheca/roman.h>

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


constexpr char const * const g_thousands[4] = {
    "",
    "M",
    "MM",
    "MMM",
};

constexpr char const * const g_hundreds[10] = {
    "",
    "C",
    "CC",
    "CCC",
    "CD",
    "D",
    "DC",
    "DCC",
    "DCCC",
    "CM"
};

constexpr char const * const g_tens[10] = {
    "",
    "X",
    "XX",
    "XXX",
    "XL",
    "L",
    "LX",
    "LXX",
    "LXXX",
    "XC"
};

constexpr char const * const g_units[10] = {
    "",
    "I",
    "II",
    "III",
    "IV",
    "V",
    "VI",
    "VII",
    "VIII",
    "IX"
};



}
// no name namespace


/** \brief Convert a string of Roman numerals to an integer.
 *
 * This function parses \p value as a Roman number. If valid, the
 * corresponding integer is returned. If an error occurs, zero is
 * returned (i.e. 0 is not a valid Roman number).
 *
 * The largest canonicalized number representable with Roman numerals
 * is 3999. You can use additional M characters to go larger.
 *
 * The function views lowercase and uppercase letters as the same thing.
 *
 * \note
 * The algorithm generally expects well formed Roman numbers and it may
 * fail on \em elaborated numbers. For example, "IIX" could be used to
 * write 8 when the normal way to do so is "VIII". Our algorithm happen
 * to work in this case, but there are certainly other cases that will
 * fail.
 *
 * \return 0 on error, the Roman number on success.
 */
part_integer_t from_roman_number(std::string const & value)
{
    std::size_t const max(value.length());
    if(max == 0)
    {
        // an empty string is not considered valid
        //
        return 0;
    }
    std::vector<int> number(max);
    for(std::size_t idx(0); idx < max; ++idx)
    {
        char c(value[idx]);
        if(c >= 'a' && c <= 'z')
        {
            c -= 0x20; // uppercase
        }
        switch(c)
        {
        case 'I':
            number[idx] = 1;
            break;

        case 'V':
            number[idx] = 5;
            break;

        case 'X':
            number[idx] = 10;
            break;

        case 'L':
            number[idx] = 50;
            break;

        case 'C':
            number[idx] = 100;
            break;

        case 'D':
            number[idx] = 500;
            break;

        case 'M':
            number[idx] = 1000;
            break;

        default:
            // unknown Roman digit
            //
            return 0;

        }
    }
    part_integer_t result(number[max - 1]);
    bool subtract(false);
    for(ssize_t idx(max - 2); idx >= 0; --idx)
    {
        if(number[idx] == number[idx + 1])
        {
            if(subtract)
            {
                result -= number[idx];
            }
            else
            {
                result += number[idx];
            }
        }
        else if(number[idx] < number[idx + 1])
        {
            result -= number[idx];
            subtract = true;
        }
        else
        {
            result += number[idx];
            subtract = false;
        }
    }

    return result;
}


/** \brief Convert \p value to a Roman numeral.
 *
 * This function is the convese of the from_roman_number(). It converts a
 * number back to its Roman numeral form. The number output will be as per
 * the \em normalized version of the Roman numeral (there is not really such
 * a thing, although there are basic rules that are to be applied to get
 * what looks like a standardized Roman numeral).
 *
 * \param[in] value  The integer to transform to Roman numeral.
 *
 * \return A string representing the number in Roman numerals or an empty
 * string if the number is out of bounds (0 or over 3999).
 */
std::string to_roman_number(part_integer_t value)
{
    if(value <= 0 || value > 3'999)
    {
        // out of bounds
        //
        return std::string();
    }
    std::string result(g_thousands[value / 1'000]);
    result += g_hundreds[(value / 100) % 10];
    result += g_tens[(value / 10) % 10];
    result += g_units[value % 10];
    return result;
}



bool roman::parse(std::string const & v)
{
    if(!trait::parse(v))
    {
        return false;
    }

    // check strings for roman numerals, if a string parses as such, then
    // it gets transformed and its type is changed to 'R'
    //
    std::size_t const max(size());
    for(std::size_t idx(0); idx < max; ++idx)
    {
        if(!at(idx).is_integer())
        {
            // get the value from the roman numeral
            //
            // if it fails, we get 0
            //
            // we can generate numbers larger than 3999, but the converse
            // requires us to have a maximum of 3999 so we "fail" if the
            // roman numeral represents a larger number
            //
            part_integer_t const value(from_roman_number(at(idx).get_string()));
            if(value >= 1 && value <= 3999)
            {
                at(idx).set_integer(value);
                at(idx).set_type('R');
            }
        }
    }

    return true;
}


std::string roman::to_string() const
{
    std::size_t max(size());
    if(max == 0)
    {
        f_last_error = "no parts to output.";
        return std::string();
    }
    while(max > 1 && at(max - 1).is_zero())
    {
        --max;
    }
    std::string result;
    for(std::size_t idx(0); idx < max; ++idx)
    {
        char32_t const sep(at(idx).get_separator());
        if(sep != U'\0')
        {
            if(idx == 0)
            {
                throw logic_error("the very first part should not have a separator defined (it is not supported)."); // LCOV_EXCL_LINE
            }
            result += sep;
        }
        if(at(idx).get_type() == 'R')
        {
            result += to_roman_number(at(idx).get_integer());
        }
        else
        {
            result += at(idx).to_string();
        }
    }
    if(max == 1)
    {
        if(size() >= 2
        && !at(1).is_integer())
        {
            result += ".A";
        }
        else
        {
            result += ".0";
        }
    }
    return result;
}



}
// namespace versiontheca
// vim: ts=4 sw=4 et
