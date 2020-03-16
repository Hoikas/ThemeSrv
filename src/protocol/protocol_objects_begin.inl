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

#define THEME_NET_STRUCT_BEGIN_COMMON(protocol_name, msg_name) \
    const theme::net_struct* theme::protocol::protocol_name##_##msg_name::net_struct = &theme::protocol::_net_structs::protocol_name##_##msg_name;
#define THEME_NET_STRUCT_BEGIN(protocol_name, msg_name) THEME_NET_STRUCT_BEGIN_COMMON(protocol_name, msg_name)

// noops
#define THEME_NET_FIELD_BLOB(name, size) ;
#define THEME_NET_FIELD_BUFFER(name) ;
#define THEME_NET_FIELD_BUFFER_REDUNDANT(name) ;
#define THEME_NET_FIELD_UINT8(name) ;
#define THEME_NET_FIELD_UINT16(name) ;
#define THEME_NET_FIELD_UINT32(name) ;
#define THEME_NET_FIELD_STRING_UTF16(name, size) ;
#define THEME_NET_FIELD_UUID(name) ;
#define THEME_NET_STRUCT_END(protocol_name, msg_name) ;
