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
#include    "versiontheca/roman.h"


// self
//
#include    "catch_main.h"



// versiontheca
//
#include    "versiontheca/exception.h"
#include    "versiontheca/versiontheca.h"


// libutf8
//
#include    "libutf8/libutf8.h"


// C++
//
#include    <cstring>
#include    <iomanip>
#include    <stdexcept>




namespace
{



versiontheca::versiontheca::pointer_t create(char const * version, char const * verify = nullptr)
{
    versiontheca::roman::pointer_t t(std::make_shared<versiontheca::roman>());
    versiontheca::versiontheca::pointer_t v(std::make_shared<versiontheca::versiontheca>(t, version));
    if(verify == nullptr)
    {
        verify = version;
    }
    CATCH_REQUIRE(v->get_version() == verify);
    return v;
}


std::string value_to_roman(int v)
{
    std::string result;

    for(; v >= 1'000; v -= 1'000)
    {
        result += 'M';
    }

    if(v >= 900)
    {
        result += "CM";
        v -= 900;
    }
    else
    {
        if(v >= 500)
        {
            result += "D";
            v -= 500;
        }
        else if(v >= 400) // v E [400 .. 500)
        {
            result += "CD";
            v -= 400;
        }

        for(; v >= 100; v -= 100)
        {
            result += 'C';
        }
    }

    if(v >= 90)
    {
        result += "XC";
        v -= 90;
    }
    else
    {
        if(v >= 50)
        {
            result += 'L';
            v -= 50;
        }
        else if(v >= 40)
        {
            result += "XL";
            v -= 40;
        }

        for(; v >= 10; v -= 10)
        {
            result += 'X';
        }
    }

    if(v == 9)
    {
        result += "IX";
    }
    else
    {
        if(v >= 5)
        {
            result += 'V';
            v -= 5;
        }
        else if(v == 4)
        {
            result += "IV";
            v = 0;
        }

        for(; v > 0; --v)
        {
            result += 'I';
        }
    }

    return result;
}



}
// no name namespace


CATCH_TEST_CASE("roman_numerals", "[roman][valid]")
{
    CATCH_START_SECTION("roman_numerals: verify roman number conversions")
    {
        for(int i(1); i <= 3999; ++i)
        {
            std::string const roman_number(value_to_roman(i));
            CATCH_REQUIRE(versiontheca::from_roman_number(roman_number) == static_cast<versiontheca::part_integer_t>(i));
            CATCH_REQUIRE(versiontheca::to_roman_number(i) == roman_number);
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("roman_versions", "[roman][valid]")
{
    CATCH_START_SECTION("roman_versions: verify test checker for version 1.0 and i.0 and I.0")
    {
        create("1.0");
        create("i.0", "I.0");
        create("I.0");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("roman_versions: verify numbers from 1 to 3999")
    {
        for(int major_version(1); major_version <= 3999; ++major_version)
        {
            // use a random number for the minor
            //
            int const minor_version(rand() % 3999 + 1);
            std::string const major_number(value_to_roman(major_version));
            std::string const minor_number(value_to_roman(minor_version));
            std::string version(major_number + '.' + minor_number);
            versiontheca::versiontheca::pointer_t v(create(version.c_str()));
            CATCH_REQUIRE(v->get_major() == static_cast<versiontheca::part_integer_t>(major_version));
            CATCH_REQUIRE(v->get_minor() == static_cast<versiontheca::part_integer_t>(minor_version));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("roman_versions: funny roman numerals")
    {
        // the numerals on the left are _possible_ but should not be used
        //
        create("I.IL", "I.XLIX");
        create("I.IC", "I.XCIX");
        create("I.vc", "I.XCV");
        create("I.ID", "I.CDXCIX");
        create("i.vd", "I.CDXCV");
        create("I.IM", "I.CMXCIX");
        create("IIII.A", "IV.A");

        // the following lot is really just circumstancial
        //
        create("I.LC", "I.L");
        create("i.llci", "I.I");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("roman_versions: including other things")
    {
        versiontheca::versiontheca::pointer_t a(create("3.L.rc5", "3.L.rc5"));
        versiontheca::versiontheca::pointer_t b(create("3.XI.rc6", "3.XI.rc6"));

        CATCH_REQUIRE(*a > *b); // 50 > 11
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("roman_invalid", "[roman][invalid]")
{
    CATCH_START_SECTION("roman_invalid: verify test checker for version 1.0 and i.0 and I.0")
    {
        versiontheca::roman::pointer_t t(std::make_shared<versiontheca::roman>());
        versiontheca::versiontheca::pointer_t v(std::make_shared<versiontheca::versiontheca>(t, ""));
        CATCH_REQUIRE(v->get_last_error().empty());
        CATCH_REQUIRE(v->get_version().empty());
        CATCH_REQUIRE(v->get_last_error() == "no parts to output.");
        CATCH_REQUIRE(v->get_last_error().empty());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("roman_invalid: two periods one after the other is not valid")
    {
        versiontheca::roman::pointer_t t(std::make_shared<versiontheca::roman>());
        versiontheca::versiontheca::pointer_t v(std::make_shared<versiontheca::versiontheca>(t, ""));
        CATCH_REQUIRE_FALSE(v->set_version("1..2"));
        CATCH_REQUIRE(v->get_last_error() == "a version value cannot be an empty string.");
        CATCH_REQUIRE(v->get_last_error().empty());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("roman_invalid: invalid roman numbers")
    {
        CATCH_REQUIRE(versiontheca::to_roman_number(0).empty());
        for(int n(4000); n <= 5000; ++n)
        {
            CATCH_REQUIRE(versiontheca::to_roman_number(n).empty());
        }

        CATCH_REQUIRE(versiontheca::from_roman_number("") == 0);
    }
    CATCH_END_SECTION()
}

// vim: ts=4 sw=4 et
