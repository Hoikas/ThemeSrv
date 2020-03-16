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

THEME_NET_STRUCT_BEGIN(gatekeeper, pingRequest)
    THEME_NET_FIELD_UINT16(type)
    THEME_NET_FIELD_UINT32(pingTime)
    THEME_NET_FIELD_UINT32(transId)
    THEME_NET_FIELD_BUFFER(payload)
THEME_NET_STRUCT_END(gatekeeper, pingRequest)

THEME_NET_STRUCT_BEGIN(gatekeeper, fileSrvRequest)
    THEME_NET_FIELD_UINT16(type)
    THEME_NET_FIELD_UINT32(transId)
    THEME_NET_FIELD_UINT8(isPatcher)
THEME_NET_STRUCT_END(gatekeeper, fileSrvRequest)

THEME_NET_STRUCT_BEGIN(gatekeeper, authSrvRequest)
    THEME_NET_FIELD_UINT16(type)
    THEME_NET_FIELD_UINT32(transId)
THEME_NET_STRUCT_END(gatekeeper, authSrvRequest)

// =================================================================================

THEME_NET_STRUCT_BEGIN(gatekeeper, pingReply)
    THEME_NET_FIELD_UINT16(type)
    THEME_NET_FIELD_UINT32(pingTime)
    THEME_NET_FIELD_UINT32(transId)
    THEME_NET_FIELD_BUFFER(payload)
THEME_NET_STRUCT_END(gatekeeper, pingReply)

THEME_NET_STRUCT_BEGIN(gatekeeper, fileSrvReply)
    THEME_NET_FIELD_UINT16(type)
    THEME_NET_FIELD_UINT32(transId)
    THEME_NET_FIELD_STRING_UTF16(address, 24)
THEME_NET_STRUCT_END(gatekeeper, fileSrvReply)

THEME_NET_STRUCT_BEGIN(gatekeeper, authSrvReply)
    THEME_NET_FIELD_UINT16(type)
    THEME_NET_FIELD_UINT32(transId)
    THEME_NET_FIELD_STRING_UTF16(address, 24)
THEME_NET_STRUCT_END(gatekeeper, authSrvReply)
