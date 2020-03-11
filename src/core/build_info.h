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

#ifndef __THEME_BUILD_INFO_H
#define __THEME_BUILD_INFO_H

#include <iosfwd>
#include <string_theory/string>

namespace theme
{
    const char* build_date();
    const char* build_hash();
    const char* build_tag();
    const char* build_branch();
    const char* build_time();
    const char* build_version();

    void build_info(std::ostream&);
    ST::string build_info();
};

#endif
