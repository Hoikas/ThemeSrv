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

#ifndef __IO_POLL_H
#define __IO_POLL_H

#include <functional>
#include <memory>

namespace theme
{
    typedef std::function<void(int fd, uint32_t events)> pollcb_t;

    class poll_dispatch
    {
    public:
        enum events
        {
            e_read = (1<<0),
            e_write = (1<<1),
            e_hup = (1<<2),
        };

    protected:
        poll_dispatch() { }

    public:
        poll_dispatch(const poll_dispatch&) = delete;
        poll_dispatch(poll_dispatch&&) = delete;

        virtual ~poll_dispatch() = default;

        virtual bool add_fd(int fd, events evmask, pollcb_t cb) = 0;
        virtual bool remove_fd(int fd) = 0;

        /** Waits for incoming events and executes dispatch callbacks.
         *  Returns: true on success, false on timeout.
         */
        virtual bool dispatch(int timeout=30000) = 0;

    public:
        static std::unique_ptr<poll_dispatch> create();
    };
};

#endif
