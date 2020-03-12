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

#ifndef __IO_SOCKET_H
#define __IO_SOCKET_H

#include <string_theory/string>

struct sockaddr;

namespace theme
{
    class socket
    {
        int m_fd;
        ST::string m_addr;

    public:
        socket()
            : m_fd(-1)
        { }

        socket(int fd)
        {
            setfd(fd);
        }

        socket(const socket&) = delete;
        socket(socket&& move)
        {
            m_fd = move.m_fd;
            move.m_fd = -1;
            m_addr = std::move(move.m_addr);
        }

        ~socket();

    public:
        bool bind(const char* addr, uint16_t port);
        bool listen(int backlog=10);
        bool accept(socket& client);
        bool shutdown();
        bool read(size_t bufsz, void* buf, ssize_t& nread);

        void setfd(int fd);
        void setfd(int fd, sockaddr* addr);

    public:
        const ST::string& to_string() const { return m_addr; }

        operator int() const { return m_fd; }
        operator const ST::string&() const { return m_addr; }
    };
};

#endif
