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
#include <string_theory/string>

namespace theme
{
    class uuid
    {
        typedef unsigned char uuid_t[16];
        union uuid_le
        {
            uuid_t m_buf;
            struct
            {
                uint32_t m_data1;
                uint16_t m_data2;
                uint16_t m_data3;
                uint8_t  m_data4[8];
            };
        };

        uuid_t m_data{};

    public:
        uuid() {}

        uint8_t* data() { return m_data; }
        const uint8_t* data() const { return m_data; }

        /** Unsafe */
        void to_le_bytes(uint8_t* buf) const;

        template<size_t _Sz>
        void to_le(uint8_t(&buf)[_Sz]) const
        {
            static_assert(_Sz == sizeof(uuid_t));
            to_le_bytes(buf);
        }

        ST::string as_string() const;
        void clear() { memset(m_data, 0, sizeof(m_data)); }
        bool from_string(const ST::string& str) { return from_string(str.c_str()); }
        bool from_string(const char*);

        bool empty() const;
        bool equals(const uuid&) const;
        bool operator ==(const uuid& rhs) const { return equals(rhs); }
        bool operator <(const uuid& rhs) const;

        uuid& operator =(const uuid& rhs) { memcpy(m_data, rhs.m_data, sizeof(m_data)); return *this; }

    private:
        uuid(const uint8_t* const buf);

    public:
        template<size_t _Sz>
        static uuid from_bytes(const uint8_t(&buf)[_Sz])
        {
            static_assert(_Sz == sizeof(uuid_t));
            return uuid((const uint8_t*)buf);
        }

        template<size_t _Sz>
        static uuid from_le(const uint8_t(&buf)[_Sz])
        {
            static_assert(_Sz == sizeof(uuid_t));
            return from_le_bytes(buf);
        }

        /** Unsafe */
        static uuid from_le_bytes(const uint8_t* const buf)
        {
            uuid result(buf);

            // Win32 GUIDs are contaminated with the scourge known as little endian.
            uuid_le* guid = (uuid_le*)result.m_data;
            guid->m_data1 = __builtin_bswap32(guid->m_data1);
            guid->m_data2 = __builtin_bswap16(guid->m_data2);
            guid->m_data3 = __builtin_bswap16(guid->m_data3);
            return result;
        }

        static uuid generate();

        static uuid null;
    };
};

#endif
