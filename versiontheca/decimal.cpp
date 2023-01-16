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
 * \brief Parse and compare decimal based versions.
 *
 * A decimal based version is a version which is formed with two numbers
 * separated by a decimal period (.) character. It can then be retrieved
 * as a floating point number.
 *
 * \note
 * It is possible that the result will not be exactly correct when retrieved
 * as a floating point. Internally, though, the version is kept as two
 * separate integers which are always perfectly defined.
 */

// self
//
#include    <versiontheca/decimal.h>



// C++
//
#include    <cmath>
#include    <sstream>


// last include
//
#include    <snapdev/poison.h>



namespace versiontheca
{



bool decimal::parse(std::string const & v)
{
    if(!trait::parse(v))
    {
        return false;
    }

    // decimal version must be:
    //
    // 1. one or two parts
    // 2. separated by a period
    // 3. integers
    //
    // note: since we limit characters to only digits, clearly the parts
    //       should already be integers or an error occurred earlier
    //
    return (size() == 1 || size() == 2)
        && at(0).is_integer()
        && (size() == 1
            || (at(1).get_separator() == '.' && at(1).is_integer()));
}


bool decimal::is_valid_character(char32_t c) const
{
    return c >= '0' && c <= '9';
}


std::string decimal::to_string() const
{
    std::stringstream ss;
    ss << get_decimal_version();
    return ss.str();
}


/** \brief Get the version as a floating point.
 *
 * This function converts the version of one or two parts in a floating
 * pointer number.
 *
 * If the version is considered invalid, this function returns a NaN.
 * You can verify such using the std::isnan() function.
 *
 * \note
 * Keep in mind that decimal numbers are not always properly represented
 * by floating point numbers. In most cases, for numbers with 2 or 3 digits
 * after the decimal point, it is likely to work as expected. Just don't
 * use the `==` to compare such numbers. Instead, look at our nearly_equal()
 * template function in snapcatch2/snapcatch2.hpp for an example of properly
 * comparing two floating point numbers of any precision.
 *
 * \return The version as a floating point double.
 */
double decimal::get_decimal_version() const
{
    if(size() > 0)
    {
        double version(at(0).get_integer());
        if(size() == 2)
        {
            double const significant_zeroes(at(1).get_significant_zeroes());
            double const fraction(at(1).get_integer());
            double const digits(1 + static_cast<int>(log(fraction)) + significant_zeroes);
            version += fraction * pow(10, -digits);
        }
        return version;
    }

    return std::numeric_limits<double>::quiet_NaN();
}



}
// namespace versiontheca
// vim: ts=4 sw=4 et
