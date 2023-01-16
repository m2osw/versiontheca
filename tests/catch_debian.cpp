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
#include    "versiontheca/versiontheca.h"


// C++
//
#include    <stdexcept>
#include    <cstring>




namespace
{

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

void check_version(char const * const version, char const * const error_msg)
{
    // validate_debian_version()
    {
        //char error_string[256];
        //strcpy(error_string, "no errors");
        versiontheca::debian::pointer_t t(std::make_shared<versiontheca::debian>());
        versiontheca::versiontheca v(t, version);
        //int valid(validate_debian_version(version, error_string, sizeof(error_string) / sizeof(error_string[0])));
//printf("from {%s} result = %d [%s] [%s]\n", print_version(version).c_str(), valid, error_string, error_msg);
        if(error_msg == 0)
        {
            // in this case it must be valid
            CATCH_REQUIRE(v.is_valid());
            //CATCH_REQUIRE(strcmp(error_string, "no errors") == 0); // err buffer unchanged
        }
        else
        {
            CATCH_REQUIRE_FALSE(v.is_valid());
            //CATCH_REQUIRE(strcmp(error_msg, error_string) == 0);
        }
    }
}

//DEBIAN_PACKAGE_EXPORT int validate_debian_version(const char *string, char *error_string, size_t error_size);
//DEBIAN_PACKAGE_EXPORT debian_version_handle_t string_to_debian_version(const char *string, char *error_string, size_t error_size);
//DEBIAN_PACKAGE_EXPORT int debian_version_to_string(const debian_version_handle_t debian_version, char *string, size_t string_size);
//DEBIAN_PACKAGE_EXPORT int debian_versions_compare(const debian_version_handle_t left, const debian_version_handle_t right);
//DEBIAN_PACKAGE_EXPORT void delete_debian_version(debian_version_handle_t debian_version);
}
// no name namespace


