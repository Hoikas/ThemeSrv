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

#include "../io/client_base.h"

#include <list>

namespace theme
{
    struct net_field;
    struct net_struct;
    class server;

    class client : public client_base
    {
    protected:
        enum flags
        {
            e_polling = (1<<0),
        };

        server* m_server;
        std::list<client>::iterator m_iterator;
        uint32_t m_flags;
        class client_handler* m_handler;

        // So the m_iterator can be lazy-inited
        friend class server;

    public:
        client(socket&& sock, class server* server);
        ~client();

    protected:
        void handle_dispatch(int fd, uint32_t events);
        void pump_read();
        void pump_write();
    };

    class client_handler
    {
    public:
        client_handler() = default;
        client_handler(const client_handler&) = delete;
        client_handler(client_handler&&) = delete;
        virtual ~client_handler() = default;

        virtual bool read(client& cli, socket& sock, const uint8_t* buf) = 0;
        virtual void hup(theme::client& cli, theme::socket& sock) = 0;

    public:
        static client_handler* create_file(theme::client& cli);
        static client_handler* create_gate(theme::client& cli);
    };
};

#endif
