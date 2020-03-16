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

bool theme::gatekeeper_daemon::copy_string(const std::u16string& src, size_t destsz, char16_t* dest) const
{
    if (src.size() <= destsz) {
        src.copy(dest, destsz);
        dest[destsz-1] = 0;
        return false;
    } else {
        src.copy(dest, src.size());
        dest[src.size()] = 0;
        return true;
    }
}

// =================================================================================

namespace theme
{
    class gatekeeper_server : public encrypted_handler, public client_handler
    {
    protected:
        std::tuple<BIGNUM*, BIGNUM*> get_keys(client& cli) override;

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

bool theme::gatekeeper_server::read(theme::client& cli, theme::socket& sock,
                                    std::unique_ptr<uint8_t[]>& buf)
{
    // The base classes take care of establishing encryption and reading complete messages
    // from the client off the wire ^_^
    if (!(cli.flags() & client::e_encrypted)) {
        // FIXME: temporarily receive the first message's header to verify encryption.
        // When the dispatcher is written, this should simply return the encrypted handler's result.
        if (encrypted_handler::read(cli, sock, buf)) {
            if (cli.flags() & client::e_encrypted)
                cli.read<protocol::common_msg_std_header>();
            return true;
        }
    }

    return false;
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
