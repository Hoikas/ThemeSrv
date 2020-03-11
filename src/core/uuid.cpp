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

theme::uuid theme::uuid::generate()
{
    uuid_t data;
    uuid_generate(data);
    return theme::uuid(data);
}

// =================================================================================

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
