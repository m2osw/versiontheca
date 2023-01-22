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
 * \brief The implementation of the versiontheca tool.
 *
 * This file is the implementation of the versiontheca tool. It uses the
 * versiontheca library to parse versions and compare them against each
 * other.
 */

// versiontheca
//
#include    <versiontheca/basic.h>
#include    <versiontheca/debian.h>
#include    <versiontheca/decimal.h>
#include    <versiontheca/exception.h>
#include    <versiontheca/rpm.h>
#include    <versiontheca/version.h>
#include    <versiontheca/versiontheca.h>



// snapdev
//
#include    <snapdev/pathinfo.h>
#include    <snapdev/not_reached.h>


// C++
//
#include    <cstring>
#include    <iostream>




namespace
{


enum class version_type_t
{
    VERSION_TYPE_DEFAULT,

    VERSION_TYPE_BASIC,
    VERSION_TYPE_DEBIAN,
    VERSION_TYPE_DECIMAL,
    VERSION_TYPE_RPM,
};

enum class function_t
{
    FUNCTION_DEFAULT,

    FUNCTION_CANONICALIZE,
    FUNCTION_COMPARE,
    FUNCTION_NEXT,
    FUNCTION_PREVIOUS,
    FUNCTION_VALIDATE,
};

function_t                  g_function = function_t::FUNCTION_COMPARE;
int                         g_limit = 0;
int                         g_errcnt = 0;
int                         g_position = -1;
std::string                 g_format = std::string();
std::string                 g_progname = std::string();
std::vector<std::string>    g_versions = std::vector<std::string>();
version_type_t              g_version_type = version_type_t::VERSION_TYPE_DEFAULT;


void set_function(function_t f)
{
    if(g_function != function_t::FUNCTION_DEFAULT)
    {
        ++g_errcnt;
        std::cerr << "error: only one of --canonicalize, --compare, --next, --previous, --validate can be used on the command line.\n";
        exit(1);
    }
    g_function = f;
}


void set_version_type(version_type_t t)
{
    if(g_version_type != version_type_t::VERSION_TYPE_DEFAULT)
    {
        ++g_errcnt;
        std::cerr << "error: only one of --basic, --debian, or --decimal can be used on the command line.\n";
        return;
    }
    g_version_type = t;
}



}


void usage()
{
    std::cout
        << "Usage: " << g_progname << " [--opts] <version> ...\n"
           "where --opts is one or more of:\n"
           "  -b | --basic         read versions as basic versions\n"
           "  -C | --canonicalize  print version back canonicalized\n"
           "  -c | --compare       compare versions (this is the default)\n"
           "  -d | --debian        read versions as Debian versions\n"
           "  -F | --decimal       read versions as decimal numbers\n"
           "  -h | --help          print out this help screen\n"
           "  -l | --limit <N>     compare the first N parts\n"
           "  -n | --next <N>      compute next versions\n"
           "  -p | --previous <N>  compute previous versions\n"
           "  -r | --rpm           read versions as RPM versions\n"
           "  -v | --validate      validate versions (instead of comparing)\n"
           "  -V | --version       print out the version\n"
           "\n"
           "when used with --compare, use: <version1> <operator> <version2>\n"
           "where the operator is one of:\n"
           "  == | eq              return true if version1 is equal to version2\n"
           "  != | ne              return true if version1 is not equal to version2\n"
           "  <  | lt              return true if version1 is before version2\n"
           "  <= | le              return true if version1 is before or equal to version2\n"
           "  >  | gt              return true if version1 is after version2\n"
           "  >= | ge              return true if version1 is after or equal to version2\n";
}


versiontheca::versiontheca::pointer_t create_version(std::string const & v)
{
    versiontheca::trait::pointer_t t;
    switch(g_version_type)
    {
    case version_type_t::VERSION_TYPE_DEFAULT:
        throw versiontheca::logic_error("create_version() called with version type still set to 'DEFAULT'.");

    case version_type_t::VERSION_TYPE_BASIC:
        t = std::make_shared<versiontheca::basic>();
        break;

    case version_type_t::VERSION_TYPE_DEBIAN:
        t = std::make_shared<versiontheca::debian>();
        break;

    case version_type_t::VERSION_TYPE_RPM:
        t = std::make_shared<versiontheca::rpm>();
        break;

    case version_type_t::VERSION_TYPE_DECIMAL:
        t = std::make_shared<versiontheca::decimal>();
        break;

    }
    return std::make_shared<versiontheca::versiontheca>(t, v);
}


