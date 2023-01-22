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
 * \brief Generate a table helping with sorting RPM characters.
 *
 * This tool generates a table that sorts RPM characters found in strings.
 *
 * The following list shows the special order in RPM versions:
 *
 * \li `'~'` -- always first
 * \li `'\0'` -- represents the empty string
 * \li `+`
 * \li `A` to `Z` -- uppercase letters
 * \li `a` to `z` -- lowercase letters
 * \li `^` -- always last
 *
 * The `-` defines a release and is never compared (it cannot otherwise appear
 * in the version strings).
 *
 * The `:` defines an epoch. It cannot appear anywhere else.
 *
 * The `.` is viewed as a separator in our system so it never gets compared.
 * Because of that, it is not necessary in our table.
 *
 * \note
 * Digits form numbers that are a separate part and compared as integers
 * (opposed to strings because "10" \< "9" when comparing as strings).
 */

// C++
//
#include    <cstdint>
#include    <iomanip>
#include    <iostream>



namespace
{


constexpr std::uint8_t const g_supported_characters[] =
{
    '~',
    '\0',
    '+',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '^',
};


}

int main()
{
    char table[256] = {};

    for(std::size_t idx(0); idx < sizeof(g_supported_characters); ++idx)
    {
        table[static_cast<int>(g_supported_characters[idx])] = idx + 1;
    }

    std::cout << "constexpr std::uint8_t const g_rpm_compare_characters[] = {\n";
    std::cout << std::hex << std::uppercase << std::setfill('0');
    for(std::size_t idx(0); idx < sizeof(table); ++idx)
    {
        std::cout << "0x" << std::setw(2) << static_cast<int>(table[idx]) << ',';
        if(idx % 16 == 15)
        {
            std::cout << '\n';
        }
    }
    std::cout << "};\n";

    return 0;
}

// vim: ts=4 sw=4 et
