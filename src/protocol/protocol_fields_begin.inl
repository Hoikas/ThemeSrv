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
    namespace theme { namespace protocol { namespace _fields { \
        static const theme::net_field protocol_name##_##msg_name[] = {

#define THEME_NET_STRUCT_BEGIN(protocol_name, msg_name) THEME_NET_STRUCT_BEGIN_COMMON(protocol_name, msg_name)

#define THEME_NET_FIELD_BLOB(name, size) \
    { theme::net_field::data_type::e_blob, #name, 1, size },

#define THEME_NET_FIELD_BUFFER(name) \
    { theme::net_field::data_type::e_buffer_size, #name "sz", sizeof(uint32_t), 1 }, \
    { theme::net_field::data_type::e_buffer, #name, 1, 0 },

#define THEME_NET_FIELD_BUFFER_REDUNDANT(name) \
    { theme::net_field::data_type::e_buffer_size, #name "sz", sizeof(uint32_t), 1 }, \
    { theme::net_field::data_type::e_buffer_redundant, #name, 1, 0 },

#define THEME_NET_FIELD_UINT8(name) \
    { theme::net_field::data_type::e_integer, #name, sizeof(uint8_t), 1 },

#define THEME_NET_FIELD_UINT16(name) \
    { theme::net_field::data_type::e_integer, #name, sizeof(uint16_t), 1 },

#define THEME_NET_FIELD_UINT32(name) \
    { theme::net_field::data_type::e_integer, #name, sizeof(uint32_t), 1 },

#define THEME_NET_FIELD_STRING_UTF16(name, size) \
    { theme::net_field::data_type::e_buffer_size, #name "sz", sizeof(uint16_t), 1 }, \
    { theme::net_field::data_type::e_string_utf16, #name, sizeof(char16_t), size },

#define THEME_NET_FIELD_UUID(name) \
    { theme::net_field::data_type::e_uuid, #name, 16, 1 },

#define THEME_NET_STRUCT_END(protocol_name, msg_name) \
    }; \
    }; }; }; \
    \
    namespace theme { namespace protocol { namespace _net_structs { \
        const theme::net_struct protocol_name##_##msg_name =\
            { #protocol_name "_" #msg_name, std::size(theme::protocol::_fields::protocol_name##_##msg_name), \
              theme::protocol::_fields::protocol_name##_##msg_name }; \
    }; }; };
