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

#include <list>
#include <openssl/ossl_typ.h>
#include <memory>
#include <tuple>

typedef std::unique_ptr<EVP_CIPHER_CTX, void(*)(EVP_CIPHER_CTX*)> evp_ptr_t;

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
        class io_state
        {
        public:
            union
            {
                struct
                {
                    const net_struct* m_struct;
                    size_t m_field;
                    size_t m_offset;
                } m_read;
                struct
                {
                    size_t m_bufoffs;
                    size_t m_fdoffs;
                    int m_fd;
                } m_write;
                uint8_t m_unionbuf[std::max(sizeof(m_read), sizeof(m_write))];
            };
            size_t m_bufsz;
            std::unique_ptr<uint8_t[]> m_buf;

            io_state()
                : m_unionbuf(), m_bufsz()
            { }
            io_state(const io_state&) = delete;
            io_state(io_state&& move)
                : m_bufsz(move.m_bufsz), m_buf(std::move(move.m_buf))
            {
                memcpy(m_unionbuf, move.m_unionbuf, sizeof(m_unionbuf));
                memset(move.m_unionbuf, 0, sizeof(m_unionbuf));
                move.m_bufsz = 0;
            }
        };

        io_state m_read;
        std::list<io_state> m_writes;

        evp_ptr_t m_encrypt;
        evp_ptr_t m_decrypt;

    private:
        bool alloc_buf(io_state& state, size_t requestsz, bool exact=false);

        /**
        * Calculates the size of a net field.
        * Some fields are resized from their memory representation on the wire. This will allow
        * you to determine the size of a field in memory and on the wire. Note that determining
        * the wire size will require the examination of the message in memory.
        * \param buf Pointer to the field in the decrypted memory variant of the struct.
        * \return Returns a tuple of:
        *         - is the size fixed (false for buffers/strings/fds)
        *         - field size in memory (to allocate)
        *         - field size on wire (to read/write)
        */
        std::tuple<bool, size_t, size_t> calc_field_sz(const net_struct* const ns,
                                                       size_t field,
                                                       const uint8_t* const buf=nullptr) const;

        /**
         * Prints the contents of a field to the log.
         * \note buf is a memory style buffer, not wire-style.
         */
        void debug_field(const net_field& field, const uint8_t* const buf) const;

        /**
         * Extracts the number of elements in a wire buffer field.
         * \param buf Pointer to the buffer field in the memory representation of the message.
         */
        size_t extract_elementcount(const net_field& sz_field, const net_field& buf_field,
                                     const uint8_t* const buf) const;
        size_t extract_integer(const net_field& field, const uint8_t* const buf) const;

    private:
        bool resume_read(io_state& state);
        bool resume_write(io_state& state);

        void enqueue_write(const net_struct* const ns, const uint8_t* const buf);

    protected:
        /**
         * Handles incoming data on the socket.
         * \return Returns a smart pointer to a completely read message. If no complete message was
         *         read, the smart pointer is empty.
         */
        std::unique_ptr<uint8_t[]> handle_read();
        bool has_pending_read() const { return m_read.m_read.m_struct != nullptr; }

        bool handle_write();
        bool has_pending_write() const { return !m_writes.empty(); }

    protected:
        client_base(socket& sock);

    public:
        client_base(const client_base&) = delete;
        client_base(client_base&&) = delete;
        virtual ~client_base() = default;

    public:
        log& logger() { return s_log; }

        template<typename T>
        void read()
        {
            read(T::net_struct);
        }

        void read(const net_struct* const ns)
        {
            m_read.m_read.m_struct = ns;
        }

        template<typename T>
        void read(size_t field, std::unique_ptr<uint8_t[]>& buf)
        {
            read(T::net_struct, field, buf);
        }

        void read(const net_struct* const ns, size_t field, std::unique_ptr<uint8_t[]>& buf)
        {
            m_read.m_read.m_struct = ns;
            if (field != (size_t)-1)
                m_read.m_read.m_field = field;
            if (buf)
                m_read.m_buf = std::move(buf);
        }

        template<typename T>
        void write(const T* const msg)
        {
            write(T::net_struct, (const uint8_t* const)msg);
        }

        void write(const net_struct* const ns, const uint8_t* const buf)
        {
            enqueue_write(ns, buf);
        }

        void set_crypt_key(size_t keysz, const uint8_t* const key);
    };
};

#endif
