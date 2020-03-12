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

#ifndef __THEME_SERVER_H
#define __THEME_SERVER_H

#include <filesystem>
#include <list>
#include <memory>

#include "../core/config_parser.h"
#include "../core/log.h"
#include "../io/socket.h"
#include "../io/uru_crypt.h"

namespace theme
{
    class server
    {
        config_parser m_config;
        crypto m_crypt;
        log m_log;

        socket m_listenSock;
        std::unique_ptr<class poll_dispatch> m_poll;
        std::list<class client> m_clients;
        bool m_active;

    public:
        server() = delete;
        server(const server&) = delete;
        server(server&&) = delete;

        server(const std::filesystem::path& config);
        ~server();

    public:
        std::list<client>& clients() { return m_clients; }
        const std::list<client>& clients() const { return m_clients; }

        config_parser& config() { return m_config; }
        const config_parser& config() const { return m_config; }

        poll_dispatch* poll() { return m_poll.get(); }
        const poll_dispatch* poll() const { return m_poll.get(); }

    public:
        void generate_client_ini(const std::filesystem::path& path) const;
        void generate_daemon_keys();

    public:
        bool run();

    protected:
        void check_crypto();
        bool init_fds();

        void accept_cb(int fd, uint32_t events);
    };
};

#endif
