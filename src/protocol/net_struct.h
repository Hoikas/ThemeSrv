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

#ifndef __PROTOCOL_NET_STRUCT_H
#define __PROTOCOL_NET_STRUCT_H

#include <cstdint>
#include <iosfwd>

namespace theme
{
    struct net_field final
    {
        enum class data_type
        {
            /** Plain old int. */
            e_integer,

            /** UUID */
            e_uuid,

            /** A fixed size buffer. */
            e_blob,

            /** Like e_integer, but represents a buffer size. */
            e_buffer_size,

            /** A maximum sized utf-16 string buffer */
            e_string_utf16,

            /** An arbitrarily sized buffer */
            e_buffer,

            /** An arbitarily sized buffer with a redundant size */
            e_buffer_redundant,
        };

        data_type m_type;
        const char* m_name;
        size_t m_elementsz;
        size_t m_count;
    };

    struct net_struct final
    {
        const char* m_name;
        size_t m_size;
        const net_field* m_fields;
    };

    size_t net_struct_calcsz(const net_struct*, size_t idx=-1);
    void net_struct_print(const net_struct*, std::ostream&);
    void net_msg_print(const net_struct*, const void*, std::ostream&);
};

#endif
