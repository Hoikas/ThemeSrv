/*   This file is part of ThemeSrv.
 *
 *   ThemeSrv is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   ThemeSrv is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with ThemeSrv.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "errors.h"

#include <iostream>

// =================================================================================

static void _default_assert_handler(const char* cond, const char* file, int line, const char* msg)
{
    std::cerr << std::endl;
    std::cerr << "Assertion Failed" << std::endl;
#ifndef NDEBUG
    std::cerr << "Condition: " << cond << std::endl;
    std::cerr << "File: " << file << std::endl;
    std::cerr << "Line: " << line << std::endl;
    if (msg)
        std::cerr << "Message: " << msg << std::endl;
#endif // NDEBUG

    std::cout.flush();
    std::cerr.flush();
    __builtin_trap();
}

// =================================================================================

static theme::assert::handler_t s_assertHandler = _default_assert_handler;

// =================================================================================

void theme::assert::handle(const char* cond, const char* file, int line, const char* msg)
{
    if (s_assertHandler)
        s_assertHandler(cond, file, line, msg);
}

// =================================================================================

void theme::assert::reset_handler()
{
    s_assertHandler = _default_assert_handler;
}

void theme::assert::set_handler(const theme::assert::handler_t& handler)
{
    s_assertHandler = handler;
}
