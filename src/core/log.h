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

#ifndef __THEME_LOG_H
#define __THEME_LOG_H

#include <string_theory/format>

namespace theme
{
    class log
    {
    public:
        enum class level
        {
            e_debug,
            e_info,
            e_warning,
            e_error,
        };

    protected:
        ST::string m_name;
        level m_level;

        void write_line(level level, const char* msg);
        void write_line(level level, const ST::string& msg) { write_line(level, msg.c_str()); }

    public:
        log() = delete;
        log(const log&) = delete;
        log(log&&) = delete;

        log(ST::string name, level level=level::e_info)
            : m_name(std::move(name)), m_level(level)
        { }

        template<typename... _Args>
        void debug(const char* fmt, _Args&&... args)
        {
            if (m_level <= level::e_debug)
                write_line(level::e_debug, ST::format(fmt, std::forward<_Args...>(args)...));
        }

        void debug(const char* msg)
        {
            if (m_level <= level::e_debug)
                write_line(level::e_debug, msg);
        }

        template<typename... _Args>
        void info(const char* fmt, _Args&&... args)
        {
            if (m_level <= level::e_info)
                write_line(level::e_info, ST::format(fmt, std::forward<_Args...>(args)...));
        }

        void info(const char* msg)
        {
            if (m_level <= level::e_info)
                write_line(level::e_info, msg);
        }

        template<typename... _Args>
        void warning(const char* fmt, _Args&&... args)
        {
            if (m_level <= level::e_warning)
                write_line(level::e_warning, ST::format(fmt, std::forward<_Args...>(args)...));
        }

        void warning(const char* msg)
        {
            if (m_level <= level::e_warning)
                write_line(level::e_warning, msg);
        }

        template<typename... _Args>
        void error(const char* fmt, _Args&&... args)
        {
            if (m_level <= level::e_error)
                write_line(level::e_error, ST::format(fmt, std::forward<_Args...>(args)...));
        }

        void error(const char* msg)
        {
            if (m_level <= level::e_error)
                write_line(level::e_error, msg);
        }

    public:
        static void tick();
    };
};

#endif