void compare()
{
    if(g_versions.size() != 3)
    {
        std::cerr << "error: in --compare mode, you must specified exactly three parameters: <version1> <operator> <version2>\n";
        ++g_errcnt;
        return;
    }

    versiontheca::versiontheca::pointer_t version1(create_version(g_versions[0]));
    if(!version1->is_valid())
    {
        std::cerr
            << "error: invalid left hand side version \""
            << g_versions[0]
            << "\n";
        ++g_errcnt;
        return;
    }

    versiontheca::versiontheca::pointer_t version2(create_version(g_versions[2]));
    if(!version2->is_valid())
    {
        std::cerr
            << "error: invalid right hand side version \""
            << g_versions[2]
            << "\n";
        ++g_errcnt;
        return;
    }

    std::string op(g_versions[1]);

    if(op == "=="
    || op == "="        // extension, there is assignment so just one '=' is fine
    || op == "eq")
    {
        exit(*version1 == *version2 ? 0 : 1);
        return;
    }

    if(op == "!="
    || op == "<>"       // extension, like SQL
    || op == "ne")
    {
        exit(*version1 != *version2 ? 0 : 1);
        return;
    }

    if(op == "<"
    || op == "lt")
    {
        exit(*version1 < *version2 ? 0 : 1);
        return;
    }

    if(op == "<="
    || op == "le")
    {
        exit(*version1 <= *version2 ? 0 : 1);
        return;
    }

    if(op == ">"
    || op == "gt")
    {
        exit(*version1 > *version2 ? 0 : 1);
        return;
    }

    if(op == ">="
    || op == "ge")
    {
        exit(*version1 >= *version2 ? 0 : 1);
        return;
    }

    std::cerr
        << "error: unrecognized operator \""
        << op
        << "\".\n";

    ++g_errcnt;
}


void canonicalize(bool display)
{
    if(g_versions.empty())
    {
        std::cerr << "error: in --canonicalize or --validate mode, you must specified at least one version.\n";
        ++g_errcnt;
        return;
    }

    for(auto const & v : g_versions)
    {
        versiontheca::versiontheca::pointer_t version(create_version(v));
        if(!version->is_valid())
        {
            std::cerr
                << "error: version \""
                << v
                << "\" is not considered valid: "
                << version->get_last_error()
                << "\n";
            ++g_errcnt;
        }
        else if(display)
        {
            std::cout << version->get_version() << '\n';
        }
    }

    exit(g_errcnt > 0 ? 1 : 0);
}


void next()
{
    if(g_versions.empty())
    {
        std::cerr << "error: in --next mode, you must specified at least one version.\n";
        ++g_errcnt;
        return;
    }

    versiontheca::versiontheca::pointer_t format;
    if(!g_format.empty())
    {
        format = create_version(g_format);
        if(!format->is_valid())
        {
            std::cerr
                << "error: format version \""
                << g_format
                << "\" is not valid.\n";
            ++g_errcnt;
            return;
        }
    }

    for(auto const & v : g_versions)
    {
        versiontheca::versiontheca::pointer_t version(create_version(v));
        if(!version->is_valid())
        {
            std::cerr
                << "error: version \""
                << v
                << "\" is not valid.\n";
            ++g_errcnt;
        }
        else
        {
            if(format != nullptr)
            {
                version->set_format(*format);
            }
            if(g_position < 0)
            {
                g_position = version->size() - 1;
            }
            if(version->next(g_position))
            {
                std::cout << version->get_version() << '\n';
            }
            else
            {
                std::cerr
                    << "error: could not compute next version for \""
                    << v
                    << "\".\n";
                ++g_errcnt;
            }
        }
    }

    exit(g_errcnt > 0 ? 1 : 0);
}


void previous()
{
    if(g_versions.empty())
    {
        std::cerr << "error: in --previous mode, you must specified at least one version.\n";
        ++g_errcnt;
        return;
    }

    versiontheca::versiontheca::pointer_t format;
    if(!g_format.empty())
    {
        format = create_version(g_format);
        if(!format->is_valid())
        {
            std::cerr
                << "error: format version \""
                << g_format
                << "\" is not valid.\n";
            ++g_errcnt;
            return;
        }
    }

    for(auto const & v : g_versions)
    {
        versiontheca::versiontheca::pointer_t version(create_version(v));
        if(!version->is_valid())
        {
            std::cerr
                << "error: version \""
                << v
                << "\" is not valid.\n";
            ++g_errcnt;
        }
        else
        {
            if(format != nullptr)
            {
                version->set_format(*format);
            }
            if(g_position < 0)
            {
                g_position = version->size() - 1;
            }
            if(version->previous(g_position))
            {
                std::cout << version->get_version() << '\n';
            }
            else
            {
                std::cerr
                    << "error: could not compute next version for \""
                    << v
                    << "\".\n";
                ++g_errcnt;
            }
        }
    }

    exit(g_errcnt > 0 ? 1 : 0);
}


