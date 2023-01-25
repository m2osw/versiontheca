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
#include    <libutf8/base.h>
#include    <libutf8/iterator.h>
#include    <libutf8/libutf8.h> // for the std::string += char32_t


// snapdev
//
#include    <snapdev/hexadecimal_string.h>
#include    <snapdev/not_reached.h>


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


void trait::push_back(part const & p)
{
    if(f_parts.size() >= MAX_PARTS)
    {
        throw invalid_parameter("trying to append more parts when maximum was already reached.");
    }

    f_parts.push_back(p);
}


void trait::insert(int index, part const & p)
{
    if(f_parts.size() >= MAX_PARTS)
    {
        throw invalid_parameter("trying to insert more parts when maximum was already reached.");
    }

    f_parts.insert(f_parts.begin() + index, p);
}


void trait::erase(int index)
{
    if(static_cast<std::size_t>(index) >= f_parts.size())
    {
        throw invalid_parameter("trying to erase a non-existant part.");
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
        throw invalid_parameter("requested too many parts.");
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
    clear();
    if(v.empty())
    {
        f_last_error = "an empty input string cannot represent a valid version.";
        return false;
    }

    return parse_version(v, U'\0');
}


bool trait::parse_version(std::string const & v, char32_t sep)
{
    libutf8::utf8_iterator it(v);
    std::string value;
    for(char32_t c(*it); c != libutf8::EOS; ++it, c = *it)
    {
        if(c == libutf8::NOT_A_CHARACTER)
        {
            f_last_error = "input string includes an invalid code not representing a valid UTF-8 character.";
            return false;
        }
        if(is_separator(c))
        {
            if(!parse_value(value, sep))
            {
                return false;
            }
            sep = c;
            value.clear();
        }
        else
        {
            value += c;
        }
    }
    return parse_value(value, sep);
}


bool trait::parse_value(std::string const & value, char32_t sep)
{
    if(value.empty())
    {
        // this happens in cases such as two periods one after the
        // other ("1..3"); with a debian version, it happens when
        // you pass a string without an upstream version ("3:-ubuntu3")
        //
        f_last_error = "a version value cannot be an empty string.";
        return false;
    }
    libutf8::utf8_iterator it(value);
    char32_t c(*it);
    while(c != libutf8::EOS)
    {
        if(c >= '0' && c <= '9')
        {
            // read one number (digits)
            //
            std::string n;
            for(; c >= '0' && c <= '9'; c = *++it)
            {
                n += c;
            }
            part p;
            if(!p.set_value(n))
            {
                f_last_error = p.get_last_error();
                return false;
            }
            p.set_width(n.length());    // TODO: use format length when available
            p.set_separator(sep);
            push_back(p);
            sep = '\0';
        }

        {
            // read "letters" (anything but a number in the base parser)
            //
            std::string n;
            while(c != libutf8::EOS && (c < '0' || c > '9'))
            {
                if(c == libutf8::NOT_A_CHARACTER)
                {
                    f_last_error = "input string includes an invalid code not representing a valid UTF-8 character.";
                    return false;
                }
                if(!is_valid_character(c))
                {
                    // trait can prevent any characters
                    //
                    f_last_error = "found unexpected character: \\U"
                        + snapdev::int_to_hex(c, true, 6)
                        + " in input.";
                    return false;
                }
                n += c;
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

    return c != U'.';
}


bool trait::is_separator(char32_t c) const
{
    return c == U'.';
}


int trait::compare(trait::pointer_t rhs) const
{
    if(empty() || rhs == nullptr || rhs->empty())
    {
        throw empty_version("one or both of the input versions are empty.");
    }

    std::size_t const max(std::max(size(), rhs->size()));
    for(std::size_t idx(0); idx < max; ++idx)
    {
        if(idx >= size())
        {
            if(idx < rhs->size()
            && !rhs->f_parts[idx].is_zero())
            {
                return -1;
            }
        }
        else if(idx >= rhs->size())
        {
            if(!f_parts[idx].is_zero())
            {
                return 1;
            }
        }
        else
        {
            int const r(f_parts[idx].compare(rhs->f_parts[idx]));
            if(r != 0)
            {
                return r;
            }
        }
    }
    return 0;
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
        result += at(idx).to_string();
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
        if(pos != 0)
        {
            maximum.set_separator(U'.');
        }
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
 *
 * \return true if the next version was successfully computed.
 */
bool trait::next(int pos, pointer_t format)
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

    if(static_cast<std::size_t>(pos) >= f_parts.size())
    {
        part zero;
        part alpha;
        do
        {
            part const f(get_format_part(format, f_parts.size(), true));
            if(f.is_integer())
            {
                zero.set_separator(f.get_separator());
                f_parts.push_back(zero);
            }
            else
            {
                alpha.set_string(std::string(f.get_string().length(), 'A'));
                alpha.set_separator(f.get_separator());
                f_parts.push_back(alpha);
            }
        }
        while(static_cast<std::size_t>(pos) >= f_parts.size());
    }
    for(;;)
    {
        if(f_parts[pos].compare(get_format_part(format, pos, f_parts[pos].is_integer())) == 0)
        {
            if(pos == 0)
            {
                f_last_error = "maximum limit reached; cannot increment version any further.";
                return false;
            }
            erase(pos);
            --pos;
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
        f_parts[1].set_integer(0U);
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
 *
 * \return true if the previous version was successfully computed.
 */
bool trait::previous(int pos, pointer_t format)
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

    if(static_cast<std::size_t>(pos) >= f_parts.size())
    {
        // we do not need the format because it's all going to be
        // zeroes and thus the loop below will take care of fixing
        // each part with the proper format
        //
        part zero;
        zero.set_separator(U'.');
        do
        {
            f_parts.push_back(zero);
        }
        while(static_cast<std::size_t>(pos) >= f_parts.size());
    }

    for(;;)
    {
        if(f_parts[pos].is_zero())
        {
            if(pos == 0)
            {
                f_last_error = "minimum limit reached; cannot decrement version any further.";
                return false;
            }
            f_parts[pos] = get_format_part(format, pos, f_parts[pos].is_integer());
            --pos;
        }
        else
        {
            f_parts[pos].previous();

            while(pos > 1
               && f_parts[pos].is_zero()
               && static_cast<std::size_t>(pos + 1) == f_parts.size())
            {
                erase(pos);
                --pos;
            }
            return true;
        }
    }
    snapdev::NOT_REACHED();
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
