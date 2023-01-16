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
#pragma once

// libexcept
//
#include    <libexcept/exception.h>



namespace versiontheca
{



DECLARE_LOGIC_ERROR(logic_error);
DECLARE_LOGIC_ERROR(overflow);

DECLARE_MAIN_EXCEPTION(versiontheca_exception);

DECLARE_EXCEPTION(versiontheca_exception, empty);
DECLARE_EXCEPTION(versiontheca_exception, invalid_parameter);
DECLARE_EXCEPTION(versiontheca_exception, missing_pointer);
DECLARE_EXCEPTION(versiontheca_exception, wrong_type);



} // namespace versiontheca
// vim: ts=4 sw=4 et