int main(int argc, char * argv[])
{
    g_progname = snapdev::pathinfo::basename(std::string(argv[0]));

    g_function = function_t::FUNCTION_DEFAULT;

    for(int i(1); i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i], "--version") == 0
            || strcmp(argv[i], "-V") == 0)
            {
                std::cout << VERSIONTHECA_VERSION_STRING << '\n';
                return 3;
            }
            if(strcmp(argv[i], "--help") == 0
            || strcmp(argv[i], "-h") == 0
            || strcmp(argv[i], "-?") == 0)
            {
                usage();
                return 3;
            }
            if(strcmp(argv[i], "--canonicalize") == 0
            || strcmp(argv[i], "-C") == 0)
            {
                set_function(function_t::FUNCTION_CANONICALIZE);
                continue;
            }
            if(strcmp(argv[i], "--compare") == 0
            || strcmp(argv[i], "-c") == 0)
            {
                set_function(function_t::FUNCTION_COMPARE);
                continue;
            }
            if(strcmp(argv[i], "--next") == 0
            || strcmp(argv[i], "-n") == 0)
            {
                set_function(function_t::FUNCTION_NEXT);
                ++i;
                if(i >= argc)
                {
                    std::cerr << "error: the --next option must be followed by a valid number.\n";
                    return 2;
                }
                g_position = std::atol(argv[i]) - 1;
                if(static_cast<std::size_t>(g_position) >= versiontheca::MAX_PARTS)
                {
                    std::cerr
                        << "error: the --next option must be followed by a number between 1 and "
                        << versiontheca::MAX_PARTS
                        << ".\n";
                    return 2;
                }
                continue;
            }
            if(strcmp(argv[i], "--previous") == 0
            || strcmp(argv[i], "-p") == 0)
            {
                set_function(function_t::FUNCTION_PREVIOUS);
                ++i;
                if(i >= argc)
                {
                    std::cerr << "error: the --previous option must be followed by a valid number.\n";
                    return 2;
                }
                g_position = std::atol(argv[i]) - 1;
                if(static_cast<std::size_t>(g_position) >= versiontheca::MAX_PARTS)
                {
                    std::cerr
                        << "error: the --previous option must be followed by a number between 1 and "
                        << versiontheca::MAX_PARTS
                        << ".\n";
                    return 2;
                }
                continue;
            }
            if(strcmp(argv[i], "--format") == 0
            || strcmp(argv[i], "-f") == 0)
            {
                ++i;
                if(i >= argc)
                {
                    std::cerr << "error: the --format option must be followed by a valid version representing the format.\n";
                    return 2;
                }
                g_format = argv[i];
                continue;
            }
            if(strcmp(argv[i], "--validate") == 0
            || strcmp(argv[i], "-v") == 0)
            {
                set_function(function_t::FUNCTION_VALIDATE);
                continue;
            }
            if(strcmp(argv[i], "--limit") == 0
            || strcmp(argv[i], "-l") == 0)
            {
                ++i;
                if(i >= argc)
                {
                    std::cerr << "error: the --limit option must be followed by a valid number.\n";
                    return 2;
                }
                g_limit = std::atol(argv[i]);
                continue;
            }
            if(strcmp(argv[i], "--debian") == 0
            || strcmp(argv[i], "-d") == 0)
            {
                set_version_type(version_type_t::VERSION_TYPE_DEBIAN);
                continue;
            }
            if(strcmp(argv[i], "--rpm") == 0
            || strcmp(argv[i], "-r") == 0)
            {
                set_version_type(version_type_t::VERSION_TYPE_RPM);
                continue;
            }
            if(strcmp(argv[i], "--decimal") == 0
            || strcmp(argv[i], "-F") == 0)
            {
                set_version_type(version_type_t::VERSION_TYPE_DECIMAL);
                continue;
            }
            if(strcmp(argv[i], "--basic") == 0
            || strcmp(argv[i], "-b") == 0)
            {
                set_version_type(version_type_t::VERSION_TYPE_BASIC);
                continue;
            }
        }
        else
        {
            g_versions.push_back(argv[i]);
        }
    }

    // still the default, change to "compare"
    //
    if(g_function == function_t::FUNCTION_DEFAULT)
    {
        set_function(function_t::FUNCTION_COMPARE);
    }

    // still the default, change to "debian"
    //
    if(g_version_type == version_type_t::VERSION_TYPE_DEFAULT)
    {
        set_version_type(version_type_t::VERSION_TYPE_DEBIAN);
    }

    switch(g_function)
    {
    case function_t::FUNCTION_DEFAULT:
        throw versiontheca::logic_error("main() found function type still set to 'DEFAULT'.");

    case function_t::FUNCTION_COMPARE:
        compare();
        break;

    case function_t::FUNCTION_CANONICALIZE:
        canonicalize(true);
        break;

    case function_t::FUNCTION_VALIDATE:
        canonicalize(false);
        break;

    case function_t::FUNCTION_NEXT:
        next();
        break;

    case function_t::FUNCTION_PREVIOUS:
        previous();
        break;

    }

    return g_errcnt > 0 ? 2 : 0;
}


// vim: ts=4 sw=4 et
