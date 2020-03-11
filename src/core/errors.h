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

#ifndef __THEME_ERROR_H
#define __THEME_ERROR_H

#include <functional>
#include <string_theory/format>

#ifndef NDEBUG
#   define THEME_ASSERTD(cond) if ((cond) == 0) { theme::assert::handle(#cond, __FILE__, __LINE__, nullptr); }
#   define THEME_ASSERTD_V(cond, msg, ...) if ((cond) == 0) { theme::assert::_handle(#cond, __FILE__, __LINE__, msg, __VA_ARGS__); }
#else
#   define THEME_ASSERTD(cond) (cond);
#   define THEME_ASSERTD_V(cond, msg, ...) (cond);
#endif
#define THEME_ASSERTR(cond) if ((cond) == 0) { theme::assert::handle(#cond, __FILE__, __LINE__, nullptr); }
#define THEME_ASSERTR_V(cond, msg, ...) if ((cond) == 0) { theme::assert::_handle(#cond, __FILE__, __LINE__, msg, __VA_ARGS__); }

namespace theme
{
    class assert
    {
    public:
        typedef std::function<void(const char*, const char*, int, const char*)> handler_t;

        static void handle(const char* cond, const char* file, int line, const char* msg);
        template<typename... args>
        static inline void _handle(const char* cond, const char* file, int line, const char* fmt, args&&... _Args)
        {
            ST::string msg = ST::format(fmt, std::forward<args>(_Args)...);
            handle(cond, file, line, msg.c_str());
        }

        static void reset_handler();
        static void set_handler(const handler_t& handler);
    };
};

#endif
