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

#include "uru_crypt.h"
#include "../core/errors.h"

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <memory>
#include <string_theory/st_codecs.h>

// ============================================================================

constexpr size_t kKeySize = 64;

// ============================================================================

theme::crypto_ctx::crypto_ctx(BN_CTX* ctx)
    : m_ctx(ctx)
{
    BN_CTX_start(ctx);
}

theme::crypto_ctx::~crypto_ctx()
{
    if (m_ctx)
        BN_CTX_end(m_ctx);
}

inline BIGNUM* theme::crypto_ctx::bignum() const
{
    return BN_CTX_get(m_ctx);
}

// ============================================================================

theme::crypto::crypto()
    : m_ctx(BN_CTX_new())
{
    static bool init = false;
    THEME_ASSERTR(!init);

    THEME_ASSERTR(RAND_poll());
    OpenSSL_add_all_algorithms();
    init = true;
}

theme::crypto::~crypto()
{
    BN_CTX_free(m_ctx);
    EVP_cleanup();
}

// ============================================================================

std::tuple<ST::string, ST::string, ST::string> theme::crypto::generate_keys(uint32_t g_value) const
{
    uint8_t k_key[kKeySize];
    uint8_t n_key[kKeySize];
    uint8_t x_key[kKeySize];

    {
        auto ctx = begin_calculation();
        auto g = ctx.bignum();
        auto k = ctx.bignum();
        auto n = ctx.bignum();
        auto x = ctx.bignum();

        // Generate primes for public (N) and private (K/A) keys
        constexpr size_t key_bits = kKeySize * 8;
        BN_generate_prime_ex(k, key_bits, 1, nullptr, nullptr, nullptr);
        BN_generate_prime_ex(n, key_bits, 1, nullptr, nullptr, nullptr);

        // Compute the client key (N/KA)
        // X = g**K%N
        BN_set_word(g, g_value);
        BN_mod_exp(x, g, k, n, m_ctx);

        // Store the keys
        BN_bn2bin(k, k_key);
        BN_bn2bin(n, n_key);
        BN_bn2bin(x, x_key);
    }

    return std::make_tuple(ST::base64_encode(k_key, sizeof(k_key)),
                           ST::base64_encode(n_key, sizeof(n_key)),
                           ST::base64_encode(x_key, sizeof(x_key)));
}
