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
    bool read(theme::client& cli, theme::socket& sock, std::unique_ptr<uint8_t[]>& buf) override
    {
        auto header = (theme::protocol::common_connection_header*)buf.get();
        switch (header->get_connType()) {
        case theme::protocol::e_protocolCli2Gate:
            cli.set_handler(theme::client_handler::create_gate(cli));
            break;
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

theme::client::client(theme::socket& socket, theme::server* server)
    : client_base(socket), m_server(server), m_flags(), m_handler(&s_incomingHandler)
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
    while (auto buf = handle_read()) {
        // Great! If we're here, this is a completed net message -- post it up to the high level
        // handler. That handler is responsible for registering the next struct for us to read.
        // A lack of a new struct is potentially an error...
        if (!m_handler->read(*this, m_socket, buf)) {
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
    do {
        if (!handle_write())
            break;
    } while (true);
}

// =================================================================================

theme::encrypted_handler::encrypted_handler(theme::client& cli)
    : m_encryptMsg({"m_encryptMsg", 1, &m_encryptBuf}),
      m_encryptBuf({net_field::data_type::e_blob, "buffer", 1, 0})
{
    // We don't actually want to hold the client, just setup the read state.
    cli.read<protocol::common_encrypt_header>();
}

// =================================================================================

bool theme::encrypted_handler::handle_handshake(theme::client& cli, theme::socket& sock, uint8_t* buf)
{
    auto header = (protocol::common_encrypt_header*)buf;
    if (header->get_msgId() != e_c2s_connect) {
        cli.logger().error("{}: unexpected encryption packet {}", sock.to_string(),
                           header->get_msgId());
        return false;
    }
    if (header->get_bufsz() != 2 && header->get_bufsz() != 66) {
        cli.logger().error("{}: bad encryption handshake size {}, expected 2 or 66",
                           sock.to_string(), header->get_bufsz());
        return false;
    }

    // time to prepare our "fake" net field.
    if (header->get_bufsz() == 2) {
#ifdef THEME_ALLOW_DECRYPTED_CONNECTIONS
        cli.logger().debug("{}: using decrypted communications... potential security smell",
                           sock.to_string());
        cli.flags() |= client::e_encrypted; // lies!
        return true;
#else
        cli.logger().error("{}: sent an empty encryption handshake, which is verboten",
                           sock.to_string());
        return false;
#endif
    } else {
        m_encryptBuf.m_count = header->get_bufsz() - 2;
        cli.read(&m_encryptMsg);
        cli.flags() |= client::e_wantClientSeed;
        return true;
    }
}

bool theme::encrypted_handler::handle_ydata(theme::client& cli, theme::socket& sock, uint8_t* buf)
{
    // This is a little over-engineered for the current application, but we may (in the future) want
    // to proxy other connections aside from just GateKeeperSrv and FileSrv, so it's best to keep
    // things fairly general.
    auto keys = get_keys(cli);

    protocol::common_encrypt_s2c reply;
    reply.set_msgId(e_s2c_encrypt);
    reply.set_bufsz(sizeof(reply));

    uint8_t key[sizeof(reply.m_srvSeed)];
    const auto& crypto = cli.server()->crypto();
    if (!crypto.make_server_key(std::get<0>(keys), std::get<1>(keys), m_encryptBuf.m_count,
                                buf, sizeof(key), reply.m_srvSeed, key)) {
        cli.logger().error("{}: failed to establish encryption", sock.to_string());
        return false;
    }

    // Write the reply back to the client in the clear, so don't apply the evp yet.
    cli.write(&reply);
    cli.set_crypt_key(sizeof(key), key);
    cli.flags() |= client::e_encrypted;
    return true;
}

bool theme::encrypted_handler::read(theme::client& cli, theme::socket& sock,
                                    std::unique_ptr<uint8_t[]>& buf)
{
    THEME_ASSERTD(!(cli.flags() & client::e_encrypted));

    // The handshake is the initial client->server packet with the contents:
    // uint8_t message ID
    // uint8_t message size
    // once we have the handshake, the following should be in the same packet:
    // uint8_t ydata[message size-2]
    // however, diafero's "decrypted" hack will send message size == 2. so, we need to
    // do this in two steps to detect that.
    if (!(cli.flags() & client::e_wantClientSeed)) {
        return handle_handshake(cli, sock, buf.get());
    } else {
        cli.flags() &= ~client::e_wantClientSeed;
        return handle_ydata(cli, sock, buf.get());
    }
}
