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

// tested file
//
#include    "versiontheca/basic.h"


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



versiontheca::versiontheca::pointer_t create(char const * version, char const * verify = nullptr)
{
    versiontheca::basic::pointer_t t(std::make_shared<versiontheca::basic>());
    versiontheca::versiontheca::pointer_t v(std::make_shared<versiontheca::versiontheca>(t, version));
    CATCH_REQUIRE(v->get_version() == (verify == nullptr ? version : verify));
    return v;
}


void invalid_version(char const * version, char const * errmsg)
{
    versiontheca::basic::pointer_t t(std::make_shared<versiontheca::basic>());
    versiontheca::versiontheca::pointer_t v(std::make_shared<versiontheca::versiontheca>(t, version));
    CATCH_REQUIRE_FALSE(v->is_valid());
    CATCH_REQUIRE(v->get_last_error(false) == errmsg);
    CATCH_REQUIRE(v->get_last_error() == errmsg);
    CATCH_REQUIRE(v->get_last_error().empty());
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



}
// no name namespace


CATCH_TEST_CASE("basic_versions", "[valid]")
{
    CATCH_START_SECTION("basic_versions: verify test checker for version 1.0")
    {
        create("1.0");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("basic_versions: verify that canonicalization happens")
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
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("basic_versions: many valid versions")
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


CATCH_TEST_CASE("next_previous_basic_versions", "[valid][next][previous]")
{
    CATCH_START_SECTION("next_previous_basic_versions: next/previous at level 4, 3, 2, 1, 0")
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
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("compare_basic_versions", "[valid][compare]")
{
    CATCH_START_SECTION("compare_basic_versions: compare many versions")
    {
        versiontheca::versiontheca::pointer_t a(create("1.2"));
        versiontheca::versiontheca::pointer_t b(create("1.1"));
        versiontheca::versiontheca::pointer_t c(create("1.2.0.0", "1.2"));  // the zero are ignored by the compare

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


CATCH_TEST_CASE("invalid_basic_versions", "[invalid]")
{
    CATCH_START_SECTION("invalid_debian_versions: empty")
    {
        // empty
        //
        // note: the empty version is "invalid" as far as versions go,
        //       but it does not generetate an error message
        //
        versiontheca::basic::pointer_t t(std::make_shared<versiontheca::basic>());
        versiontheca::versiontheca v(t, "");
        CATCH_REQUIRE_FALSE(v.is_valid());
        CATCH_REQUIRE(v.get_last_error().empty());

        CATCH_REQUIRE(v.get_version().empty());
        CATCH_REQUIRE(v.get_last_error() == "no parts to output.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_basic_versions: no support for ':' or '-' or '#' or '$'...")
    {
        invalid_version("3A3:1.2.3-pre55", "basic versions only support integers separated by periods (.).");
        invalid_version("33:-55", "basic versions only support integers separated by periods (.).");
        invalid_version(":", "basic versions only support integers separated by periods (.).");
        invalid_version("a:", "basic versions only support integers separated by periods (.).");
        invalid_version("-10:", "basic versions only support integers separated by periods (.).");
        invalid_version("99999999999999999:", "integer too large for a valid version.");
        invalid_version("3:", "basic versions only support integers separated by periods (.).");
        invalid_version("-751", "basic versions only support integers separated by periods (.).");
        invalid_version("-", "basic versions only support integers separated by periods (.).");
        invalid_version("--", "basic versions only support integers separated by periods (.).");
        invalid_version("+-", "basic versions only support integers separated by periods (.).");
        invalid_version("#-", "basic versions only support integers separated by periods (.).");
        invalid_version("55:435123-", "basic versions only support integers separated by periods (.).");
        invalid_version("-a", "basic versions only support integers separated by periods (.).");
        invalid_version("-0", "basic versions only support integers separated by periods (.).");
        invalid_version("-+", "basic versions only support integers separated by periods (.).");
        invalid_version("-3$7", "basic versions only support integers separated by periods (.).");
        invalid_version("32:1.2.55-3:7", "basic versions only support integers separated by periods (.).");
        invalid_version("-3.7", "basic versions only support integers separated by periods (.).");
        invalid_version("3.7#", "basic versions only support integers separated by periods (.).");
        invalid_version("3$7", "basic versions only support integers separated by periods (.).");
        invalid_version("3;7", "basic versions only support integers separated by periods (.).");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_basic_versions: max + 1 fails")
    {
        versiontheca::versiontheca::pointer_t a(create("4294967295.4294967295.4294967295"));
        CATCH_REQUIRE(a->is_valid());
        CATCH_REQUIRE_FALSE(a->next(2));
        CATCH_REQUIRE_FALSE(a->is_valid());
        CATCH_REQUIRE(a->get_last_error() == "maximum limit reached; cannot increment version any further.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_basic_versions: min - 1 fails")
    {
        versiontheca::versiontheca::pointer_t a(create("0.0"));
        CATCH_REQUIRE(a->is_valid());
        CATCH_REQUIRE_FALSE(a->previous(2));
        CATCH_REQUIRE_FALSE(a->is_valid());
        CATCH_REQUIRE(a->get_last_error() == "minimum limit reached; cannot decrement version any further.");
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("bad_basic_calls", "[invalid]")
{
    CATCH_START_SECTION("bad_basic_calls: next without a version")
    {
        versiontheca::basic::pointer_t t(std::make_shared<versiontheca::basic>());
        versiontheca::versiontheca v(t);
        CATCH_REQUIRE(v.next(0));
        CATCH_REQUIRE(v.get_last_error() == "");
        CATCH_REQUIRE(v.get_version() == "1.0");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bad_basic_calls: previous without a version")
    {
        versiontheca::basic::pointer_t t(std::make_shared<versiontheca::basic>());
        versiontheca::versiontheca v(t);
        CATCH_REQUIRE_FALSE(v.previous(0));
        CATCH_REQUIRE(v.get_last_error() == "minimum limit reached; cannot decrement version any further.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bad_basic_calls: next out of bounds")
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

    CATCH_START_SECTION("bad_basic_calls: previous out of bounds")
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

    CATCH_START_SECTION("bad_basic_calls: compare against an empty (invalid) version")
    {
        versiontheca::versiontheca::pointer_t a(create("1.2"));
        versiontheca::basic::pointer_t t(std::make_shared<versiontheca::basic>());
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

    CATCH_START_SECTION("bad_basic_calls: compare using an empty (invalid) version")
    {
        versiontheca::basic::pointer_t t(std::make_shared<versiontheca::basic>());
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
}



// vim: ts=4 sw=4 et
