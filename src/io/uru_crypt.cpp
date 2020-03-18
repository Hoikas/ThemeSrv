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
#include "../core/config_parser.h"
#include "../core/errors.h"

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string_theory/st_codecs.h>

using namespace ST::literals;

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
        THEME_ASSERTD(BN_generate_prime_ex(k, key_bits, 1, nullptr, nullptr, nullptr) == 1);
        THEME_ASSERTD(BN_generate_prime_ex(n, key_bits, 1, nullptr, nullptr, nullptr) == 1);

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

// ============================================================================

BIGNUM* theme::crypto::load_key(const ST::string& key) const
{
    uint8_t key_bytes[64];
    THEME_ASSERTD(ST::base64_decode(key, key_bytes, sizeof(key_bytes)) == sizeof(key_bytes));

    BIGNUM* ret = BN_new();
    BN_bin2bn(key_bytes, sizeof(key_bytes), ret);
    return ret;
}

// ============================================================================

std::tuple<bool, BIGNUM*, BIGNUM*> theme::crypto::load_keys(const theme::config_parser& config,
                                                            const ST::string& section) const
{
    bool result = true;
    BIGNUM* k;
    BIGNUM* n;

    ST::string kstr = config.get<const ST::string&>(section, "crypt_k"_st);
    ST::string nstr = config.get<const ST::string&>(section, "crypt_n"_st);
    if (kstr.empty() || nstr.empty()) {
        ST::string xstr;
        std::tie(kstr, nstr, xstr) = generate_keys(config.get<unsigned int>(section, "crypt_g"_st));
        result = false;
    }
    k = load_key(kstr);
    n = load_key(nstr);

    return std::make_tuple(result, k, n);
}

// ============================================================================

bool theme::crypto::make_server_key(BIGNUM* k, BIGNUM* n, size_t cli_seedsz,
                                    const uint8_t* const y_data, size_t keysz,
                                    uint8_t* srv_seed, uint8_t* key) const
{
    if (cli_seedsz < keysz)
        return false;

    uint8_t cli_seed[kKeySize];
    {
        auto ctx = begin_calculation();
        BIGNUM* y = ctx.bignum();
        BIGNUM* seed = ctx.bignum();

        BN_lebin2bn(y_data, cli_seedsz, y);
        BN_mod_exp(seed, y, k, n, ctx);
        BN_bn2lebinpad(seed, cli_seed, sizeof(cli_seed));
        RAND_bytes(srv_seed, keysz);
    }
    for (size_t i = 0; i < keysz; ++i)
        key[i] = cli_seed[i] ^ srv_seed[i];
    return true;
}
