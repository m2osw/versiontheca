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

#define protected public

// tested file
//
#include    "versiontheca/unicode.h"


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
    versiontheca::unicode::pointer_t t(std::make_shared<versiontheca::unicode>());
    versiontheca::versiontheca::pointer_t v(std::make_shared<versiontheca::versiontheca>(t, version));
    if(verify == nullptr)
    {
        verify = version;
    }
    CATCH_REQUIRE(v->get_version() == verify);
    return v;
}


versiontheca::versiontheca::pointer_t invalid_version(char const * version, char const * errmsg)
{
    versiontheca::unicode::pointer_t t(std::make_shared<versiontheca::unicode>());
    versiontheca::versiontheca::pointer_t v(std::make_shared<versiontheca::versiontheca>(t, version));
if(v->is_valid())
{
std::cerr << "--- testing invalid versions, but [" << version << "] is considered valid!\n";
}

    CATCH_REQUIRE_FALSE(v->is_valid());
    CATCH_REQUIRE(v->get_last_error(false) == errmsg);
    CATCH_REQUIRE(v->get_last_error() == errmsg);
    CATCH_REQUIRE(v->get_last_error().empty());

    return v;
}


std::string generate_number()
{
    versiontheca::part_integer_t value;
    SNAP_CATCH2_NAMESPACE::random(value);
    return std::to_string(value);
}


std::string generate_version(std::size_t max)
{
    std::string v;
    for(std::size_t i(0); i < max; ++i)
    {
        if(!v.empty())
        {
            v += '.';
        }
        v += generate_number();
    }
    return v;
}


// a version of wctombs which encodes anything including invalid unicode
// characters to make sure the library detects such as expected
//
std::string wctombs(char32_t wc)
{
    char mb[4];
    if(wc < 0x80)
    {
        /* this will also encode '\0'... */
        mb[0] = static_cast<char>(wc);
        return std::string(mb, 1);
    }
    if(wc < 0x800)
    {
        mb[0] = static_cast<char>((wc >> 6) | 0xC0);
        mb[1] = (wc & 0x3F) | 0x80;
        return std::string(mb, 2);
    }
    if(wc < 0x10000)
    {
        mb[0] = static_cast<char>((wc >> 12) | 0xE0);
        mb[1] = ((wc >> 6) & 0x3F) | 0x80;
        mb[2] = (wc & 0x3F) | 0x80;
        return std::string(mb, 3);
    }
    mb[0] = static_cast<char>((wc >> 18) | 0xF0);
    mb[1] = ((wc >> 12) & 0x3F) | 0x80;
    mb[2] = ((wc >> 6) & 0x3F) | 0x80;
    mb[3] = (wc & 0x3F) | 0x80;
    return std::string(mb, 4);
}

}
// no name namespace


