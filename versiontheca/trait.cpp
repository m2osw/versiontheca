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
#include    <versiontheca/trait.h>

#include    <versiontheca/exception.h>


// libutf8
//
#include    <libutf8/iterator.h>


// snapdev
//
#include    <snapdev/hexadecimal_string.h>
#include    <snapdev/tokenize_string.h>


// libutf8
//
#include    <libutf8/libutf8.h>


// C++
//
#include    <iostream>


// last include
//
#include    <snapdev/poison.h>



namespace versiontheca
{




trait::~trait()
{
}


void trait::clear()
{
    f_parts.clear();
}


part & trait::at(int index)
{
    return f_parts.at(index);
}


part const & trait::at(int index) const
{
    return f_parts.at(index);
}


part & trait::operator [] (int index)
{
    return f_parts.operator [] (index);
}


part const & trait::operator [] (int index) const
{
    return f_parts.operator [] (index);
}


void trait::push_back(part const & p)
{
    if(f_parts.size() >= MAX_PARTS)
    {
        throw overflow("trying to append more parts when maximum was already reached.");
    }

    f_parts.push_back(p);
}


void trait::insert(int index, part const & p)
{
    if(f_parts.size() >= MAX_PARTS)
    {
        throw overflow("trying to insert more parts when maximum was already reached.");
    }

    f_parts.insert(f_parts.begin() + index, p);
}


void trait::erase(int index)
{
    if(static_cast<std::size_t>(index) >= f_parts.size())
    {
        throw overflow("trying to erase a non-existant part.");
    }

    f_parts.erase(f_parts.begin() + index);
}


std::size_t trait::size() const
{
    return f_parts.size();
}


bool trait::empty() const
{
    return f_parts.empty();
}


void trait::resize(std::size_t sz)
{
    if(sz > MAX_PARTS)
    {
        throw overflow("requested too many parts");
    }

    f_parts.resize(sz);
}


/** \brief Default version parser.
 *
 * Most of the version support use either this parse() function or at least
 * the parse_value() sub-function. First it separates each element by their
 * period (.) character. Then within an element, it separates numbers and
 * letters.
 *
 * Separating letters and numbers is important since the following release
 * canditate notation would otherwise fail:
 *
 * \code
 *    1.3.2-rc3
 *    1.3.2-rc11
 * \endcode
 *
 * Viewing the "-rc3" and "-rc11" as strings would sort the eleventh release
 * candidate before the third. The two above versions become vectors of
 * parts like so:
 *
 * \code
 *     +------------+-------------+---------+
 *     | 1.3.2-rc3  | 1.2.3-rc11  |   Type  |
 *     +------------+-------------+---------+
 *     | 1          | 1           | integer |
 *     | 3          | 3           | integer |
 *     | 2          | 2           | integer |
 *     | -rc        | -rc         | string  |
 *     | 3          | 11          | integer |
 *     +------------+-------------+---------+
 * \endcode
 *
 * As we can see, all the parts except the last one are equal. The last one,
 * 3 is smaller than 11 properly sorting those two versions as expected.
 *
 * \note
 * If your version has special cases to be handled, make sure to override
 * this function and implement your own parser. The parse_value() is separate
 * for that reason since a segment is likely to be parsed the same way in
 * most versions.
 *
 * \param[in] v  The version to be parsed.
 *
 * \return true if all the parts were parsed successfully and the input was
 * not an empty string.
 */
bool trait::parse(std::string const & v)
{
std::cerr << "--- trait::parse() ...\n";
    clear();
    if(v.empty())
    {
        f_last_error = "an empty input string cannot represent a valid version.";
        return false;
    }
    std::list<std::string> numbers;
    snapdev::tokenize_string(numbers, v, { "." });
    char32_t sep(U'\0');
    for(auto const & n : numbers)
    {
        if(n.empty())
        {
            // two period one after the other is not valid ("1..3")
            //
            f_last_error = "two periods found one after the other.";
            return false;
        }
        if(!parse_value(n, sep))
        {
            return false;
        }
        sep = U'.';
    }

    // this should never happen
    //
    if(empty())
    {
        throw logic_error("parse() found no parts even though the list of numbers was not empty."); // LCOV_EXCL_LINE
    }

    return true;
}


bool trait::parse_value(std::string const & value, char32_t sep)
{
    libutf8::utf8_iterator it(value);
    char32_t c(*it);
    while(c != libutf8::EOS)
    {
        if(c >= '0' && c <= '9')
        {
            // read one number (digits)
            //
            std::uint8_t significant_zeroes(0);
            while(c == '0')
            {
                ++significant_zeroes;
                ++it;
                c = *it;
            }
            std::string n;
            for(; c >= '0' && c <= '9'; c = *++it)
            {
                n += c;
            }
            if(n.empty()
            && significant_zeroes > 0)
            {
                --significant_zeroes;
                n += '0';
            }
            part p;
            p.set_significant_zeroes(significant_zeroes);
            p.set_separator(sep);
            if(!p.set_value(n))
            {
                f_last_error = p.get_last_error();
                return false;
            }
            push_back(p);
            sep = '\0';
        }

        {
            // read "letters" (anything but a number in the base parser)
            //
            std::string n;
            while(c != libutf8::EOS && (c < '0' || c > '9'))
            {
                if(!is_valid_character(c))
                {
                    // trait can prevent any characters
                    //
                    f_last_error = "found unexpected character: \\U"
                        + snapdev::int_to_hex(c, true, 6)
                        + " in input.";
                    return false;
                }
                n += libutf8::to_u8string(c);
                ++it;
                c = *it;
            }

            // note: this may be empty if we just read a number not
            //       followed by any letters and in that case we ignore
            //
            if(!n.empty())
            {
                part p;
                p.set_separator(sep);
                p.set_string(n);
                push_back(p);
                sep = U'\0';
            }
        }
    }

    return true;
}


bool trait::is_valid_character(char32_t c) const
{
    if(!libutf8::is_valid_unicode(c, false))
    {
        return false;
    }

    return c != '.';
}


/** \brief Default canonicalization of a version.
 *
 * By default, we generate a string with is composed of each part separated
 * by a period. There are two exception to the separator character:
 *
 * \li an epoch requires a colon after
 * \li a release requires a tilde before
 *
 * \note
 * If the version represents "0.0" then it is likely this function
 * will return an empty string.
 *
 * \return The canonicalized version returned as a string.
 */
std::string trait::to_string() const
{
    // ignore all .0 at the end except for the minor version
    // (i.e. "1.0" keep that zero)
    //
    std::size_t max(size());
    if(max == 0)
    {
        f_last_error = "no parts to output.";
        return std::string();
    }
    std::string result;
    for(std::size_t idx(0); idx < max; ++idx)
    {
        char const sep(at(idx).get_separator());
        if(sep != '\0')
        {
            if(idx == 0)
            {
                throw logic_error("the very first part should not have a separator defined (it is not supported).");
            }
            result += sep;
        }
        result += at(idx).to_string();
    }
    return result;
}


part trait::get_format_part(pointer_t format, int pos, bool integer)
{
    if(format != nullptr
    && static_cast<std::size_t>(pos) < format->size())
    {
        return format->f_parts[pos];
    }

    part maximum;
    if(integer)
    {
        maximum.set_to_max_integer();
    }
    else
    {
        maximum.set_to_max_string();
    }
    return maximum;
}


/** \brief Increment the version by 1.
 *
 * This function increments this version by 1.
 *
 * The specified position (\p pos) parameter represents the part that needs
 * to be incremented. Any part after that position gets removed.
 *
 * If the version currently has less parts, then new parts get added and
 * the last one gets set to \em zero.
 *
 * So for example, if you have version `"1.3"` on input and you want to
 * increment part 2 (non-existant, so it is viewed as `"0"`), the version
 * becomes `"1.3.1"`.
 *
 * If after incrementing the value, it is past the limit, then the version
 * at that position is removed and the next one up gets incremented by 1.
 * So if the \p limit is set to 9999, version `"1.3.9999"` plus one at
 * position 2, the version becomes `"1.4"`.
 *
 * \param[in] pos  The position of the part to update, zero based.
 * \param[in] format  The format defining the maximum value each part can have.
 */
bool trait::next(int pos, pointer_t format)
{
    if(pos < 0)
    {
        throw overflow("position in next() cannot be a negative number.");
    }
    if(static_cast<std::size_t>(pos) >= MAX_PARTS)
    {
        throw overflow(
              "position in next() cannot be more than "
            + std::to_string(pos)
            + ".");
    }

    part zero;
    zero.set_integer(0U);
    zero.set_separator(U'.');
    while(f_parts.size() <= static_cast<std::size_t>(pos))
    {
        f_parts.push_back(zero);
    }
    for(;;)
    {
        if(f_parts[pos] == get_format_part(format, pos, f_parts[pos].is_integer()))
        {
            if(pos == 0)
            {
                f_last_error = "maximum limit reached; cannot increment version any further.";
                return false;
            }
            f_parts.resize(pos);
            // "recursive"
        }
        else
        {
            f_parts[pos].next();
            break;
        }
    }

    // keep part one if it is an integer and the last incremented part
    // was part 0
    //
    if(pos == 0
    && f_parts.size() >= 2
    && f_parts[1].is_integer())
    {
        f_parts[1].set_integer(0);
        ++pos;
    }
    f_parts.resize(pos + 1);

    return true;
}


/** \brief Decrement the version by 1.
 *
 * This function decrements this version by 1.
 *
 * The specified position (\p pos) parameter represents the part that needs
 * to be decremented. Any part after that position gets removed.
 *
 * If the version currently has less parts, then new parts get added and
 * the last one gets set to \p limit. This is used when we reach zero at the
 * specified position.
 *
 * So for example, if you have version `"1.0"` on input and you want to
 * decrement part 1 (the `"0"`), the version because `"0.<limit>"`. So if
 * you set \p limit to 9999, now the version is `"0.9999"`. This would also
 * mean that you expect the maximum version number to be 9999 and after that
 * you'd anyway increment the next number (so when incrementing `"0.9999"`,
 * you expect the version to be come `"1.0"` again).
 *
 * \param[in] pos  The position of the part to update, zero based.
 * \param[in] format  The format defining the maximum value each part can have.
 */
bool trait::previous(int pos, pointer_t format)
{
    if(pos < 0)
    {
        throw overflow("position in previous() cannot be a negative number.");
    }
    if(static_cast<std::size_t>(pos) >= MAX_PARTS)
    {
        throw overflow(
              "position in previous() cannot be more than "
            + std::to_string(pos)
            + ".");
    }

    bool result(false);
    for(;;)
    {
        while(static_cast<std::size_t>(pos) > f_parts.size())
        {
            part z;
            z.set_integer(0U);
            z.set_separator(U'.');
            f_parts.push_back(z);
        }
        if(static_cast<std::size_t>(pos) == f_parts.size())
        {
            result = true;
            f_parts.push_back(format->at(pos));
        }
        else if(f_parts[pos].is_zero())
        {
            if(pos == 0)
            {
                f_last_error = "minimum limit reached; cannot decrement version any further.";
                return false;
            }
            result = false;
            f_parts[pos] = format->at(pos);
            --pos;
            continue;
        }
        else
        {
            f_parts[pos].previous();
        }
    }

    return true;
}


std::string trait::get_last_error(bool clear) const
{
    std::string const result(f_last_error);
    if(clear)
    {
        f_last_error.clear();
    }
    return result;
}



}
// namespace versiontheca
// vim: ts=4 sw=4 et
