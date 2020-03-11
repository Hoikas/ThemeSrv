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

#include "build_info.h"
#include <iostream>
#include <string_theory/iostream>
#include <string_theory/st_stringstream.h>

// =================================================================================

namespace theme
{
    namespace buildinfo
    {
        extern const char* BUILD_HASH;
        extern const char* BUILD_TAG;
        extern const char* BUILD_BRANCH;
        extern const char* BUILD_DATE;
        extern const char* BUILD_TIME;
    };
};

// =================================================================================

const char* theme::build_date()
{
    return theme::buildinfo::BUILD_DATE;
}

const char* theme::build_hash()
{
    return theme::buildinfo::BUILD_HASH;
}

const char* theme::build_branch()
{
    return theme::buildinfo::BUILD_BRANCH;
}

const char* theme::build_tag()
{
    return theme::buildinfo::BUILD_TAG;
}

const char* theme::build_time()
{
    return theme::buildinfo::BUILD_TIME;
}

const char* theme::build_version()
{
    if (*theme::buildinfo::BUILD_TAG == 0)
        return theme::buildinfo::BUILD_HASH;
    else
        return theme::buildinfo::BUILD_TAG;
}

// =================================================================================

ST::string theme::build_info()
{
    ST::string_stream stream;
    stream << "THEME - The H'uru Enhanced MOULa Experience";
    if (*theme::buildinfo::BUILD_TAG != 0)
        stream << " (" << theme::buildinfo::BUILD_TAG << ")";
    stream << " [" << theme::buildinfo::BUILD_HASH << " (" << theme::buildinfo::BUILD_BRANCH << ")] ";
    stream << theme::buildinfo::BUILD_DATE << " " << theme::buildinfo::BUILD_TIME << " ";
#if defined(__clang__)
    stream << "clang++ " << __clang_version__;
#elif defined(__GNUC__)
    stream << "g++ " << __VERSION__;
#endif

    // Don't whine at me about this because I don't care right now.
    if (sizeof(void*) == 4)
        stream << " (x86)";
    else if (sizeof(void*) == 8)
        stream << " (x64)";
    return stream.to_string();
}

void theme::build_info(std::ostream& stream)
{
    stream << theme::build_info() << std::endl;
}
