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

#include "socket.h"
#include "../core/errors.h"
#include "../core/log.h"

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

// =================================================================================

typedef std::unique_ptr<addrinfo, void(*)(addrinfo*)> addrinfoptr_t;
union sockaddr_t {
    sockaddr m_addr;
    sockaddr_in m_addr4;
    sockaddr_in6 m_addr6;
    sockaddr_storage m_storage;
};

static theme::log s_log{"SOCKET"};
static const int SOCK_NO = 0;
static const int SOCK_YES = 1;

// =================================================================================

theme::socket::~socket()
{
    if (m_fd != -1)
        THEME_ASSERTD(close(m_fd) == 0);
}

// =================================================================================

bool theme::socket::bind(const char* addr, uint16_t port)
{
    s_log.info("bind() using host: {} port: {}", addr, port);

    addrinfo info{};
    // fixme: ipv6? (do we really care?)
    info.ai_family = AF_INET;
    info.ai_socktype = SOCK_STREAM;
    info.ai_flags = AI_PASSIVE;

    char portstr[64];
    snprintf(portstr, sizeof(portstr), "%i", port);
    portstr[sizeof(portstr)-1] = 0;

    addrinfoptr_t addrList{ nullptr, [](addrinfo* x) { if (x) freeaddrinfo(x); } };
    addrinfo* addrListPtr = addrList.get();
    if (getaddrinfo(addr, portstr, &info, &addrListPtr) < 0) {
        s_log.error("bind() getaddrinfo() failed {}", strerror(errno));
        return false;
    }

    for (addrinfo* it = addrListPtr; it != nullptr; it = addrListPtr->ai_next) {
        int fd = ::socket(it->ai_family, (it->ai_socktype | SOCK_NONBLOCK), it->ai_protocol);
        if (fd == -1)
            continue;

        if (::bind(fd, it->ai_addr, it->ai_addrlen) == 0) {
            if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &SOCK_YES, sizeof(SOCK_YES)) != 0)
                s_log.warning("bind() failed to set SO_REUSEADDR");
            s_log.debug("bind() success!");
            setfd(fd, it->ai_addr);
            return true;
        } else {
            THEME_ASSERTD(close(fd) == 0);
        }
    }

    s_log.error("socket::bind() did not bind socket");
    return false;
}

bool theme::socket::listen(int backlog)
{
    if (::listen(m_fd, backlog) == -1) {
        s_log.error("{}: listen() failed on fd {}: {}", m_addr, m_fd, strerror(errno));
        return false;
    }

    s_log.debug("{}: fd {} listening...", m_addr, m_fd);
    return true;
}

bool theme::socket::accept(theme::socket& client)
{
    sockaddr_t addr{};
    socklen_t len = sizeof(addr);
    int result = ::accept4(m_fd, (sockaddr*)&addr, &len, SOCK_NONBLOCK);

    if (result == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return false;
    } else if (result == -1) {
        s_log.warning("{}: accept() failed on fd {}: {}", m_addr, m_fd, strerror(errno));
        return false;
    }

    client.setfd(result, (sockaddr*)&addr);
    return true;
}

bool theme::socket::shutdown()
{
    if (::shutdown(m_fd, SHUT_RDWR) == -1) {
        s_log.warning("{}: shutdown() failed on fd {}: {}", m_addr, m_fd, strerror(errno));
        return false;
    }
    return true;
}

std::tuple<bool, size_t> theme::socket::read(size_t bufsz, uint8_t* const buf)
{
    ssize_t nread = ::read(m_fd, buf, bufsz);
    if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return std::make_tuple(false, 0);
    } else if (nread == -1) {
        s_log.warning("{}: read failed on fd {}: {}", m_addr, m_fd, strerror(errno));
        return std::make_tuple(false, -1);
    } else {
        return std::make_tuple(true, (size_t)nread);
    }
}

std::tuple<bool, size_t> theme::socket::write(size_t bufsz, const uint8_t* const buf)
{
    ssize_t nwrite = ::write(m_fd, buf, bufsz);
    if (nwrite == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return std::make_tuple(false, 0);
    } else if (nwrite == -1) {
        s_log.warning("{}: write failed on fd {}: {}", m_addr, m_fd, strerror(errno));
        return std::make_tuple(false, -1);
    } else {
        return std::make_tuple(true, (size_t)nwrite);
    }
}

// =================================================================================

void theme::socket::setfd(int fd)
{
    if (fd == -1) {
        m_fd = -1;
    } else {
        sockaddr_t addr;
        socklen_t len = sizeof(addr);
        if (getsockname(fd, (sockaddr*)&addr, &len) == -1) {
            s_log.warning("setfd() unable to getsockname on fd {}: {}", fd, strerror(errno));
            return;
        }

        setfd(fd, (sockaddr*)&addr);
    }
}

void theme::socket::setfd(int fd, sockaddr* addr)
{
    m_fd = fd;

    if (fd != -1) {
        // Uru protocols require Nagling disabled
        if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &SOCK_YES, sizeof(SOCK_YES)) == -1) {
            s_log.warning("setfd() unable to disable Nagling on fd {}: {}", fd, strerror(errno));
        }

        // Cache remote endpoint string
        void* addrin;
        uint16_t port;
        switch (addr->sa_family) {
        case AF_INET:
            addrin = (void*)&((sockaddr_in*)addr)->sin_addr;
            port = ntohs(((sockaddr_in*)addr)->sin_port);
            break;
        case AF_INET6:
            addrin = (void*)&((sockaddr_in6*)addr)->sin6_addr;
            port = ntohs(((sockaddr_in6*)addr)->sin6_port);
            break;
        default:
            s_log.warning("setfd() addrinfo of an unexpected address family {}, will not be available",
                          addr->sa_family);
            m_addr = ST_LITERAL("???");
            return;
        }

        char addrbuf[INET6_ADDRSTRLEN + 1];
        if (!inet_ntop(addr->sa_family, addrin, addrbuf, sizeof(addrbuf))) {
            s_log.warning("setfd() inet_ntop() failed - no address will be available for fd {}: {}",
                          fd, strerror(errno));
            return;
        }

        addrbuf[sizeof(addrbuf)-1] = 0;
        m_addr = ST::format("{}/{}", addrbuf, port);
    }
}
