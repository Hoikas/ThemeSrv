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

#include "poll.h"
#include "../core/errors.h"
#include "../core/log.h"

#include <map>
#include <sys/epoll.h>
#include <unistd.h>

// =================================================================================

static theme::log s_log{"EPOLL"};

// =================================================================================

namespace theme
{
    typedef std::map<int, struct epoll_cb> epoll_cb_map_t;

    struct epoll_cb
    {
        pollcb_t m_cb;
        epoll_cb_map_t::iterator m_iterator;

        epoll_cb() = delete;
        epoll_cb(const epoll_cb&) = delete;
        epoll_cb(epoll_cb&& move)
            : m_cb(std::move(move.m_cb)), m_iterator(std::move(move.m_iterator))
        { }
        epoll_cb(pollcb_t cb)
            : m_cb(std::move(cb))
        { }
    };

    class epoll_dispatch : public poll_dispatch
    {
        int m_fd;
        epoll_event m_events[1024]; // problem? [trollface.jpg]
        epoll_cb_map_t m_callbacks;

        inline uint32_t xlate_mask_to_epoll(events mask) const;
        inline events xlate_epoll_to_mask(uint32_t events) const;

    public:
        epoll_dispatch();
        ~epoll_dispatch();

        bool add_fd(int fd, events evmask, pollcb_t cb) override;
        bool remove_fd(int fd) override;

        bool dispatch(int timeout=30000) override;
    };
};

// =================================================================================

uint32_t theme::epoll_dispatch::xlate_mask_to_epoll(events mask) const
{
    uint32_t epoll_events = EPOLLET;
    if (mask & events::e_read)
        epoll_events |= EPOLLIN;
    if (mask & events::e_write)
        epoll_events |= EPOLLOUT;
    if (mask & events::e_hup)
        epoll_events |= EPOLLRDHUP | EPOLLHUP;
    return epoll_events;
}

theme::poll_dispatch::events theme::epoll_dispatch::xlate_epoll_to_mask(uint32_t events) const
{
    uint32_t mask = 0;
    if (events & EPOLLIN)
        mask |= events::e_read;
    if (events & EPOLLOUT)
        mask |= events::e_write;
    if (events & EPOLLRDHUP || events & EPOLLHUP)
        mask |= events::e_hup;
    return (poll_dispatch::events)mask;
}

// =================================================================================

theme::epoll_dispatch::epoll_dispatch()
    : m_fd(-1)
{
    m_fd = epoll_create1(0);
    THEME_ASSERTR_V(m_fd != -1, "epoll_create1 failed {}", strerror(errno));
}

theme::epoll_dispatch::~epoll_dispatch()
{
    if (m_fd != -1)
        THEME_ASSERTD(close(m_fd) == 0);
}

bool theme::epoll_dispatch::add_fd(int fd, events evmask, pollcb_t cb)
{
    auto cbresult = m_callbacks.try_emplace(fd, cb);
    if (!cbresult.second) {
        s_log.warning("add_fd() tried to double-add fd {}", fd);
        return false;
    }

    // This adds the iterator to the cb struct to the struct itself. This is useful for constant
    // time deletions. And yes, another copy. Get over it.
    cbresult.first->second.m_iterator = cbresult.first;

    epoll_event event{};
    event.events = xlate_mask_to_epoll(evmask);
    event.data.ptr = &cbresult.first->second;

    if (epoll_ctl(m_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        s_log.warning("add_fd() failed to add fd {}: {}", fd, strerror(errno));
        m_callbacks.erase(cbresult.first);
        return false;
    }
    return true;
}

bool theme::epoll_dispatch::remove_fd(int fd)
{
    if (epoll_ctl(m_fd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        s_log.warning("remove_fd() failed to remove fd {}: {}", fd, strerror(errno));
        return false;
    }

    m_callbacks.erase(fd);
    return true;
}

// =================================================================================

bool theme::epoll_dispatch::dispatch(int timeout)
{
    int result = epoll_wait(m_fd, m_events, std::size(m_events), timeout);
    log::tick();

    if (result == -1 && errno == EINTR) {
        return false;
    } else if (result == -1) {
        THEME_ASSERTR_V(false, "epoll_wait() failed {}: {}", m_fd, strerror(errno));
        return false;
    }

    for (int i = 0; i < result; ++i) {
        const auto& event = m_events[i];
        auto events_mask = xlate_epoll_to_mask(event.events);
        auto cb = (epoll_cb*)event.data.ptr;

        // If the socket hung up on us, that's an implicit removal from the epoll because we
        // give no more shits about it or its data (how dare you hang up on me?!?!?!)
        // Anyway, go ahead and do this BEFORE the callback so the callback doesn't do something
        // stupid like lookup the callback in the map.
        if (events_mask & events::e_hup) {
            if (epoll_ctl(m_fd, EPOLL_CTL_DEL, cb->m_iterator->first, nullptr) == -1) {
                s_log.warning("dispatch() failed to remove HUP'd fd {}: {}", cb->m_iterator->first,
                              strerror(errno));
            }
        }

        if (cb->m_cb)
            cb->m_cb(cb->m_iterator->first, events_mask);

        // Performance optimization: we already have the iterator, so we can constant-time
        // delete from the cb map here. Beware this will trigger the deletion of the callback
        // object, so don't use it anymore.
        if ((events_mask & events::e_hup))
            m_callbacks.erase(cb->m_iterator);
    }

    return true;
}

// =================================================================================

std::unique_ptr<theme::poll_dispatch> theme::poll_dispatch::create()
{
    return std::make_unique<epoll_dispatch>();
}
