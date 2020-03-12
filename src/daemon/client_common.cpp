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

#include "client.h"

#include "../core/errors.h"
#include "../core/log.h"
#include "../io/poll.h"
#include "../protocol/common.h"
#include "server.h"

// =================================================================================

class incoming_client : public theme::client_handler
{
public:
    bool read(theme::client& cli, theme::socket& sock, const uint8_t* buf) override
    {
        const auto header = (const theme::protocol::common_connection_header*)buf;
        switch (header->get_connType()) {
        default:
            cli.logger().warning("{}: unhandled connection type {x}, discarding",
                                 sock.to_string(), header->get_connType());
            return false;
        }
        return true;
    }

    void hup(theme::client& cli, theme::socket& sock) override
    {
        cli.logger().debug("{}: HUP before handshake completion :'(", sock.to_string());
    }
} s_incomingHandler;

// =================================================================================

theme::client::client(theme::socket&& socket, theme::server* server)
    : client_base(std::move(socket)), m_server(server), m_flags(), m_handler(&s_incomingHandler)
{
    constexpr uint32_t events = poll_dispatch::e_read | poll_dispatch::e_write | poll_dispatch::e_hup;

    if (server->poll()->add_fd(m_socket, (poll_dispatch::events)events,
                               std::bind(&client::handle_dispatch, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2)))
        m_flags |= e_polling;

    // We're awaiting a connection packet...
    read<protocol::common_connection_header>();
}

theme::client::~client()
{
    if (m_flags & e_polling)
        m_server->poll()->remove_fd(m_socket);
}

// =================================================================================

void theme::client::handle_dispatch(int fd, uint32_t events)
{
    // Important: handle hang-ups first since the base class is in an undefined state
    // (read: nullptrs may be in mah state) -- it would be a shame if that crashed us.
    if (events & poll_dispatch::e_hup) {
        // handler is responsible for cleaning itself up
        m_handler->hup(*this, m_socket);
        m_flags &= ~e_polling;

        // triggers destruction of this instance.
        m_server->clients().erase(m_iterator);
        return;
    }

    if (events & poll_dispatch::e_write)
        pump_write();
    if (events & poll_dispatch::e_read)
        pump_read();
}

void theme::client::pump_read()
{
    while (handle_read()) {
        // Great! If we're here, this is a completed net message -- post it up to the high level
        // handler. That handler is responsible for registering the next struct for us to read.
        // A lack of a new struct is potentially an error...
        if (!m_handler->read(*this, m_socket, get_readbuf())) {
            s_log.debug("{}: handle_dispatch() read says it's time to shutdown.",
                        m_socket.to_string());
            m_socket.shutdown();
            return;
        } else if (!has_pending_read()) {
            s_log.error("{}: handle_dispatch() has no queued reads. Bug?",
                        m_socket.to_string());
            m_socket.shutdown();
            return;
        }
    }
}

void theme::client::pump_write()
{
    // TODO
}
