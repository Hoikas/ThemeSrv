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

#include "../core/endian.h"
#include "../core/errors.h"
#include "../core/uuid.h"
#include "net_struct.h"

#include <iostream>

// =================================================================================

size_t theme::net_struct_calcsz(const net_struct* msg, size_t idx)
{
    size_t size = 0;
    for (size_t i = 0; i < msg->m_size && i < idx; ++i)
        size += msg->m_fields[i].m_elementsz * msg->m_fields[i].m_count;
    return size;
}

static const char* _get_data_type_str(theme::net_field::data_type type)
{
    switch (type) {
    case theme::net_field::data_type::e_blob:
        return "BLOB";
    case theme::net_field::data_type::e_buffer:
    case theme::net_field::data_type::e_buffer_redundant:
        return "BUFFER";
    case theme::net_field::data_type::e_integer:
    case theme::net_field::data_type::e_buffer_size:
        return "INTEGER";
    case theme::net_field::data_type::e_string_utf16:
        return "STRING";
    case theme::net_field::data_type::e_uuid:
        return "UUID";
    default:
        return "???";
    }
}

void theme::net_struct_print(const net_struct* msg, std::ostream& stream)
{
    stream << "--- BEGIN NETSTRUCT ---" << std::endl;
    stream << "    Name: '" << msg->m_name << "'" << std::endl;
    stream << "    Field(s): " << msg->m_size << std::endl;
    for (size_t i = 0; i < msg->m_size; ++i) {
        const char* type = _get_data_type_str(msg->m_fields[i].m_type);
        stream << "    -> " << msg->m_fields[i].m_name << " ";
        stream << "[TYPE: '" << _get_data_type_str(msg->m_fields[i].m_type) << "'] ";
        stream << std::endl;
    }
    stream << "--- END   NETSTRUCT ---" << std::endl;
}

void theme::net_msg_print(const net_struct* msg, const void* data, std::ostream& stream)
{
    stream << "--- BEGIN NETMSG ---" << std::endl;
    stream << "    Name: '" << msg->m_name << "'" << std::endl;
    stream << "    Field(s): " << msg->m_size << std::endl;

    size_t offset = 0;
    for (size_t i = 0; i < msg->m_size; ++i) {
        stream << "    -> " << msg->m_fields[i].m_name << std::endl;
        stream << "        - TYPE: '" << _get_data_type_str(msg->m_fields[i].m_type) << "'" << std::endl;

        uint8_t* datap = (uint8_t*)data + offset;
        switch (msg->m_fields[i].m_type) {
        case net_field::data_type::e_integer:
        case net_field::data_type::e_buffer_size:
            {
                stream << "        - DATA: ";
                switch (msg->m_fields[i].m_elementsz) {
                case 1:
                    stream << (int)(*datap);
                    break;
                case 2:
                    stream << THEME_LE16(*(uint16_t*)datap);
                    break;
                case 4:
                    stream << THEME_LE32(*(uint32_t*)datap);
                    break;
                case 8:
                    stream << THEME_LE64(*(uint64_t*)datap);
                    break;
                default:
                    THEME_ASSERTD(0);
                    break;
                }
            }
            stream << std::endl;
            break;
        case net_field::data_type::e_string_utf16:
            stream << "        - DATA: '" << (char16_t*)datap << "'" << std::endl;
            break;
        case net_field::data_type::e_uuid:
            stream << "        - DATA: '" << ((theme::uuid*)datap)->as_string().c_str() << "'" << std::endl;
        }

        offset += msg->m_fields[i].m_elementsz * msg->m_fields[i].m_count;
    }

    stream << "--- END   NETMSG ---" << std::endl;
}
