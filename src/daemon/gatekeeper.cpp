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

#include "gatekeeper.h"

#include "client.h"
#include "server.h"

#include <openssl/bn.h>

#include "../core/log.h"
#include "../io/uru_crypt.h"
#include "../protocol/common.h"
#include "../protocol/gatekeeper.h"

using namespace ST::literals;

// =================================================================================

static theme::log s_log{"GATEKEEPER"};

// =================================================================================

theme::gatekeeper_daemon::gatekeeper_daemon(const server* parent)
    : m_parent(parent),
      m_authsrv(parent->config().get<std::u16string>("gate", "authaddr")),
      m_filesrv(parent->config().get<std::u16string>("gate", "fileaddr")),
      m_cryptK(), m_cryptN()
{
    bool result;
    std::tie(result, m_cryptK, m_cryptN) = parent->crypto().load_keys(parent->config(), "gate"_st);
    if (!result) {
        s_log.warning("gatekeeper encryption keys not configured properly--encrypted connections will fail");
    }
}

theme::gatekeeper_daemon::~gatekeeper_daemon()
{
    BN_free(m_cryptK);
    BN_free(m_cryptN);
}

// =================================================================================

namespace theme
{
    class gatekeeper_server : public encrypted_handler, public client_handler
    {
    protected:
        std::tuple<BIGNUM*, BIGNUM*> get_keys(client& cli) override;

        bool handle_ping(client& cli, socket& sock, protocol::gatekeeper_pingRequest* req);
        bool handle_fileSrvReq(client& cli, socket& sock, protocol::gatekeeper_fileSrvRequest* req);
        bool handle_authSrvReq(client& cli, socket& sock, protocol::gatekeeper_authSrvRequest* req);

        bool handle_encryption(client& cli, socket& sock) override;
        bool dispatch_msg(client& cli, socket& sock, std::unique_ptr<uint8_t[]>& buf);
        bool read_msg(client& cli, socket& sock, std::unique_ptr<uint8_t[]>& buf);

    public:
        gatekeeper_server(client& cli)
            : encrypted_handler(cli)
        { }

        bool read(client& cli, socket& sock, std::unique_ptr<uint8_t[]>& buf) override;
        void hup(client& cli, socket& sock) override;
    };
};

// =================================================================================

std::tuple<BIGNUM*, BIGNUM*> theme::gatekeeper_server::get_keys(theme::client& cli)
{
    return cli.server()->gatekeeper()->get_keys();
}

// =================================================================================

bool theme::gatekeeper_server::handle_ping(theme::client& cli, theme::socket& sock,
                                           theme::protocol::gatekeeper_pingRequest* req)
{
    // Response is simply a bitwise copy--no need for any additional processing.
    cli.write<protocol::gatekeeper_pingReply>((protocol::gatekeeper_pingReply*)req);
    return true;
}

bool theme::gatekeeper_server::handle_fileSrvReq(theme::client& cli, theme::socket& sock,
                                                 theme::protocol::gatekeeper_fileSrvRequest* req)
{
    protocol::gatekeeper_fileSrvReply reply;
    reply.set_type(reply.id());
    reply.set_transId(req->get_transId());
    reply.set_address(cli.server()->gatekeeper()->get_filesrv_address());
    cli.write(&reply);
    return true;
}

bool theme::gatekeeper_server::handle_authSrvReq(theme::client& cli, theme::socket& sock,
                                                 theme::protocol::gatekeeper_authSrvRequest* req)
{
    protocol::gatekeeper_fileSrvReply reply;
    reply.set_type(reply.id());
    reply.set_transId(req->get_transId());
    reply.set_address(cli.server()->gatekeeper()->get_authsrv_address());
    cli.write(&reply);
    return true;
}

// =================================================================================

bool theme::gatekeeper_server::handle_encryption(theme::client& cli, theme::socket& sock)
{
    // Begin reading encrypted messages from the client.
    cli.flags() |= client::e_wantMsgHeader;
    cli.read<protocol::common_msg_std_header>();
    return true;
}

bool theme::gatekeeper_server::dispatch_msg(theme::client& cli, theme::socket& sock,
                                            std::unique_ptr<uint8_t[]>& buf)
{
    auto header = (const protocol::common_msg_std_header*)buf.get();
    switch (header->get_type()) {
    case protocol::gatekeeper::e_pingRequest:
        return handle_ping(cli, sock, (protocol::gatekeeper_pingRequest*)buf.get());
    case protocol::gatekeeper::e_fileSrvRequest:
        return handle_fileSrvReq(cli, sock, (protocol::gatekeeper_fileSrvRequest*)buf.get());
    case protocol::gatekeeper::e_authSrvRequest:
        return handle_authSrvReq(cli, sock, (protocol::gatekeeper_authSrvRequest*)buf.get());
    default:
        cli.logger().warning("{}: dispatch_msg() no dispatcher for {x}", sock.to_string(),
                             header->get_type());
        return false;
    }
}

bool theme::gatekeeper_server::read_msg(theme::client& cli, theme::socket& sock,
                                        std::unique_ptr<uint8_t[]>& buf)
{
    const net_struct* ns;
    auto header = (const protocol::common_msg_std_header*)buf.get();

    switch (header->get_type()) {
    case protocol::gatekeeper::e_pingRequest:
        ns = protocol::gatekeeper_pingRequest::net_struct;
        break;
    case protocol::gatekeeper::e_fileSrvRequest:
        ns = protocol::gatekeeper_fileSrvRequest::net_struct;
        break;
    case protocol::gatekeeper::e_authSrvRequest:
        ns = protocol::gatekeeper_authSrvRequest::net_struct;
        break;
    default:
        cli.logger().warning("{}: read_msg() sent unknown message {x}", sock.to_string(),
                             header->get_type());
        return false;
    }

    // Keep the same buffer as before but advance beyond the type field
    cli.read(ns, 1, buf);
    return true;
}

bool theme::gatekeeper_server::read(theme::client& cli, theme::socket& sock,
                                    std::unique_ptr<uint8_t[]>& buf)
{
    // The base classes take care of establishing encryption and reading complete messages
    // from the client off the wire ^_^
    if (!(cli.flags() & client::e_encrypted))
        return encrypted_handler::read(cli, sock, buf);

    if (cli.flags() & client::e_wantMsgHeader) {
        cli.flags() &= ~client::e_wantMsgHeader;
        return read_msg(cli, sock, buf);
    } else {
        cli.flags() |= client::e_wantMsgHeader;
        // this is safe because the read is just a state change
        cli.read<protocol::common_msg_std_header>();
        return dispatch_msg(cli, sock, buf);
    }
}

void theme::gatekeeper_server::hup(theme::client& cli, theme::socket& sock)
{
    s_log.debug("{}: good-bye, cruel world!", sock.to_string());
    delete this;
}

// =================================================================================

theme::client_handler* theme::client_handler::create_gate(theme::client& cli)
{
    return new gatekeeper_server(cli);
}
