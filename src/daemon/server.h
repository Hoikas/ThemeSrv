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

#include "../core/config_parser.h"
#include "../io/uru_crypt.h"

namespace theme
{
    class server
    {
        config_parser m_config;
        crypto m_crypt;

    public:
        server() = delete;
        server(const server&) = delete;
        server(server&&) = delete;

        server(const std::filesystem::path& config);

    public:
        config_parser& config() { return m_config; }
        const config_parser& config() const { return m_config; }

    public:
        void generate_client_ini(const std::filesystem::path& path) const;
        void generate_daemon_keys();

    public:
        void run();
    };
};

#endif
