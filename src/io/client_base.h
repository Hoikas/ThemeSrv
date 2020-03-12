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

#ifndef __IO_CLIENT_BASE_H
#define __IO_CLIENT_BASE_H

#include "socket.h"

namespace theme
{
    class log;
    struct net_field;
    struct net_struct;

    class client_base
    {
    protected:
        socket m_socket;
        static log s_log;

    private:
        struct io_state
        {
            const net_struct* m_struct{};
            size_t m_field{};
            size_t m_defer{};
            size_t m_bufoffs{};
            size_t m_bufsz{};
            uint8_t* m_buf{};
        };

        io_state m_read;
        io_state m_write;

    private:
        ssize_t calc_offset(const net_struct* netstruct, size_t field, const uint8_t* buf, bool write=false) const;
        ssize_t extract_elementcount(const net_field& sz_field, const net_field& buf_field, const uint8_t* buf) const;
        size_t extract_integer(const net_field& field, const uint8_t* buf) const;
        bool resize_buf(size_t& currentsz, size_t desiredsz, uint8_t*& buf);
        void debug_field(const net_field& field, const uint8_t* buf) const;

    protected:
        /** Handles incoming data on the socket.
         *  Returns boolean whether a completed message is available for processing.
         */
        bool handle_read();
        const uint8_t* get_readbuf() const { return m_read.m_buf; }
        bool has_pending_read() const { return m_read.m_struct != nullptr; }

        void handle_write();
        const uint8_t* get_writebuf() const { return m_write.m_buf; }
        bool has_pending_write() const { return m_write.m_struct != nullptr; }

    protected:
        client_base(socket&& sock)
            : m_socket(std::move(sock))
        { }

    public:
        client_base(const client_base&) = delete;
        client_base(client_base&&) = delete;
        ~client_base();

    public:
        log& logger() { return s_log; }

        template<typename T>
        void read(size_t field=-1)
        {
            m_read.m_struct = T::net_struct;
            if (field != (size_t)-1)
                m_read.m_field = field;
        }
    };
};

#endif
