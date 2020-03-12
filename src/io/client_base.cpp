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

#include "client_base.h"

#include "../core/errors.h"
#include "../core/log.h"
#include "../protocol/common.h"

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>

 // =================================================================================

 // Uncomment to print out extremely verbose protocol schtuff
 //#define CLIENT_PROTOCOL_DEBUG

theme::log theme::client_base::s_log{"CLIENT"};

// Die if the client tries to send more than this many elements in a buffer.
size_t constexpr kMaxBufCount = 1024;

// =================================================================================

theme::client_base::~client_base()
{
    if (m_read.m_buf)
        free(m_read.m_buf);
    if (m_write.m_buf)
        free(m_write.m_buf);
}

// =================================================================================

ssize_t theme::client_base::calc_offset(const net_struct* netstruct, size_t field,
                                        const uint8_t* buf, bool write) const
{
    size_t offset = 0;
    for (size_t i = 0; i < field;) {
        const net_field& cur = netstruct->m_fields[i];

        if (cur.m_type == net_field::data_type::e_buffer_size && buf) {
            // OK, this is the size field for a variable field immediately following this field
            // whew. so, we need to actually examine the working buffer to figure this out.
            // To make life more fun, strings do this too, so we need to fall back gracefully.
            THEME_ASSERTD(cur.m_count == 1);

            ssize_t elementcount;
            const net_field& buf_field = netstruct->m_fields[i+1];
            if (write && buf_field.m_count != -1) {
                elementcount += buf_field.m_count;
            } else {
                const uint8_t* ptr = buf + offset;
                elementcount = extract_elementcount(cur, buf_field, buf);
                if (elementcount == -1)
                    return -1;
            }

            offset += cur.m_elementsz;
            offset += elementcount * buf_field.m_elementsz;
            i += 2;
        } else {
            offset += cur.m_count * cur.m_elementsz;
            ++i;
        }
    }

    return offset;
}

ssize_t theme::client_base::extract_elementcount(const net_field& sz_field, const net_field& buf_field,
                                                 const uint8_t* buf) const
{
    ssize_t result = extract_integer(sz_field, buf);

    // Yikes - redundant buffers include the size of the size parameter in their size.
    // So, ugh, we have to compensate for that.
    if (buf_field.m_type == net_field::data_type::e_buffer_redundant) {
        THEME_ASSERTD((sz_field.m_elementsz % buf_field.m_elementsz) == 0);
        result -= sz_field.m_elementsz / buf_field.m_elementsz;
    }

    if (result > kMaxBufCount) {
        s_log.warning("{}: extract_elementcount yielded too many elements ({}), over the limit {}",
                      m_socket.to_string(), result, kMaxBufCount);
        return -1;
    }
    return result;
}

size_t theme::client_base::extract_integer(const theme::net_field& field, const uint8_t* buf) const
{
    switch (field.m_elementsz) {
    case 1:
        return *buf;
    case 2:
        return THEME_LE16(*(const uint16_t*)buf);
    case 4:
        return THEME_LE32(*(const uint32_t*)buf);
    case 8:
        return THEME_LE64(*(const uint64_t*)buf);
    default:
        THEME_ASSERTD_V(false, "field {} has an unexpected element size {}",
                        field.m_name, field.m_elementsz);
        return 0;
    }
}

bool theme::client_base::resize_buf(size_t& currentsz, size_t desiredsz, uint8_t*& buf)
{
    if (currentsz >= desiredsz)
        return true;

    size_t blocks = desiredsz / sizeof(size_t);
    size_t bufsz = (blocks + 4) * sizeof(size_t);

    uint8_t* result = (uint8_t*)realloc((void*)buf, bufsz);
    if (result) {
        buf = result;
        currentsz = bufsz;
        return true;
    } else {
        s_log.error("failed to resize buffer current: {x} desired: {x}", currentsz, desiredsz);
        return false;
    }
}

