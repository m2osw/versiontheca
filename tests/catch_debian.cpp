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
#include    "catch_main.h"



// versiontheca
//
#include    "versiontheca/debian.h"
#include    "versiontheca/exception.h"
#include    "versiontheca/versiontheca.h"


// C++
//
#include    <cstring>
#include    <iomanip>
#include    <stdexcept>




namespace
{



versiontheca::versiontheca::pointer_t create(char const * version)
{
    versiontheca::debian::pointer_t t(std::make_shared<versiontheca::debian>());
    versiontheca::versiontheca::pointer_t v(std::make_shared<versiontheca::versiontheca>(t, version));
    CATCH_REQUIRE(v->get_version() == version);
    return v;
}



std::string print_version(std::string const & version)
{
    std::stringstream result;
    for(char const * s(version.c_str()); *s != '\0'; ++s)
    {
        if(static_cast<unsigned char>(*s) < ' ')
        {
            result << "^" << static_cast<char>(*s + '@');
        }
        else if(static_cast<unsigned char>(*s) >= 0x80)
        {
            result << "\\x" << std::hex << static_cast<int>(static_cast<unsigned char>(*s));
        }
        else if(*s == 0x7F)
        {
            result << "<DEL>";
        }
        else if(*s == '^')
        {
            result << "^^";
        }
        else if(*s == '@')
        {
            result << "@@";
        }
        else
        {
            result << *s;
        }
    }

    return result.str();
}


void check_version(std::string const & version, std::string const & error_msg)
{
    // validate_debian_version()
    {
        //char error_string[256];
        //strcpy(error_string, "no errors");
        versiontheca::debian::pointer_t t(std::make_shared<versiontheca::debian>());
        versiontheca::versiontheca v(t, version);
        //int valid(validate_debian_version(version, error_string, sizeof(error_string) / sizeof(error_string[0])));
//printf("from {%s} result = %d [%s] [%s]\n", print_version(version).c_str(), valid, error_string, error_msg);
        if(error_msg.empty())
        {
if(!v.is_valid())
std::cerr << "--- BAD: checked version [" << version << "], expected to be valid; err = [" << v.get_last_error(false) << "]\n";
            // in this case it must be valid
            CATCH_REQUIRE(v.is_valid());
            CATCH_REQUIRE(v.get_last_error().empty());
        }
        else
        {
if(v.is_valid())
std::cerr << "--- BAD: checked version [" << version << "], expected to be invalid; message: [" << error_msg << "]\n";
else if(v.get_last_error(false) != error_msg)
std::cerr << "--- BAD: checked version [" << version << "] invalid as expected, error message do not match, however: [" << v.get_last_error(false) << "] instead of [" << error_msg << "]\n";
            CATCH_REQUIRE_FALSE(v.is_valid());
            CATCH_REQUIRE(error_msg == v.get_last_error());
        }
    }
}


//constexpr char const g_valid_digits[]    = "0123456789";
//constexpr std::size_t const g_valid_digits_length = std::size(g_valid_digits) - 1;

// WARNING: the alphanum is used by "invalid tests" and it includes a '.'
//          which will cause issues in valid tests (i.e. two periods one after
//          the other)
//
constexpr char const g_valid_alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:-+.~";
constexpr std::size_t const g_valid_alphanum_length = std::size(g_valid_alphanum) - 1;

// all of the following support a '.' but we handle it specially to avoid
//
//     1. periods at the end
//     2. two periods in a row
//
constexpr char const g_valid_letters[]   = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+~";
constexpr std::size_t const g_valid_letters_length = std::size(g_valid_letters) - 1;

constexpr char const g_valid_letters_colon[]   = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+~:";
constexpr std::size_t const g_valid_letters_colon_length = std::size(g_valid_letters_colon) - 1;

constexpr char const g_valid_letters_dash[]   = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+~-";
constexpr std::size_t const g_valid_letters_dash_length = std::size(g_valid_letters_dash) - 1;

constexpr char const g_valid_all_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+~:-";
constexpr std::size_t const g_valid_all_chars_length = std::size(g_valid_all_chars) - 1;


std::string generate_number()
{
    versiontheca::part_integer_t value;
    SNAP_CATCH2_NAMESPACE::random(value);
    return std::to_string(value);
}


std::string generate_word(char const * valid_chars, std::size_t length)
{
    std::string v;
    int const size(rand() % 10 + 1);
    for(int j(0); j < size; ++j)
    {
        v += valid_chars[rand() % length];
    }
    while(!v.empty()
       && v.back() == '-')
    {
        v.back() = valid_chars[rand() % length];
    }
    return v;
}


std::string generate_version(
      std::size_t max
    , char const * valid_chars
    , std::size_t length
    , bool prepend_number = true)
{
    std::string v;
    std::size_t i(0);
    if(prepend_number)
    {
        v += generate_number();
        ++i;
        prepend_number = false;
    }
    for(; i < max; ++i)
    {
        if(!v.empty()
        && rand() % 100 < 10)
        {
            v += '.';
        }
        if(prepend_number)
        {
            v += generate_number();
        }
        else
        {
            v += generate_word(valid_chars, length);
        }
    }
    return v;
}


//DEBIAN_PACKAGE_EXPORT int validate_debian_version(const char *string, char *error_string, size_t error_size);
//DEBIAN_PACKAGE_EXPORT debian_version_handle_t string_to_debian_version(const char *string, char *error_string, size_t error_size);
//DEBIAN_PACKAGE_EXPORT int debian_version_to_string(const debian_version_handle_t debian_version, char *string, size_t string_size);
//DEBIAN_PACKAGE_EXPORT int debian_versions_compare(const debian_version_handle_t left, const debian_version_handle_t right);
//DEBIAN_PACKAGE_EXPORT void delete_debian_version(debian_version_handle_t debian_version);
}
// no name namespace


