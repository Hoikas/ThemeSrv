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
#include "uuid.h"

#include <uuid/uuid.h>

// =================================================================================

theme::uuid theme::uuid::null{};

theme::uuid::uuid(const uint8_t* const buf)
{
    memcpy(m_data, buf, sizeof(m_data));
}

theme::uuid theme::uuid::generate()
{
    uuid_t data;
    uuid_generate(data);
    return theme::uuid(data);
}

// =================================================================================

void theme::uuid::to_le_bytes(uint8_t* buf) const
{
    uuid_le* guid = (uuid_le*)buf;
    uuid_le* src = (uuid_le*)m_data;
    guid->m_data1 = __builtin_bswap32(src->m_data1);
    guid->m_data2 = __builtin_bswap16(src->m_data2);
    guid->m_data3 = __builtin_bswap16(src->m_data3);
    memcpy(guid->m_data4, src->m_data4, sizeof(guid->m_data4));
}

ST::string theme::uuid::as_string() const
{
    char str[37];
    uuid_unparse(m_data, str);
    return ST::string::from_utf8(str, 36);
}

bool theme::uuid::from_string(const char* str)
{
    return uuid_parse((char*)str, m_data) == 0;
}

// =================================================================================

bool theme::uuid::empty() const
{
    return uuid_is_null(m_data) == 1;
}

bool theme::uuid::equals(const theme::uuid& rhs) const
{
    return uuid_compare(m_data, rhs.m_data) == 0;
}

bool theme::uuid::operator <(const theme::uuid& rhs) const
{
    return uuid_compare(m_data, rhs.m_data) == -1;
}