void theme::client_base::debug_field(const theme::net_field& field, const uint8_t* buf) const
{
    switch (field.m_type) {
    case net_field::data_type::e_blob:
    case net_field::data_type::e_buffer:
    case net_field::data_type::e_buffer_redundant:
        s_log.debug("{}: READ {} [BUFFER]", m_socket.to_string(), field.m_name);
        break;
    case net_field::data_type::e_integer:
    case net_field::data_type::e_buffer_size:
        s_log.debug("{}: READ {} [INT]: {}", m_socket.to_string(), field.m_name,
                    extract_integer(field, buf));
        break;
    case net_field::data_type::e_string_utf16:
        s_log.debug("{}: READ {} [STRING]: {}", m_socket.to_string(), field.m_name,
            (char16_t*)buf);
        break;
    case net_field::data_type::e_uuid:
        s_log.debug("{}: READ {} [UUID]: {}", m_socket.to_string(), field.m_name,
            ((uuid*)buf)->as_string());
        break;
    default:
        s_log.debug("{}: READ {} [UNKNOWN]", m_socket.to_string(), field.m_name);
        break;
    }
}

// =================================================================================

bool theme::client_base::handle_read()
{
#ifdef CLIENT_PROTOCOL_DEBUG
    s_log.debug("{}: BEGIN READ {}", m_socket.to_string(), m_read.m_struct->m_name);
#endif

    // Size up to where we've read--we'll continue resizing the buff as we go.
    size_t structsz = calc_offset(m_read.m_struct, m_read.m_field, m_read.m_buf);
    if (!resize_buf(m_read.m_bufsz, structsz, m_read.m_buf)) {
        s_log.error("{}: handle_read() nuking due to excessive memory at start in {}.{}",
                    m_socket.to_string(), m_read.m_struct->m_name,
                    m_read.m_struct->m_fields[m_read.m_field].m_name);
        m_socket.shutdown();
        return false;
    }

    while (m_read.m_field < m_read.m_struct->m_size) {
        const net_field& cur = m_read.m_struct->m_fields[m_read.m_field];
        ssize_t sz;

        // Handle variable sized fields
        switch (cur.m_type) {
        case net_field::data_type::e_buffer:
        case net_field::data_type::e_buffer_redundant:
        case net_field::data_type::e_string_utf16:
        {
            const net_field& size_field = m_read.m_struct->m_fields[m_read.m_field-1];
            THEME_ASSERTD(size_field.m_count == 1);
            ptrdiff_t diff = m_read.m_bufoffs - size_field.m_elementsz;
            THEME_ASSERTD(diff > 0);
            uint8_t* ptr = m_read.m_buf + diff;
            sz = extract_elementcount(size_field, cur, ptr) * cur.m_elementsz;
            break;
        }
        default:
            sz = cur.m_count * cur.m_elementsz;
            break;
        }
        sz -= m_read.m_defer;

        // An illegal buffer size or just something bad happened...
        if (sz < 0) {
            s_log.warning("{}: handle_read() bad read size {} for field {}.{} -- the client may be flooding us",
                            m_socket.to_string(), m_read.m_struct->m_name, cur.m_name);
            m_socket.shutdown();
            return false;
        }

        // Resize buff if needed
        structsz += sz;
        if (!resize_buf(m_read.m_bufsz, structsz, m_read.m_buf)) {
            s_log.error("{}: handle_read() nuking due to excessive memory in {}.{}",
                        m_socket.to_string(), m_read.m_struct->m_name, cur.m_name);
            m_socket.shutdown();
            return false;
        }

        ssize_t nread;
        uint8_t* buf = m_read.m_buf + m_read.m_bufoffs + m_read.m_defer;
        if (!m_socket.read(sz, buf, nread)) {
            if (nread == -1) {
                s_log.debug("{}: handle_read() remote went down during read?", m_socket.to_string());
                m_socket.shutdown();
                return false;
            } else {
                // we read something, but not the entire field, so we must defer.
                m_read.m_defer += nread;
                return false;
            }
        }

#ifdef CLIENT_PROTOCOL_DEBUG
        debug_field(cur, buf - m_read.m_defer);
#endif

        // Great, we successfully read this field.
        m_read.m_defer = 0;
        m_read.m_field++;
        m_read.m_bufoffs += sz;
    }

#ifdef CLIENT_PROTOCOL_DEBUG
    s_log.debug("{}: END READ {}", m_socket.to_string(), m_read.m_struct->m_name);
#endif

    // Reset for the next struct
    m_read.m_struct = nullptr;
    m_read.m_field = 0;
    m_read.m_bufoffs = 0;
    return true;
}

void theme::client_base::handle_write()
{
    // TODO
}