CATCH_TEST_CASE("debian_versions", "[valid]")
{
    CATCH_START_SECTION("debian_versions: verify test checker for version 1.0")
    {
        check_version("1.0", std::string());
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("debian_versions: many valid versions")
    {
        // many valid versions generated randomly to increase the likelyhood
        // of things I would otherwise not think of
        //
        for(int i(0); i < 10'000; ++i)
        {
            int const parts(i % 25 + 1);

            // simple version (no epoch/revision)
            {
                std::string v(generate_version(parts, g_valid_letters, g_valid_letters_length));
                check_version(v.c_str(), std::string());
            }

            // epoch + version
            if(parts > 1)
            {
                std::stringstream ss;
                ss << generate_number() << ':';
                ss << generate_version(parts - 1, g_valid_letters_colon, g_valid_letters_colon_length);
                check_version(ss.str(), std::string());
            }

            // version + revision
            if(parts > 1)
            {
                std::string v(generate_version(std::max(1UL, parts / 2UL), g_valid_letters_dash, g_valid_letters_dash_length));
                v += '-';
                v += generate_version(std::max(1UL, parts / 2UL), g_valid_letters, g_valid_letters_length, false);
                check_version(v.c_str(), std::string());
            }

            // epoch + version + revision
            if(parts > 2)
            {
                std::stringstream ss;
                ss << generate_number() << ':'
                      // anything can appear in upstream version when we have an epoch & revision
                   << generate_version(std::max(1UL, parts / 2UL), g_valid_all_chars, g_valid_all_chars_length)
                   << '-'
                      // no dashes, no colons in revisions
                   << generate_version(std::max(1UL, parts / 2UL), g_valid_letters, g_valid_letters_length, false);
                check_version(ss.str(), std::string());
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("next_previous_debian_versions", "[valid]")
{
    CATCH_START_SECTION("next_previous_debian_versions: next/previous at level 4, 3, 2, 1, 0")
    {
        {
            versiontheca::versiontheca::pointer_t a(create("1.3.2"));
            CATCH_REQUIRE(a->next(4));
            CATCH_REQUIRE(a->get_version() == "1.3.2.0.1");
            CATCH_REQUIRE(a->previous(4));
            CATCH_REQUIRE(a->get_version() == "1.3.2"); // +1 -1, back to original
            CATCH_REQUIRE(a->previous(4));
            CATCH_REQUIRE(a->get_version() == "1.3.1.4294967295.4294967295");
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

    CATCH_START_SECTION("next_previous_debian_versions: next/previous with epoch")
    {
        versiontheca::versiontheca::pointer_t a(create("75:1.5.3"));
        CATCH_REQUIRE(a->next(2));
        CATCH_REQUIRE(a->get_version() == "75:1.5.4");
        CATCH_REQUIRE(a->previous(2));
        CATCH_REQUIRE(a->get_version() == "75:1.5.3");
        CATCH_REQUIRE(a->previous(2));
        CATCH_REQUIRE(a->get_version() == "75:1.5.2");
        CATCH_REQUIRE(a->next(2));
        CATCH_REQUIRE(a->get_version() == "75:1.5.3");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("next_previous_debian_versions: next/previous with release")
    {
        versiontheca::versiontheca::pointer_t a(create("1.5.3-r5"));
        CATCH_REQUIRE(a->next(2));
        CATCH_REQUIRE(a->get_version() == "1.5.4-r5");
        CATCH_REQUIRE(a->previous(2));
        CATCH_REQUIRE(a->get_version() == "1.5.3-r5");
        CATCH_REQUIRE(a->previous(2));
        CATCH_REQUIRE(a->get_version() == "1.5.2-r5");
        CATCH_REQUIRE(a->next(2));
        CATCH_REQUIRE(a->get_version() == "1.5.3-r5");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("next_previous_debian_versions: previous/next with release")
    {
        versiontheca::versiontheca::pointer_t a(create("5:1.5.3-r5"));
        CATCH_REQUIRE(a->previous(4));
        CATCH_REQUIRE(a->get_version() == "5:1.5.2.4294967295.4294967295-r5");
        CATCH_REQUIRE(a->next(4));
        CATCH_REQUIRE(a->get_version() == "5:1.5.3-r5");
        CATCH_REQUIRE(a->next(4));
        CATCH_REQUIRE(a->get_version() == "5:1.5.3.0.1-r5");
        CATCH_REQUIRE(a->previous(4));
        CATCH_REQUIRE(a->get_version() == "5:1.5.3-r5");
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_debian_versions", "[invalid]")
{
    CATCH_START_SECTION("invalid_debian_versions: empty")
    {
        // empty
        //
        // note: the empty version is "invalid" as far as versions go,
        //       but it does not generetate an error message
        //
        //       -- the check_version() cannot be used here because ""
        //          is the empty string and that means a valid version
        //
        versiontheca::debian::pointer_t t(std::make_shared<versiontheca::debian>());
        versiontheca::versiontheca v(t, "");
        CATCH_REQUIRE_FALSE(v.is_valid());
        CATCH_REQUIRE(v.get_last_error().empty());

        CATCH_REQUIRE(v.get_version().empty());
        CATCH_REQUIRE(v.get_last_error() == "no parts to output.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_debian_versions: various invalid epoch")
    {
        // epoch
        //
        check_version("3A3:1.2.3-pre55", "epoch must be a valid integer.");
        check_version("33:-55", "a version value cannot be an empty string.");
        check_version(":", "invalid ':' and/or '-' positions in \":\".");
        check_version("a:", "epoch must be a valid integer.");
        check_version("-10:", "invalid ':' and/or '-' positions in \"-10:\".");
        check_version("99999999999999999:", "integer too large for a valid version.");
        check_version("3:", "a version value cannot be an empty string.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_debian_versions: revision")
    {
        // revision
        check_version("-", "invalid ':' and/or '-' positions in \"-\".");
        check_version("--", "a debian version must always start with a number \"--\".");
        check_version("+-", "a debian version must always start with a number \"+-\".");
        check_version("#-", "found unexpected character: \\U000023 in input.");
        check_version("55:435123-", "a version value cannot be an empty string.");
        check_version("-a", "invalid ':' and/or '-' positions in \"-a\".");
        check_version("-0", "invalid ':' and/or '-' positions in \"-0\".");
        check_version("-+", "invalid ':' and/or '-' positions in \"-+\".");
        check_version("-3$7", "invalid ':' and/or '-' positions in \"-3$7\".");
        check_version("32:1.2.55-3:7", "found unexpected character: \\U00003A in input.");
        check_version("-3.7", "invalid ':' and/or '-' positions in \"-3.7\".");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_debian_versions: version")
    {
        // version
        //
        check_version("3.7#", "found unexpected character: \\U000023 in input.");
        check_version("3$7", "found unexpected character: \\U000024 in input.");
        check_version("3;7", "found unexpected character: \\U00003B in input.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_debian_versions: randomized")
    {
        // do another loop for some random unicode characters
        //
        for(int i(1); i < 128; ++i)
        {
            //char const valid_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:-+.~";
            char c(static_cast<char>(i));
            if(strchr(g_valid_alphanum, c) != nullptr)
            {
                // skip all valid characters
                continue;
            }
            std::string v;
            std::size_t bad_at(1000);
            for(int j(0); j < 12; ++j)
            {
                if(v.empty()
                || v[v.length() - 1] == '-'
                || v[v.length() - 1] == ':')
                {
                    std::stringstream ss;
                    ss << rand() % 10;
                    v += ss.str();
                }
                if(j == 6)
                {
                    // add the spurious character now
                    //
                    bad_at = v.length();
                    v += c;
                }
                char vc(g_valid_alphanum[rand() % g_valid_alphanum_length]);
                if(!v.empty()
                && v.back() == '.'
                && (vc == ':' || vc == '-' || vc == '.'))
                {
                    v += 'N'; // add a nugget between '.' and '-'/':'
                }
                if(vc == ':')
                {
                    if(strchr(v.c_str(), ':') == nullptr)
                    {
                        // on first ':' ensure epoch is a number
                        //
                        std::string::size_type const p(v.find_first_not_of("0123456789"));
                        if(p != std::string::npos)
                        {
                            // not a number, create such
                            //
                            std::string const epoch(generate_number());
                            v = epoch + ":" + v;
                            bad_at += epoch.length() + 1;
                            continue;
                        }
                    }
                }
                v += vc;
            }
//std::cerr << "--- bad character is 0x" << static_cast<int>(c) << "\n";
            std::stringstream last_error;
            last_error << "found unexpected character: \\U"
                       << std::hex << std::uppercase << std::setfill('0')
                                   << std::setw(6) << static_cast<int>(c)
                       << " in input.";
            // check whether the bad character was inserted after the last dash
            {
                std::string::size_type const p(v.find_last_of("-"));
                if(p == std::string::npos)
                {
                    check_version(v.c_str(), last_error.str());
                }
                else
                {
                    if(p == v.length() - 1)
                    {
                        // avoid invalid (empty) revisions because that's not
                        // the purpose of this test
                        std::stringstream ss;
                        ss << rand() % 10;
                        v += ss.str();
                    }
                    if(p < bad_at)
                    {
                        // bad character ended up in the revision
                        check_version(v.c_str(), last_error.str());
                    }
                    else
                    {
                        if(v.find_first_of(':', p + 1) == std::string::npos)
                        {
                            check_version(v.c_str(), last_error.str());
                        }
                        else
                        {
                            // a revision does not accept a ':' character and since
                            // it is checked before the version we get that error
                            // instead instead of the version error...
                            check_version(v.c_str(), last_error.str());
                        }
                    }
                }
            }
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_debian_versions: max + 1 fails")
    {
        versiontheca::versiontheca::pointer_t a(create("4294967295.4294967295.4294967295"));
        CATCH_REQUIRE(a->is_valid());
        CATCH_REQUIRE_FALSE(a->next(2));
        CATCH_REQUIRE_FALSE(a->is_valid());
        CATCH_REQUIRE(a->get_last_error() == "maximum limit reached; cannot increment version any further.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_debian_versions: min - 1 fails")
    {
        versiontheca::versiontheca::pointer_t a(create("0.0"));
        CATCH_REQUIRE(a->is_valid());
        CATCH_REQUIRE_FALSE(a->previous(2));
        CATCH_REQUIRE_FALSE(a->is_valid());
        CATCH_REQUIRE(a->get_last_error() == "minimum limit reached; cannot decrement version any further.");
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("bad_calls", "[invalid]")
{
    CATCH_START_SECTION("bad_calls: next without a version")
    {
        versiontheca::debian::pointer_t t(std::make_shared<versiontheca::debian>());
        versiontheca::versiontheca v(t);
        CATCH_REQUIRE_FALSE(v.next(0));
        CATCH_REQUIRE(v.get_last_error() == "no parts in this Debian version; cannot computer next/previous.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bad_calls: previous without a version")
    {
        versiontheca::debian::pointer_t t(std::make_shared<versiontheca::debian>());
        versiontheca::versiontheca v(t);
        CATCH_REQUIRE_FALSE(v.previous(0));
        CATCH_REQUIRE(v.get_last_error() == "no parts in this Debian version; cannot computer next/previous.");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("bad_calls: next out of bounds")
    {
        versiontheca::versiontheca::pointer_t a(create("1.5.3-r5"));
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

    CATCH_START_SECTION("bad_calls: previous out of bounds")
    {
        versiontheca::versiontheca::pointer_t a(create("1.5.3-r5"));
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
}



// vim: ts=4 sw=4 et
