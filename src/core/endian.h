/*   This file is part of theme.
*
*   theme is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Affero General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   theme is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Affero General Public License for more details.
*
*   You should have received a copy of the GNU Affero General Public License
*   along with theme.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __THEME_ENDIAN_H
#define __THEME_ENDIAN_H

#include <cstdint>
#include "theme_config.h"

namespace theme
{
    inline uint16_t swap_endian(uint16_t value)
    {
        return __builtin_bswap16(value);
    }

    inline uint32_t swap_endian(uint32_t value)
    {
        return __builtin_bswap32(value);
    }

    inline uint64_t swap_endian(uint64_t value)
    {
        return __builtin_bswap64(value);
    }
};

#ifdef THEME_BIG_ENDIAN
#   define THEME_BE16(x) x
#   define THEME_BE32(x) x
#   define THEME_BE64(x) x
#   define THEME_LE16(x) theme::swap_endian(x);
#   define THEME_LE32(x) theme::swap_endian(x);
#   define THEME_LE64(x) theme::swap_endian(x);
#else
#   define THEME_BE16(x) theme::swap_endian(x);
#   define THEME_BE32(x) theme::swap_endian(x);
#   define THEME_BE64(x) theme::swap_endian(x);
#   define THEME_LE16(x) x
#   define THEME_LE32(x) x
#   define THEME_LE64(x) x
#endif

#endif
