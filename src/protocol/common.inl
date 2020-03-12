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

THEME_NET_STRUCT_BEGIN_COMMON(common, connection_header)
    THEME_NET_FIELD_UINT8(connType)
    THEME_NET_FIELD_UINT16(msgsz)
    THEME_NET_FIELD_UINT32(buildId)
    THEME_NET_FIELD_UINT32(buildType)
    THEME_NET_FIELD_UINT32(branchId)
    THEME_NET_FIELD_UUID(product)
    THEME_NET_FIELD_BUFFER_REDUNDANT(buf)
THEME_NET_STRUCT_END(common, connection_header)

THEME_NET_STRUCT_BEGIN_COMMON(common, msg_std_header)
    THEME_NET_FIELD_UINT16(type)
THEME_NET_STRUCT_END(common, msg_std_header)

THEME_NET_STRUCT_BEGIN_COMMON(common, msg_size_header)
    THEME_NET_FIELD_UINT16(type)
    THEME_NET_FIELD_UINT16(size)
THEME_NET_STRUCT_END(common, msg_size_header)
