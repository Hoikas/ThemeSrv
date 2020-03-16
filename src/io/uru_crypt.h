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

#ifndef __THEME_URU_CRYPT_H
#define __THEME_URU_CRYPT_H

#include <memory>
#include <openssl/ossl_typ.h>
#include <string_theory/string>
#include <tuple>

namespace theme
{
    class crypto_ctx
    {
        BN_CTX* m_ctx;

        crypto_ctx(BN_CTX* ctx);

        friend class crypto;

    public:
        crypto_ctx() = delete;
        crypto_ctx(const crypto_ctx&) = delete;
        crypto_ctx(crypto_ctx&& move)
            : m_ctx(move.m_ctx)
        {
            move.m_ctx = nullptr;
        }

        ~crypto_ctx();

        BIGNUM* bignum() const;

        operator BN_CTX*() const { return m_ctx; }
    };

    class crypto
    {
        BN_CTX* m_ctx;

    public:
        crypto();
        ~crypto();

        crypto_ctx begin_calculation() const { return crypto_ctx(m_ctx); }

        std::tuple<ST::string, ST::string, ST::string> generate_keys(uint32_t g_value) const;
        bool make_server_key(BIGNUM* k, BIGNUM* n, size_t cli_seedsz,
                             const uint8_t* const y_data, size_t keysz,
                             uint8_t* srv_seed, uint8_t* key) const;


        BIGNUM* load_key(const ST::string& key) const;
        std::tuple<bool, BIGNUM*, BIGNUM*> load_keys(const class config_parser& config,
                                                     const ST::string& section) const;
    };
};

#endif
