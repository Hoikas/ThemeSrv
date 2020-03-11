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

#include "log.h"

#include <ctime>
#include <iostream>

// =================================================================================

static char s_time[64]{};

theme::log::level theme::log::s_level = theme::log::level::e_info;

// =================================================================================

void theme::log::write_line(theme::log::level level, const char* msg)
{
    // No need for any fancy pants log file handling here. This is a simple, single threaded
    // file server. We'll just dump directly to stdout and tee to a file.
    std::cout << s_time << ' ' << m_name.c_str();
    switch (level) {
    case theme::log::level::e_debug:
        std::cout << " DEBUG:";
        break;
    case theme::log::level::e_info:
        std::cout << " INFO:";
        break;
    case theme::log::level::e_warning:
        std::cout << " WARNING:";
        break;
    case theme::log::level::e_error:
        std::cout << " ERROR:";
        break;
    }
    std::cout << ' ' << msg << std::endl;
}

// =================================================================================

void theme::log::tick()
{
    std::time_t rawtime;
    std::time(&rawtime);
    std::tm* timeinfo = std::gmtime(&rawtime);
    std::strftime(s_time, sizeof(s_time), "[%Y-%m-%d %H:%M:%S]", timeinfo);
}
