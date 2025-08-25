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

// tested file
//
#include    "versiontheca/part.h"


// self
//
#include    "catch_main.h"



// versiontheca
//
#include    "versiontheca/exception.h"
#include    "versiontheca/versiontheca.h"


// C++
//
#include    <cstring>
#include    <iomanip>
#include    <stdexcept>




namespace
{


bool is_number(std::string value)
{
    for(auto const c : value)
    {
        if(c < '0' || c > '9')
        {
            return false;
        }
    }

    return !value.empty();
}



}
// no name namespace


CATCH_TEST_CASE("part_integer", "[part][integer][valid]")
{
    CATCH_START_SECTION("part_integer: verify default part")
    {
        versiontheca::part p;

        CATCH_REQUIRE(p.get_separator() == U'\0');
        CATCH_REQUIRE(p.get_width() == 0);
        CATCH_REQUIRE(p.get_type() == '\0');
        CATCH_REQUIRE(p.is_integer());
        //CATCH_REQUIRE(p.get_string()); -- would throw
        CATCH_REQUIRE(p.get_integer() == 0);
        CATCH_REQUIRE(p.to_string() == "0");
        CATCH_REQUIRE(p.get_last_error().empty());
        CATCH_REQUIRE(p.is_zero());

        CATCH_REQUIRE(p.next());
        CATCH_REQUIRE(p.get_integer() == 1);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_integer: set_value(\"\") sets the value to integer 0")
    {
        versiontheca::part p;

        p.set_value(std::string());

        CATCH_REQUIRE(p.get_separator() == U'\0');
        CATCH_REQUIRE(p.get_width() == 0);
        CATCH_REQUIRE(p.get_type() == '\0');
        CATCH_REQUIRE(p.is_integer());
        //CATCH_REQUIRE(p.get_string()); -- would throw
        CATCH_REQUIRE(p.get_integer() == 0);
        CATCH_REQUIRE(p.to_string() == "0");
        CATCH_REQUIRE(p.get_last_error().empty());
        CATCH_REQUIRE(p.is_zero());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_integer: random integers, using set_integer()")
    {
        for(int i(0); i < 100; ++i)
        {
            versiontheca::part p;

            versiontheca::part_integer_t value;
            SNAP_CATCH2_NAMESPACE::random(value);
            p.set_integer(value);

            CATCH_REQUIRE(p.get_separator() == U'\0');
            CATCH_REQUIRE(p.get_width() == 0);
            CATCH_REQUIRE(p.get_type() == '\0');
            CATCH_REQUIRE(p.is_integer());
            //CATCH_REQUIRE(p.get_string() == value); -- would throw
            CATCH_REQUIRE(p.get_integer() == value);
            std::stringstream ss;
            ss << value;
            CATCH_REQUIRE(p.to_string() == ss.str());
            CATCH_REQUIRE(p.get_last_error().empty());
            if(value == 0)
            {
                CATCH_REQUIRE(p.is_zero());
            }
            else
            {
                CATCH_REQUIRE_FALSE(p.is_zero());
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_integer: random integers from string, using set_value()")
    {
        for(int i(0); i < 100; ++i)
        {
            versiontheca::part p;

            versiontheca::part_integer_t value;
            SNAP_CATCH2_NAMESPACE::random(value);
            std::stringstream ss;
            ss << value;
            p.set_value(ss.str());

            CATCH_REQUIRE(p.get_separator() == U'\0');
            CATCH_REQUIRE(p.get_width() == 0);
            CATCH_REQUIRE(p.get_type() == '\0');
            CATCH_REQUIRE(p.is_integer());
            //CATCH_REQUIRE(p.get_string() == value); -- would throw
            CATCH_REQUIRE(p.get_integer() == value);
            CATCH_REQUIRE(p.to_string() == ss.str());
            CATCH_REQUIRE(p.get_last_error().empty());
            if(value == 0)
            {
                CATCH_REQUIRE(p.is_zero());
            }
            else
            {
                CATCH_REQUIRE_FALSE(p.is_zero());
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_integer: check minimum integer")
    {
        versiontheca::part p;
        p.set_integer(0);
        CATCH_REQUIRE(p.is_integer());
        CATCH_REQUIRE(p.get_integer() == 0U);

        CATCH_REQUIRE_FALSE(p.previous());
        CATCH_REQUIRE(p.get_integer() == 0U);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_integer: check maximum integer")
    {
        versiontheca::part p;
        p.set_to_max_integer();
        CATCH_REQUIRE(p.is_integer());
        CATCH_REQUIRE(p.get_integer() == std::numeric_limits<versiontheca::part_integer_t>::max());

        CATCH_REQUIRE_FALSE(p.next());
        CATCH_REQUIRE(p.get_integer() == std::numeric_limits<versiontheca::part_integer_t>::max());
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("part_string", "[part][integer][valid]")
{
    CATCH_START_SECTION("part_string: strings of digits, using set_string()")
    {
        for(int i(0); i < 100; ++i)
        {
            versiontheca::part p;

            versiontheca::part_integer_t number;
            SNAP_CATCH2_NAMESPACE::random(number);
            std::stringstream ss;
            ss << number;
            p.set_string(ss.str());

            CATCH_REQUIRE(p.get_separator() == U'\0');
            CATCH_REQUIRE(p.get_width() == 0);
            CATCH_REQUIRE(p.get_type() == '\0');
            CATCH_REQUIRE_FALSE(p.is_integer());
            CATCH_REQUIRE(p.get_string() == ss.str());
            //CATCH_REQUIRE(p.get_integer() == value); -- would throw
            CATCH_REQUIRE(p.to_string() == ss.str());
            CATCH_REQUIRE(p.get_last_error().empty());
            CATCH_REQUIRE_FALSE(p.is_zero());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_string: random strings, using set_string()")
    {
        for(int i(0); i < 100; ++i)
        {
            versiontheca::part p;

            std::string value;
            value = SNAP_CATCH2_NAMESPACE::random_string(1, 25, SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_UNICODE);
            p.set_string(value);

            CATCH_REQUIRE(p.get_separator() == U'\0');
            CATCH_REQUIRE(p.get_width() == 0);
            CATCH_REQUIRE(p.get_type() == '\0');
            CATCH_REQUIRE_FALSE(p.is_integer());
            CATCH_REQUIRE(p.get_string() == value);
            //CATCH_REQUIRE(p.get_integer() == value); -- would throw
            CATCH_REQUIRE(p.to_string() == value);
            CATCH_REQUIRE(p.get_last_error().empty());
            CATCH_REQUIRE_FALSE(p.is_zero());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_string: random strings, using set_value()")
    {
        for(int i(0); i < 100; ++i)
        {
            versiontheca::part p;

            std::string value;
            do
            {
                value = SNAP_CATCH2_NAMESPACE::random_string(1, 25, SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_UNICODE);
            }
            while(is_number(value));
            p.set_value(value);

            CATCH_REQUIRE(p.get_separator() == U'\0');
            CATCH_REQUIRE(p.get_width() == 0);
            CATCH_REQUIRE(p.get_type() == '\0');
            CATCH_REQUIRE_FALSE(p.is_integer());
            CATCH_REQUIRE(p.get_string() == value);
            //CATCH_REQUIRE(p.get_integer() == value); -- would throw
            CATCH_REQUIRE(p.to_string() == value);
            CATCH_REQUIRE(p.get_last_error().empty());
            CATCH_REQUIRE_FALSE(p.is_zero());
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_string: check maximum string")
    {
        for(int sz(1); sz <= 10; ++sz)
        {
            versiontheca::part p;
            p.set_to_max_string(sz);
            CATCH_REQUIRE_FALSE(p.is_integer());
            std::string expected;
            expected.reserve(sz);
            for(int i(0); i < sz; ++i)
            {
                expected += 'z';
            }
            CATCH_REQUIRE(p.get_string() == expected);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_string: next on two letters up to max.")
    {
        char buf[3] = { 'A', 'A', '\0' };
        versiontheca::part p;
        p.set_value(buf);
        CATCH_REQUIRE_FALSE(p.is_integer());
        CATCH_REQUIRE(p.get_string() == buf);
        for(int j(0); j < 51; ++j)
        {
            for(int i(0); i < 25; ++i) // AA to AZ
            {
                CATCH_REQUIRE(p.next());
                ++buf[1];
                CATCH_REQUIRE(p.get_string() == buf);
            }
            CATCH_REQUIRE(p.next()); // Z + 1 = 'a'
            buf[1] = 'a';
            CATCH_REQUIRE(p.get_string() == buf);
            for(int i(0); i < 25; ++i)
            {
                CATCH_REQUIRE(p.next());
                ++buf[1];
                CATCH_REQUIRE(p.get_string() == buf);
            }
            CATCH_REQUIRE(p.next()); // z + 1 = 'A' + carry
            if(buf[0] == 'Z')
            {
                buf[0] = 'a';
            }
            else
            {
                ++buf[0];
            }
            buf[1] = 'A';
            CATCH_REQUIRE(p.get_string() == buf);
        }

        // one more time on the lower character
        //
        for(int i(0); i < 25; ++i) // AA to AZ
        {
            CATCH_REQUIRE(p.next());
            ++buf[1];
            CATCH_REQUIRE(p.get_string() == buf);
        }
        CATCH_REQUIRE(p.next()); // Z + 1 = 'a'
        buf[1] = 'a';
        CATCH_REQUIRE(p.get_string() == buf);
        for(int i(0); i < 25; ++i)
        {
            CATCH_REQUIRE(p.next());
            ++buf[1];
            CATCH_REQUIRE(p.get_string() == buf);
        }
        CATCH_REQUIRE_FALSE(p.next());
        CATCH_REQUIRE(p.get_string() == buf);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_string: next on two letters separated by + up to max.")
    {
        char buf[4] = { 'A', '+', 'A', '\0' };
        versiontheca::part p;
        p.set_value(buf);
        CATCH_REQUIRE_FALSE(p.is_integer());
        CATCH_REQUIRE(p.get_string() == buf);
        for(int j(0); j < 51; ++j)
        {
            for(int i(0); i < 25; ++i)
            {
                CATCH_REQUIRE(p.next());
                ++buf[2];
                CATCH_REQUIRE(p.get_string() == buf);
            }
            CATCH_REQUIRE(p.next()); // Z + 1 = 'a'
            buf[2] = 'a';
            CATCH_REQUIRE(p.get_string() == buf);
            for(int i(0); i < 25; ++i)
            {
                CATCH_REQUIRE(p.next());
                ++buf[2];
                CATCH_REQUIRE(p.get_string() == buf);
            }
            CATCH_REQUIRE(p.next()); // z + 1 = 'A' + carry
            if(buf[0] == 'Z')
            {
                buf[0] = 'a';
            }
            else
            {
                ++buf[0];
            }
            buf[2] = 'A';
            CATCH_REQUIRE(p.get_string() == buf);
        }

        // one more time on the lower character
        //
        for(int i(0); i < 25; ++i) // AA to AZ
        {
            CATCH_REQUIRE(p.next());
            ++buf[2];
            CATCH_REQUIRE(p.get_string() == buf);
        }
        CATCH_REQUIRE(p.next()); // Z + 1 = 'a'
        buf[2] = 'a';
        CATCH_REQUIRE(p.get_string() == buf);
        for(int i(0); i < 25; ++i)
        {
            CATCH_REQUIRE(p.next());
            ++buf[2];
            CATCH_REQUIRE(p.get_string() == buf);
        }
        CATCH_REQUIRE_FALSE(p.next());
        CATCH_REQUIRE(p.get_string() == buf);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_string: previous on two letters up to min.")
    {
        char buf[3] = { 'z', 'z', '\0' };
        versiontheca::part p;
        p.set_value(buf);
        CATCH_REQUIRE_FALSE(p.is_integer());
        CATCH_REQUIRE(p.get_string() == buf);
        for(int j(0); j < 51; ++j)
        {
            for(int i(0); i < 25; ++i)
            {
                CATCH_REQUIRE(p.previous());
                --buf[1];
                CATCH_REQUIRE(p.get_string() == buf);
            }
            CATCH_REQUIRE(p.previous()); // a - 1 = 'Z'
            buf[1] = 'Z';
            CATCH_REQUIRE(p.get_string() == buf);
            for(int i(0); i < 25; ++i)
            {
                CATCH_REQUIRE(p.previous());
                --buf[1];
                CATCH_REQUIRE(p.get_string() == buf);
            }
            CATCH_REQUIRE(p.previous()); // a - 1 = 'Z' + carry
            if(buf[0] == 'a')
            {
                buf[0] = 'Z';
            }
            else
            {
                --buf[0];
            }
            buf[1] = 'z';
            CATCH_REQUIRE(p.get_string() == buf);
        }

        // one more time on the lower character
        //
        for(int i(0); i < 25; ++i) // AA to AZ
        {
            CATCH_REQUIRE(p.previous());
            --buf[1];
            CATCH_REQUIRE(p.get_string() == buf);
        }
        CATCH_REQUIRE(p.previous()); // a + 1 = 'Z'
        buf[1] = 'Z';
        CATCH_REQUIRE(p.get_string() == buf);
        for(int i(0); i < 25; ++i)
        {
            CATCH_REQUIRE(p.previous());
            --buf[1];
            CATCH_REQUIRE(p.get_string() == buf);
        }
        CATCH_REQUIRE_FALSE(p.previous());
        CATCH_REQUIRE(p.get_string() == buf);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_string: previous on two letters separated by + up to min.")
    {
        char buf[4] = { 'z', '+', 'z', '\0' };
        versiontheca::part p;
        p.set_value(buf);
        CATCH_REQUIRE_FALSE(p.is_integer());
        CATCH_REQUIRE(p.get_string() == buf);
        for(int j(0); j < 51; ++j)
        {
            for(int i(0); i < 25; ++i)
            {
                CATCH_REQUIRE(p.previous());
                --buf[2];
                CATCH_REQUIRE(p.get_string() == buf);
            }
            CATCH_REQUIRE(p.previous()); // a - 1 = 'Z'
            buf[2] = 'Z';
            CATCH_REQUIRE(p.get_string() == buf);
            for(int i(0); i < 25; ++i)
            {
                CATCH_REQUIRE(p.previous());
                --buf[2];
                CATCH_REQUIRE(p.get_string() == buf);
            }
            CATCH_REQUIRE(p.previous()); // a - 1 = 'Z' + carry
            if(buf[0] == 'a')
            {
                buf[0] = 'Z';
            }
            else
            {
                --buf[0];
            }
            buf[2] = 'z';
            CATCH_REQUIRE(p.get_string() == buf);
        }

        // one more time on the lower character
        //
        for(int i(0); i < 25; ++i) // AA to AZ
        {
            CATCH_REQUIRE(p.previous());
            --buf[2];
            CATCH_REQUIRE(p.get_string() == buf);
        }
        CATCH_REQUIRE(p.previous()); // a - 1 = 'Z'
        buf[2] = 'Z';
        CATCH_REQUIRE(p.get_string() == buf);
        for(int i(0); i < 25; ++i)
        {
            CATCH_REQUIRE(p.previous());
            --buf[2];
            CATCH_REQUIRE(p.get_string() == buf);
        }
        CATCH_REQUIRE_FALSE(p.previous());
        CATCH_REQUIRE(p.get_string() == buf);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("part_compare", "[part][compare][valid]")
{
    CATCH_START_SECTION("part_compare: integers")
    {
        versiontheca::part a;
        versiontheca::part b;

        CATCH_REQUIRE(a.compare(b) == 0);
        a.set_integer(10);
        CATCH_REQUIRE(a.compare(b) == 1);
        b.set_integer(253);
        CATCH_REQUIRE(a.compare(b) == -1);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_compare: strings")
    {
        versiontheca::part a;
        versiontheca::part b;

        a.set_string("abc");
        b.set_string("xyz");

        CATCH_REQUIRE(a.compare(b) == -1);
        CATCH_REQUIRE(a.compare(a) == 0);
        CATCH_REQUIRE(b.compare(b) == 0);
        a.set_string("xyz~more");
        CATCH_REQUIRE(a.compare(b) == 1);
        b.set_string("xyz~more");
        CATCH_REQUIRE(a.compare(b) == 0);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_compare: mix")
    {
        versiontheca::part a;
        versiontheca::part b;

        a.set_string("10");
        b.set_integer(2);

        CATCH_REQUIRE(a.compare(b) == -1); // 10 > 2 but as string "10" < "2"
        CATCH_REQUIRE(b.compare(a) == 1);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("part_meta", "[part][valid]")
{
    CATCH_START_SECTION("part_meta: separators")
    {
        for(char32_t sep(U' '); sep < 0x110000; ++sep)
        {
            if((sep >= 0x7F && sep <= 0x9F)
            || (sep >= 0xD800 && sep <= 0xDFFF))
            {
                continue;
            }
            versiontheca::part p;
            p.set_separator(sep);
            CATCH_REQUIRE(p.get_separator() == sep);

            // we can always reset back to U'\0'
            //
            p.set_separator(U'\0');
            CATCH_REQUIRE(p.get_separator() == U'\0');
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_meta: width")
    {
        for(int width(0); width < 256; ++width)
        {
            versiontheca::part p;
            CATCH_REQUIRE(p.get_width() == 0);
            p.set_width(width);
            CATCH_REQUIRE(p.get_width() == width);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_meta: type")
    {
        for(int type(0); type < 256; ++type)
        {
            versiontheca::part p;
            CATCH_REQUIRE(p.get_type() == '\0');
            p.set_type(type);
            CATCH_REQUIRE(p.get_type() == static_cast<char>(type));
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("part_invalid", "[part][invalid]")
{
    CATCH_START_SECTION("part_invalid: bad separators (controls)")
    {
        for(char32_t sep(U'\x01'); sep < U' '; ++sep)
        {
            versiontheca::part p;
            CATCH_REQUIRE_THROWS_MATCHES(
                  p.set_separator(sep)
                , versiontheca::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "versiontheca_exception: separator cannot be a control other than U'\\0' or a surrogate."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_invalid: bad separators (graphical controls)")
    {
        for(char32_t sep(U'\x7F'); sep < U'\xA0'; ++sep)
        {
            versiontheca::part p;
            CATCH_REQUIRE_THROWS_MATCHES(
                  p.set_separator(sep)
                , versiontheca::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "versiontheca_exception: separator cannot be a control other than U'\\0' or a surrogate."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_invalid: bad separators (surrogates)")
    {
        for(char32_t sep(0XD800); sep < 0xDFFF; ++sep)
        {
            versiontheca::part p;
            CATCH_REQUIRE_THROWS_MATCHES(
                  p.set_separator(sep)
                , versiontheca::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "versiontheca_exception: separator cannot be a control other than U'\\0' or a surrogate."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_invalid: get_string() when integer")
    {
        versiontheca::part p;
        CATCH_REQUIRE_THROWS_MATCHES(
              p.get_string()
            , versiontheca::wrong_type
            , Catch::Matchers::ExceptionMessage(
                      "versiontheca_exception: this part is not a string."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("part_invalid: get_integer() when string")
    {
        versiontheca::part p;
        p.set_string("now this is a string");
        CATCH_REQUIRE_THROWS_MATCHES(
              p.get_integer()
            , versiontheca::wrong_type
            , Catch::Matchers::ExceptionMessage(
                      "versiontheca_exception: this part is not an integer."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
