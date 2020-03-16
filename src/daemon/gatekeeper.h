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

#ifndef __THEME_GATEKEEPER
#define __THEME_GATEKEEPER

#include "client.h"

#include <openssl/ossl_typ.h>
#include <string>
#include <tuple>

namespace theme
{
    class gatekeeper_daemon
    {
        const class server* m_parent;
        std::u16string m_authsrv;
        std::u16string m_filesrv;
        BIGNUM* m_cryptK;
        BIGNUM* m_cryptN;

        bool copy_string(const std::u16string& src, size_t destsz, char16_t* dest) const;

    public:
        gatekeeper_daemon() = delete;
        gatekeeper_daemon(const gatekeeper_daemon&) = delete;
        gatekeeper_daemon(gatekeeper_daemon&&) = delete;
        gatekeeper_daemon(const class server* parent);
        ~gatekeeper_daemon();

    public:
        template<size_t _Sz>
        bool get_authsrv_address(char16_t(&dest)[_Sz]) const
        {
            return copy_string(m_authsrv, _Sz, dest);
        }

        template<size_t _Sz>
        bool get_filesrv_address(char16_t(&dest)[_Sz]) const
        {
            return copy_string(m_filesrv, _Sz, dest);
        }

        std::tuple<BIGNUM*, BIGNUM*> get_keys() const
        {
            return std::make_tuple(m_cryptK, m_cryptN);
        };
    };
};

#endif
