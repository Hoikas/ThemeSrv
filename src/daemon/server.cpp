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

#include "server.h"

#include <fstream>
#include <iostream>
#include <string_theory/iostream>

 // =================================================================================

theme::config_item s_daemonConfig[] = {
    THEME_CONFIG_STR("lobby", "bindaddr", "127.0.0.1",
                     "Lobby Bind Address\n"
                     "IP Address that this THEME server should listen for connections on")

    THEME_CONFIG_STR("lobby", "extaddr", "",
                     "Lobby External Address\n"
                     "IP Address that this THEME server can be reached on if behind NAT")

    THEME_CONFIG_INT("lobby", "port", 14617,
                     "Lobby Bind Port\n"
                     "Port that this THEME server should listen for connections on")

    THEME_CONFIG_STR("gate", "crypt_k", "", "Private Key")
    THEME_CONFIG_STR("gate", "crypt_n", "", "Public Key")
    THEME_CONFIG_STR("gate", "crypt_x", "", "Shared Key")
    THEME_CONFIG_INT("gate", "crypt_g", 4, "Base Value")

    THEME_CONFIG_STR("gate", "fileaddr", "", "File Server Address\n"
                     "Return this address to clients asking for a file server")
    THEME_CONFIG_STR("gate", "authaddr", "184.73.198.22", "Auth Server Address\n"
                     "Return this address to clients asking for an auth server")

    THEME_CONFIG_STR("file", "path", "", "File Server Path\n"
                     "Path to use for DirtSand-style manifest and file downloads")
    THEME_CONFIG_INT("file", "maxfds", 1024, "Maximum File Descriptors\n"
                     "This is the maximum number of REGULAR file descriptors that THEME will "
                     "hold open to avoid blocking system calls.")
};

// =================================================================================

theme::server::server(const std::filesystem::path& config)
    : m_config(s_daemonConfig)
{
    m_config.read(config);
}

// =================================================================================

void theme::server::generate_client_ini(const std::filesystem::path& path) const
{
    std::ofstream stream;
    stream.open(path, std::ios_base::out | std::ios_base::trunc);

    stream << "Server.Gate.G " << m_config.get<unsigned int>("gate", "crypt_g") << std::endl;
    stream << "Server.Gate.N " << m_config.get<const ST::string&>("gate", "crypt_n") << std::endl;
    stream << "Server.Gate.X " << m_config.get<const ST::string&>("gate", "crypt_x") << std::endl;

    const ST::string& bindaddr = m_config.get<const ST::string&>("lobby", "bindaddr");
    const ST::string& extaddr = m_config.get<const ST::string&>("lobby", "extaddr");
    stream << "Server.Gate.Host " << (extaddr.empty() ? bindaddr : extaddr) << std::endl;
    stream << "Server.Port " << m_config.get<unsigned int>("lobby", "port") << std::endl;
}

void theme::server::generate_daemon_keys()
{
    std::cout << "Generating keys..." << std::endl;

    // Only need gate keys ^_^
    auto g_value = m_config.get<unsigned int>("gate", "crypt_g");
    auto gate_keys = m_crypt.generate_keys(g_value);
    m_config.set<const ST::string&>("gate", "crypt_k", std::get<0>(gate_keys));
    m_config.set<const ST::string&>("gate", "crypt_n", std::get<1>(gate_keys));
    m_config.set<const ST::string&>("gate", "crypt_x", std::get<2>(gate_keys));
}

// =================================================================================

void theme::server::run()
{
    // TODO
}