CATCH_TEST_CASE("unicode_versions", "[valid]")
{
    CATCH_START_SECTION("unicode_versions: verify test checker for version 1.0")
    {
        create("1.0");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("unicode_versions: verify that canonicalization happens")
    {
        {
            versiontheca::versiontheca::pointer_t v(create("3", "3.0"));
            CATCH_REQUIRE(v->get_major() == 3);
            CATCH_REQUIRE(v->get_minor() == 0);
            CATCH_REQUIRE(v->get_patch() == 0);
            CATCH_REQUIRE(v->get_build() == 0);
        }
        {
            versiontheca::versiontheca::pointer_t v(create("1.0.0", "1.0"));
            CATCH_REQUIRE(v->get_major() == 1);
            CATCH_REQUIRE(v->get_minor() == 0);
            CATCH_REQUIRE(v->get_patch() == 0);
            CATCH_REQUIRE(v->get_build() == 0);
        }
        {
            versiontheca::versiontheca::pointer_t v(create("1.0.0.0", "1.0"));
            CATCH_REQUIRE(v->get_major() == 1);
            CATCH_REQUIRE(v->get_minor() == 0);
            CATCH_REQUIRE(v->get_patch() == 0);
            CATCH_REQUIRE(v->get_build() == 0);
        }
        {
            versiontheca::versiontheca::pointer_t v(create("A.A.A", "A.A"));
            CATCH_REQUIRE(v->get_major() == 0);
            CATCH_REQUIRE(v->get_minor() == 0);
            CATCH_REQUIRE(v->get_patch() == 0);
            CATCH_REQUIRE(v->get_build() == 0);
        }
        {
            versiontheca::versiontheca::pointer_t v(create("C.A.I"));
            CATCH_REQUIRE(v->get_major() == 0);
            CATCH_REQUIRE(v->get_minor() == 0);
            CATCH_REQUIRE(v->get_patch() == 0);
            CATCH_REQUIRE(v->get_build() == 0);
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("unicode_versions: many valid versions")
    {
        // many valid versions generated randomly to increase the likelyhood
        // of things I would otherwise not think of
        //
        for(int i(0); i < 10'000; ++i)
        {
            int const parts(i % 25 + 1);
            if(parts == 1)
            {
                std::string v(generate_version(parts));
                create(v.c_str(), (v + ".0").c_str());
            }
            else
            {
                std::string v(generate_version(parts));
                create(v.c_str());
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("next_previous_unicode_versions", "[valid][next][previous]")
{
    CATCH_START_SECTION("next_previous_unicode_versions: next/previous at level 4, 3, 2, 1, 0")
    {
        {
            versiontheca::versiontheca::pointer_t a(create("1.3.2"));
            CATCH_REQUIRE(a->next(4));
            CATCH_REQUIRE(a->get_version() == "1.3.2.0.1");
            CATCH_REQUIRE(a->previous(4));
            CATCH_REQUIRE(a->get_version() == "1.3.2"); // +1 -1, back to original
            CATCH_REQUIRE(a->previous(4));
            CATCH_REQUIRE(a->get_version() == "1.3.1.4294967295.4294967295");
            CATCH_REQUIRE(a->get_major() == 1);
            CATCH_REQUIRE(a->get_minor() == 3);
            CATCH_REQUIRE(a->get_patch() == 1);
            CATCH_REQUIRE(a->get_build() == 4294967295);
            CATCH_REQUIRE(a->next(4));
            CATCH_REQUIRE(a->get_version() == "1.3.2"); // +1 -1 -1 +1, back to original
        }

        {
            versiontheca::versiontheca::pointer_t a(create("1.3.2"));
            CATCH_REQUIRE(a->next(3));
            CATCH_REQUIRE(a->get_version() == "1.3.2.1");
            CATCH_REQUIRE(a->previous(3));
            CATCH_REQUIRE(a->get_version() == "1.3.2");
            CATCH_REQUIRE(a->previous(3));
            CATCH_REQUIRE(a->get_version() == "1.3.1.4294967295");
            CATCH_REQUIRE(a->next(3));
            CATCH_REQUIRE(a->get_version() == "1.3.2");
        }

        {
            versiontheca::versiontheca::pointer_t a(create("1.3.2"));
            CATCH_REQUIRE(a->next(2));
            CATCH_REQUIRE(a->get_version() == "1.3.3");
            CATCH_REQUIRE(a->previous(2));
            CATCH_REQUIRE(a->get_version() == "1.3.2");
            CATCH_REQUIRE(a->previous(2));
            CATCH_REQUIRE(a->get_version() == "1.3.1");
            CATCH_REQUIRE(a->next(2));
            CATCH_REQUIRE(a->get_version() == "1.3.2");
        }

        {
            versiontheca::versiontheca::pointer_t a(create("1.3.2"));
            CATCH_REQUIRE(a->next(1));
            CATCH_REQUIRE(a->get_version() == "1.4");
            CATCH_REQUIRE(a->previous(1));
            CATCH_REQUIRE(a->get_version() == "1.3");
            CATCH_REQUIRE(a->previous(1));
            CATCH_REQUIRE(a->get_version() == "1.2");
            CATCH_REQUIRE(a->next(1));
            CATCH_REQUIRE(a->get_version() == "1.3");
        }

        {
            versiontheca::versiontheca::pointer_t a(create("1.3.2"));
            CATCH_REQUIRE(a->next(0));
            CATCH_REQUIRE(a->get_version() == "2.0");
            CATCH_REQUIRE(a->previous(0));
            CATCH_REQUIRE(a->get_version() == "1.0");
            CATCH_REQUIRE(a->previous(0));
            CATCH_REQUIRE(a->get_version() == "0.0");
            CATCH_REQUIRE(a->next(0));
            CATCH_REQUIRE(a->get_version() == "1.0");
        }

        {
            versiontheca::versiontheca::pointer_t a(create("1.3"));
            versiontheca::versiontheca::pointer_t f(create("9.9z"));
            a->set_format(*f);
            CATCH_REQUIRE(a->next(2));
            CATCH_REQUIRE(a->get_version() == "1.3B");
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("compare_unicode_versions", "[valid][compare]")
{
    CATCH_START_SECTION("compare_unicode_versions: compare many versions")
    {
        versiontheca::versiontheca::pointer_t a(create("1.2"));
        versiontheca::versiontheca::pointer_t b(create("1.1"));
        versiontheca::versiontheca::pointer_t c(create("1.2.0.0", "1.2"));  // the zero are ignored by the compare
        versiontheca::versiontheca::pointer_t d(create("1.2.5", "1.2.5"));

        CATCH_REQUIRE(a->is_valid());
        CATCH_REQUIRE(b->is_valid());
        CATCH_REQUIRE(c->is_valid());

        CATCH_REQUIRE(*a == *a);
        CATCH_REQUIRE_FALSE(*a != *a);
        CATCH_REQUIRE_FALSE(*a > *a);
        CATCH_REQUIRE(*a >= *a);
        CATCH_REQUIRE_FALSE(*a < *a);
        CATCH_REQUIRE(*a <= *a);

        CATCH_REQUIRE_FALSE(*a == *b);
        CATCH_REQUIRE(*a != *b);
        CATCH_REQUIRE(*a > *b);
        CATCH_REQUIRE(*a >= *b);
        CATCH_REQUIRE_FALSE(*a < *b);
        CATCH_REQUIRE_FALSE(*a <= *b);

        CATCH_REQUIRE_FALSE(*b == *a);
        CATCH_REQUIRE(*b != *a);
        CATCH_REQUIRE_FALSE(*b > *a);
        CATCH_REQUIRE_FALSE(*b >= *a);
        CATCH_REQUIRE(*b < *a);
        CATCH_REQUIRE(*b <= *a);

        CATCH_REQUIRE(*a == *c);
        CATCH_REQUIRE_FALSE(*a != *c);
        CATCH_REQUIRE_FALSE(*a > *c);
        CATCH_REQUIRE(*a >= *c);
        CATCH_REQUIRE_FALSE(*a < *c);
        CATCH_REQUIRE(*a <= *c);

        CATCH_REQUIRE(*c == *a);
        CATCH_REQUIRE_FALSE(*c != *a);
        CATCH_REQUIRE_FALSE(*c > *a);
        CATCH_REQUIRE(*c >= *a);
        CATCH_REQUIRE_FALSE(*c < *a);
        CATCH_REQUIRE(*c <= *a);

        CATCH_REQUIRE_FALSE(*a == *d);
        CATCH_REQUIRE(*a != *d);
        CATCH_REQUIRE_FALSE(*a > *d);
        CATCH_REQUIRE_FALSE(*a >= *d);
        CATCH_REQUIRE(*a < *d);
        CATCH_REQUIRE(*a <= *d);

        CATCH_REQUIRE_FALSE(*d == *a);
        CATCH_REQUIRE(*d != *a);
        CATCH_REQUIRE(*d > *a);
        CATCH_REQUIRE(*d >= *a);
        CATCH_REQUIRE_FALSE(*d < *a);
        CATCH_REQUIRE_FALSE(*d <= *a);

        {
            std::stringstream ss;
            ss << *a;
            CATCH_REQUIRE(ss.str() == "1.2");
        }
        {
            std::stringstream ss;
            ss << *b;
            CATCH_REQUIRE(ss.str() == "1.1");
        }
        {
            std::stringstream ss;
            ss << *c;
            CATCH_REQUIRE(ss.str() == "1.2");
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_unicode_versions", "[invalid]")
{
    CATCH_START_SECTION("invalid_unicode_versions: empty")
    {
        // empty
        //
        // note: the empty version is "invalid" as far as versions go,
        //       but it does not generetate an error message
        //
        versiontheca::unicode::pointer_t t(std::make_shared<versiontheca::unicode>());
        versiontheca::versiontheca v(t, "");
        CATCH_REQUIRE_FALSE(v.is_valid());
        CATCH_REQUIRE(v.get_last_error().empty());

        CATCH_REQUIRE(v.get_version().empty());
        CATCH_REQUIRE(v.get_last_error() == "no parts to output.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_unicode_versions: two periods, period at the start or end")
    {
        // most characters are valid in Unicode versions
        // we still have a few cases of invalid entry
        //
        invalid_version("3A3:1.2..3-pre55", "a version value cannot be an empty string.");
        invalid_version(".33:-55", "a version value cannot be an empty string.");
        invalid_version(":.", "a version value cannot be an empty string.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_unicode_versions: randomized")
    {
        // check all invalid unicode characters
        //
        for(char32_t c(1); c < 0x110000; ++c)
        {
            if(libutf8::is_valid_unicode(c, false))
            {
                // skip valid characters
                //
                continue;
            }
            std::size_t const max_parts(rand() % (versiontheca::MAX_PARTS - 1) + 1);
            std::size_t bad_pos(rand() % (max_parts * 5));
            std::string v;
            std::size_t count(0);
            if(count == bad_pos)
            {
                v += wctombs(c);
            }
            for(std::size_t part_no(0); part_no < max_parts; ++part_no)
            {
                if(part_no != 0)
                {
                    v += '.';
                }
                std::size_t const length(rand() % 10 + 1);
                for(std::size_t l(0); l < length; ++l)
                {
                    char32_t wc(U'\0');
                    do
                    {
                        wc = SNAP_CATCH2_NAMESPACE::random_char(SNAP_CATCH2_NAMESPACE::character_t::CHARACTER_UNICODE);
                    }
                    while(wc == '.' || wc < 0x20 || (wc >= 0x7F && wc <= 0x9F));
                    v += libutf8::to_u8string(wc);
                    ++count;
                    if(count == bad_pos)
                    {
                        v += wctombs(c);
                    }
                }
            }
            if(bad_pos >= count)
            {
                v += wctombs(c);
            }
            std::stringstream last_error;
            if(c >= 0xD800 && c <= 0xDFFF)
            {
                last_error << "input string includes an invalid code not representing a valid UTF-8 character.";
            }
            else
            {
                last_error << "found unexpected character: \\U"
                           << std::hex << std::uppercase << std::setfill('0')
                                       << std::setw(6) << static_cast<int>(c)
                           << " in input.";
            }
            versiontheca::versiontheca::pointer_t a(invalid_version(v.c_str(), last_error.str().c_str()));
            if(max_parts < versiontheca::MAX_PARTS - 1)
            {
                // make sure the parse_value() function catches invalid Unicode
                //
                CATCH_REQUIRE_FALSE(a->get_trait()->parse_value(v, U'.'));
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_unicode_versions: max + 1 fails")
    {
        versiontheca::versiontheca::pointer_t a(create("4294967295.4294967295.4294967295"));
        CATCH_REQUIRE(a->is_valid());
        CATCH_REQUIRE_FALSE(a->next(2));
        CATCH_REQUIRE_FALSE(a->is_valid());
        CATCH_REQUIRE(a->get_last_error() == "maximum limit reached; cannot increment version any further.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_unicode_versions: min - 1 fails")
    {
        versiontheca::versiontheca::pointer_t a(create("0.0"));
        CATCH_REQUIRE(a->is_valid());
        CATCH_REQUIRE_FALSE(a->previous(2));
        CATCH_REQUIRE_FALSE(a->is_valid());
        CATCH_REQUIRE(a->get_last_error() == "minimum limit reached; cannot decrement version any further.");
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("bad_unicode_calls", "[invalid]")
{
    CATCH_START_SECTION("bad_unicode_calls: next without a version")
    {
        versiontheca::unicode::pointer_t t(std::make_shared<versiontheca::unicode>());
        versiontheca::versiontheca v(t);
        CATCH_REQUIRE(v.next(0));
        CATCH_REQUIRE(v.get_last_error() == "");
        CATCH_REQUIRE(v.get_version() == "1.0");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bad_unicode_calls: previous without a version")
    {
        versiontheca::unicode::pointer_t t(std::make_shared<versiontheca::unicode>());
        versiontheca::versiontheca v(t);
        CATCH_REQUIRE_FALSE(v.previous(0));
        CATCH_REQUIRE(v.get_last_error() == "minimum limit reached; cannot decrement version any further.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bad_unicode_calls: next out of bounds")
    {
        versiontheca::versiontheca::pointer_t a(create("1.5.3"));
        for(int p(-100); p < 0; ++p)
        {
            CATCH_REQUIRE_THROWS_MATCHES(
                  a->next(p)
                , versiontheca::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "versiontheca_exception: position calling next() cannot be a negative number."));
        }
        for(int p(versiontheca::MAX_PARTS); p < static_cast<int>(versiontheca::MAX_PARTS + 100); ++p)
        {
            CATCH_REQUIRE_THROWS_MATCHES(
                  a->next(p)
                , versiontheca::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "versiontheca_exception: position calling next() cannot be more than "
                        + std::to_string(versiontheca::MAX_PARTS)
                        + "."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bad_unicode_calls: previous out of bounds")
    {
        versiontheca::versiontheca::pointer_t a(create("1.5.3"));
        for(int p(-100); p < 0; ++p)
        {
            CATCH_REQUIRE_THROWS_MATCHES(
                  a->previous(p)
                , versiontheca::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "versiontheca_exception: position calling previous() cannot be a negative number."));
        }
        for(int p(versiontheca::MAX_PARTS); p < static_cast<int>(versiontheca::MAX_PARTS + 100); ++p)
        {
            CATCH_REQUIRE_THROWS_MATCHES(
                  a->previous(p)
                , versiontheca::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "versiontheca_exception: position calling previous() cannot be more than "
                        + std::to_string(versiontheca::MAX_PARTS)
                        + "."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bad_unicode_calls: compare against an empty (invalid) version")
    {
        versiontheca::versiontheca::pointer_t a(create("1.2"));
        versiontheca::unicode::pointer_t t(std::make_shared<versiontheca::unicode>());
        versiontheca::versiontheca empty(t, "");

        CATCH_REQUIRE(a->is_valid());
        CATCH_REQUIRE_FALSE(empty.is_valid());

        CATCH_REQUIRE_THROWS_MATCHES(
              a->compare(empty)
            , versiontheca::invalid_version
            , Catch::Matchers::ExceptionMessage(
                      "versiontheca_exception: one or both of the input versions are not valid."));

        CATCH_REQUIRE_THROWS_MATCHES(
              a->get_trait()->compare(t)
            , versiontheca::empty_version
            , Catch::Matchers::ExceptionMessage(
                      "versiontheca_exception: one or both of the input versions are empty."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bad_unicode_calls: compare using an empty (invalid) version")
    {
        versiontheca::unicode::pointer_t t(std::make_shared<versiontheca::unicode>());
        versiontheca::versiontheca empty(t, "");
        versiontheca::versiontheca::pointer_t b(create("5.3"));

        CATCH_REQUIRE_FALSE(empty.is_valid());
        CATCH_REQUIRE(b->is_valid());

        CATCH_REQUIRE(empty.get_major() == 0);
        CATCH_REQUIRE(empty.get_minor() == 0);
        CATCH_REQUIRE(empty.get_patch() == 0);
        CATCH_REQUIRE(empty.get_build() == 0);

        CATCH_REQUIRE_THROWS_MATCHES(
              empty.compare(*b)
            , versiontheca::invalid_version
            , Catch::Matchers::ExceptionMessage(
                      "versiontheca_exception: one or both of the input versions are not valid."));

        CATCH_REQUIRE_THROWS_MATCHES(
              t->compare(b->get_trait())
            , versiontheca::empty_version
            , Catch::Matchers::ExceptionMessage(
                      "versiontheca_exception: one or both of the input versions are empty."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bad_unicode_calls: too many parts")
    {
        // up to MAX_PARTS, it works
        //
        std::string version("1");  // version "1" is a special case
        create(version.c_str(), "1.0");
        std::size_t idx(2);
        for(; idx <= versiontheca::MAX_PARTS; ++idx)
        {
            version += '.';
            version += std::to_string(idx);
            create(version.c_str());
        }

        // when more than MAX_PARTS, it throws
        //
        for(; idx < versiontheca::MAX_PARTS + 100; ++idx)
        {
            version += '.';
            version += std::to_string(idx);
            CATCH_REQUIRE_THROWS_MATCHES(
                  create(version.c_str())
                , versiontheca::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "versiontheca_exception: trying to append more parts when maximum was already reached."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bad_unicode_calls: next/erase out of bounds")
    {
        versiontheca::unicode::pointer_t t(std::make_shared<versiontheca::unicode>());
        CATCH_REQUIRE_FALSE(t->parse(std::string()));
        CATCH_REQUIRE(t->get_last_error() == "an empty input string cannot represent a valid version.");
        CATCH_REQUIRE(t->get_last_error().empty());
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
