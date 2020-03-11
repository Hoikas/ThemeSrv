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

#include <gflags/gflags.h>
#include <iostream>
#include <memory>

#include "../core/build_info.h"
#include "server.h"

// =================================================================================

DEFINE_string(config_path, "theme.ini", "Path to the ThemeSrv configuation file");
DEFINE_string(generate_client_ini, "", "Generates a server.ini file for plClient");
DEFINE_bool(generate_keys, false, "Generate a new set of encryption keys");
DEFINE_bool(save_config, false, "Saves the server configuration file");

// =================================================================================

// Prevents static initialization order issues.
static std::unique_ptr<theme::server> s_server;

// =================================================================================

int main(int argc, char* argv[])
{
    theme::build_info(std::cout);
    gflags::SetVersionString(theme::build_version());
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    s_server = std::make_unique<theme::server>(FLAGS_config_path);
    if (FLAGS_generate_keys)
        s_server->generate_daemon_keys();
    if (!FLAGS_generate_client_ini.empty())
        s_server->generate_client_ini(FLAGS_generate_client_ini);
    if (FLAGS_save_config)
        s_server->config().write(FLAGS_config_path);
    return s_server->run() ? 0 : 1;
}