CATCH_TEST_CASE("valid_versions", "[valid]")
{
    CATCH_START_SECTION("valid_versions: verify test checker for version 1.0")
    {
        check_version("1.0", NULL);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("valid_versions: many valid versions")
    {
        // many valid versions generated randomly to increase the likelyhood
        // of things I would otherwise not think of
        //
        for(int i(0); i < 10000; ++i)
        {
            // WARNING: in the sizeof() before we do a -1 to ignore the '\0' at the end of the string
            //
            char const valid_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+.~:-";

            // simple version (no epoch/revision)
            {
                std::stringstream ss;
                ss << rand() % 10;
                std::string v(ss.str());
                int size(rand() % 20);
                for(int j(0); j < size; ++j)
                {
                    // we do not include the : and - from the valid chars
                    //
                    char c(valid_chars[rand() % (sizeof(valid_chars) / sizeof(valid_chars[0]) - 3)]);
                    v += c;
                }
                check_version(v.c_str(), NULL);
            }

            // epoch + version
            {
                std::stringstream ss;
                ss << rand() % 0x7FFFFFFF << (rand() & 1 ? ":" : ";") << rand() % 10;
                std::string v(ss.str());
                int size(rand() % 20);
                for(int j(0); j < size; ++j)
                {
                    // we do not include the - from the valid chars
                    char c(valid_chars[rand() % (sizeof(valid_chars) / sizeof(valid_chars[0]) - 2)]);
                    if(c == ':' && (rand() % 3) == 0)
                    {
                        c = ';';
                    }
                    v += c;
                }
                check_version(v.c_str(), nullptr);
            }

            // version + revision
            {
                std::stringstream ss;
                ss << rand() % 10;
                std::string v(ss.str());
                int vsize(rand() % 20);
                for(int j(0); j < vsize; ++j)
                {
                    // we do not include the - from the valid chars
                    char c(valid_chars[rand() % (sizeof(valid_chars) / sizeof(valid_chars[0]) - 2)]);
                    if(c == ':' || c == ';')
                    {
                        c = '-';
                    }
                    v += c;
                }
                v += '-';
                int rsize(rand() % 20 + 1);
                for(int j(0); j < rsize; ++j)
                {
                    // we do not include the : and - from the valid chars
                    char c(valid_chars[rand() % (sizeof(valid_chars) / sizeof(valid_chars[0]) - 3)]);
                    v += c;
                }
                check_version(v.c_str(), nullptr);
            }

            // epoch + version + revision
            {
                std::stringstream ss;
                ss << rand() % 0x7FFFFFFF << (rand() & 1 ? ":" : ";") << rand() % 10;
                std::string v(ss.str());
                int vsize(rand() % 20 + 1);
                for(int j(0); j < vsize; ++j)
                {
                    char c(valid_chars[rand() % (sizeof(valid_chars) / sizeof(valid_chars[0]) - 1)]);
                    if(c == ':' && (rand() & 3) == 0)
                    {
                        c = ';';
                    }
                    v += c;
                }
                v += '-';
                int rsize(rand() % 20 + 1);
                for(int j(0); j < rsize; ++j)
                {
                    // we do not include the : and - from the valid chars
                    char c(valid_chars[rand() % (sizeof(valid_chars) / sizeof(valid_chars[0]) - 3)]);
                    v += c;
                }
                check_version(v.c_str(), nullptr);
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("invalid_versions", "[invalid]")
{
    CATCH_START_SECTION("invalid_versions: empty")
    {
        // empty
        check_version("", "invalid version, digit expected as first character");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_versions: various invalid epoch")
    {
        // epoch
        check_version(":", "empty epoch");
        check_version(";", "empty epoch");
        check_version("a:", "non-decimal epoch");
        check_version("a;", "non-decimal epoch");
        check_version("-10:", "non-decimal epoch");
        check_version("-10;", "non-decimal epoch");
        check_version("99999999999999999:", "invalid decimal epoch");
        check_version("99999999999999999;", "invalid decimal epoch");
        check_version("3:", "invalid version, digit expected as first character");
        check_version("3;", "invalid version, digit expected as first character");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_versions: revision")
    {
        // revision
        check_version("-", "empty revision");
        check_version("--", "empty revision");
        check_version("+-", "empty revision");
        check_version("#-", "empty revision");
        check_version("55:435123-", "empty revision");
        check_version("55;435123-", "empty revision");
        check_version("-a", "invalid version, digit expected as first character");
        check_version("-0", "invalid version, digit expected as first character");
        check_version("-+", "invalid version, digit expected as first character");
        check_version("-3$7", "invalid character in revision");
        check_version("32:1.2.55-3:7", "invalid character in revision");
        check_version("32;1.2.55-3:7", "invalid character in revision");
        check_version("-3.7", "invalid version, digit expected as first character");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_versions: version")
    {
        // version
        check_version("3.7#", "invalid character in version");
        check_version("3$7", "invalid character in version");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("invalid_versions: randomized")
    {
        for(int i(1); i < 256; ++i)
        {
            const char valid_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:;-+.~";
            char c(static_cast<char>(i));
            if(strchr(valid_chars, c) != NULL)
            {
                // skip all valid characters
                continue;
            }
            std::string v;
            size_t bad_at(1000);
            for(int j(0); j < 12; ++j)
            {
                if(v.empty()
                || v[v.length() - 1] == '-'
                || v[v.length() - 1] == ':'
                || v[v.length() - 1] == ';')
                {
                    std::stringstream ss;
                    ss << rand() % 10;
                    v += ss.str();
                }
                if(j == 6)
                {
                    // add the spurious character now
                    bad_at = v.length();
                    v += c;
                }
                char vc(valid_chars[rand() % (sizeof(valid_chars) / sizeof(valid_chars[0]) - 1)]);
                if(vc == ':' || vc == ';')
                {
                    if(strchr(v.c_str(), ':') == NULL
                    || strchr(v.c_str(), ';') == NULL)
                    {
                        // on first ':' ensure epoch is a number
                        std::string::size_type p = v.find_first_not_of("0123456789");
                        if(p != std::string::npos)
                        {
                            // not a number, create such
                            std::stringstream ss;
                            if(v[0] < '0' || v[0] > '9')
                            {
                                // first character must be a digit
                                ss << rand() % 10;
                                v = ss.str() + v;
                            }
                            ss << rand();
                            v = ss.str() + (rand() & 1 ? ":" : ";") + v;
                            bad_at += ss.str().length() + 1;
                            continue;
                        }
                    }
                }
                v += vc;
            }
            // check whether the bad character was inserted after the last dash
            {
                std::string::size_type p(v.find_last_of("-"));
                if(p == std::string::npos)
                {
                    check_version(v.c_str(), "invalid character in version");
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
                        check_version(v.c_str(), "invalid character in revision");
                    }
                    else
                    {
                        if(v.find_first_of(':', p + 1) == std::string::npos
                        && v.find_first_of(';', p + 1) == std::string::npos)
                        {
                            check_version(v.c_str(), "invalid character in version");
                        }
                        else
                        {
                            // a revision does not accept a ':' character and since
                            // it is checked before the version we get that error
                            // instead instead of the version error...
                            check_version(v.c_str(), "invalid character in revision");
                        }
                    }
                }
            }
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
