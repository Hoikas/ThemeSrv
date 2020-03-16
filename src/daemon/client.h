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

#ifndef __THEME_CLIENT_H
#define __THEME_CLIENT_H

#include "../core/endian.h"
#include "../core/log.h" // :(
#include "../io/client_base.h"
#include "../protocol/net_struct.h" // :(

#include <list>
#include <openssl/ossl_typ.h>

namespace theme
{
    class client : public client_base
    {
    protected:
        class server* m_server;
        std::list<client>::iterator m_iterator;
        uint32_t m_flags;
        class client_handler* m_handler;

        // So the m_iterator can be lazy-inited
        friend class server;

    public:
        client(socket& sock, class server* server);
        ~client();

    protected:
        void handle_dispatch(int fd, uint32_t events);
        void pump_read();
        void pump_write();

    public:
        enum flags
        {
            /** The socket is being polled by the server's dispatcher loop. */
            e_polling = (1<<0),

            /** The connection is fully encrypted. */
            e_encrypted = (1<<1),

            /** Encryption is pending the receipt of the client's seed. */
            e_wantClientSeed = (1<<2),

            /** The client is peeking for a message type from the client. */
            e_peekMsgType = (1<<3),
        };

        uint32_t& flags() { return m_flags; }
        uint32_t flags() const { return m_flags; }

        class client_handler* handler() { return m_handler; }
        const class client_handler* handler() const { return m_handler; }
        void set_handler(class client_handler* handler) { m_handler = handler; }

        const class server* server() const { return m_server; }
    };

    class client_handler
    {
    public:
        client_handler() = default;
        client_handler(const client_handler&) = delete;
        client_handler(client_handler&&) = delete;
        virtual ~client_handler() = default;

        virtual bool read(client& cli, socket& sock, std::unique_ptr<uint8_t[]>& buf) = 0;
        virtual void hup(client& cli, socket& sock) = 0;

    public:
        static client_handler* create_file(client& cli);
        static client_handler* create_gate(client& cli);
    };

    class encrypted_handler
    {
        enum
        {
            e_c2s_connect,
            e_s2c_encrypt,
            e_s2c_error,
        };

        /** Even more crazy variable length buffer hackery. */
        net_struct m_encryptMsg;
        net_field m_encryptBuf;

    private:
        bool handle_handshake(client& cli, socket& sock, uint8_t* buf);
        bool handle_ydata(client& cli, socket& sock, uint8_t* buf);

    protected:
        encrypted_handler() = delete;
        encrypted_handler(client& cli);
        encrypted_handler(const encrypted_handler&) = delete;
        encrypted_handler(encrypted_handler&&) = delete;
        virtual ~encrypted_handler() = default;

        virtual std::tuple<BIGNUM*, BIGNUM*> get_keys(client& cli) = 0;

        bool read(client& cli, socket& sock, std::unique_ptr<uint8_t[]>& buf);
    };
};

#endif
