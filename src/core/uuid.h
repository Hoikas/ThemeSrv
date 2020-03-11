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

#ifndef __THEME_UUID_H
#define __THEME_UUID_H

#include <cstdint>
#include <cstring>
#include <string_theory/string>

namespace theme
{
    class uuid
    {
        typedef unsigned char uuid_t[16];
        uuid_t m_data{};

    public:
        uuid() { }
        uuid(const void* data) { memcpy(m_data, data, sizeof(m_data)); }

        template<size_t _Sz>
        uuid(const uint8_t(*data)[_Sz])
        {
            static_assert(_Sz == 16);
            memcpy(m_data, data, sizeof(m_data));
        }

        uint8_t* data() { return m_data; }
        const uint8_t* data() const { return m_data; }

        ST::string as_string() const;
        void clear() { memset(m_data, 0, sizeof(m_data)); }
        bool from_string(const ST::string& str) { return from_string(str.c_str()); }
        bool from_string(const char*);

        bool empty() const;
        bool equals(const uuid&) const;
        bool operator ==(const uuid& rhs) const { return equals(rhs); }
        bool operator <(const uuid& rhs) const;

        uuid& operator =(const uuid& rhs) { memcpy(m_data, rhs.m_data, sizeof(m_data)); return *this; }

        static uuid generate();
        static uuid null;
    };
};

#endif
