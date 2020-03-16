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

#include "client_base.h"
#include "theme_config.h"

#include "../core/errors.h"
#include "../core/log.h"
#include "../protocol/common.h"

#include <openssl/evp.h>
#include <sys/types.h>

// =================================================================================

theme::log theme::client_base::s_log{"CLIENT"};

// Die if the client tries to send more than this many elements in a buffer.
constexpr size_t kMaxBufCount = 1024;

// Maximum buffer size we can allocate on the stack
constexpr size_t kMaxStackBufSize = 2048;

// =================================================================================

theme::client_base::client_base(theme::socket& sock)
    : m_socket(std::move(sock)),
      m_encrypt({ EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free }),
      m_decrypt({ EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free })
{
    EVP_EncryptInit_ex(m_encrypt.get(), EVP_enc_null(), nullptr, nullptr, nullptr);
    EVP_DecryptInit_ex(m_decrypt.get(), EVP_enc_null(), nullptr, nullptr, nullptr);
}

void theme::client_base::set_crypt_key(size_t keysz, const uint8_t* const key)
{
    s_log.debug("{}: changing encryption ({} bits)", m_socket.to_string(), keysz * 8);

    auto cipher = EVP_rc4();
    auto initproc = EVP_CIPHER_meth_get_init(cipher);

    EVP_CIPHER_CTX_reset(m_encrypt.get());
    THEME_ASSERTD(EVP_EncryptInit_ex(m_encrypt.get(), cipher, nullptr, nullptr, nullptr) == 1);
    EVP_CIPHER_CTX_set_key_length(m_encrypt.get(), keysz);
    THEME_ASSERTD(initproc(m_encrypt.get(), key, nullptr, 1) == 1);

    EVP_CIPHER_CTX_reset(m_decrypt.get());
    THEME_ASSERTD(EVP_DecryptInit_ex(m_decrypt.get(), cipher, nullptr, nullptr, nullptr) == 1);
    EVP_CIPHER_CTX_set_key_length(m_decrypt.get(), keysz);
    THEME_ASSERTD(initproc(m_decrypt.get(), key, nullptr, 0) == 1);
}

// =================================================================================

bool theme::client_base::alloc_buf(theme::client_base::io_state& state, size_t requestsz, bool exact)
{
    size_t bufsz;
    if (exact) {
        bufsz = requestsz;
    } else {
        size_t blocks = requestsz / sizeof(size_t);
        bufsz = (blocks + 4) * sizeof(size_t);
    }

    auto buf = std::make_unique<uint8_t[]>(bufsz);
    if (!buf) {
        s_log.error("{}: failed to resize buffer current: {x} desired: {x} final: {x}",
                    m_socket.to_string(), state.m_bufsz, requestsz, bufsz);
        return false;
    }

    if (state.m_buf)
        memcpy(buf.get(), state.m_buf.get(), state.m_bufsz);
    state.m_bufsz = bufsz;
    state.m_buf = std::move(buf);
    return true;
}

std::tuple<bool, size_t, size_t> theme::client_base::calc_field_sz(const theme::net_struct* const ns,
                                                                   size_t field,
                                                                   const uint8_t* const buf) const
{
    bool known = true;
    size_t alloc = 0;
    size_t wire = 0;

    THEME_ASSERTD(field < ns->m_size);
    const net_field& cur_field = ns->m_fields[field];

    switch (cur_field.m_type) {
    case net_field::data_type::e_buffer:
    case net_field::data_type::e_buffer_redundant:
        // These fields MUST be the last one in the struct.
        THEME_ASSERTD(field + 1 == ns->m_size);
        // fall-through
    case net_field::data_type::e_string_utf16:
        THEME_ASSERTD(field > 0);
        alloc += cur_field.m_elementsz * cur_field.m_count;
        if (buf) {
            const net_field& sz_field = ns->m_fields[field - 1];
            wire = cur_field.m_elementsz * extract_elementcount(sz_field, cur_field, buf);
        } else {
            // Don't increment the wire size -- we don't know the size of this field, so it's
            // better for the IO operation to complete and then for us to "double check" to
            // ensure we are actually done. In the best case scenario, we have an empty buffer
            // (eg pings) and the accurate size == the inaccurate size. Worst case, we have
            // to resume reading the message multiple times (embedded strings).
            known = false;
        }
        break;
    default:
        alloc = cur_field.m_elementsz * cur_field.m_count;
        wire = cur_field.m_elementsz * cur_field.m_count;
        break;
    }

    return std::make_tuple(known, alloc, wire);
}

size_t theme::client_base::extract_elementcount(const theme::net_field& sz_field,
                                                const theme::net_field& buf_field,
                                                const uint8_t* const buf) const
{
    // These assertions should never happen because the macros will build the protocol
    // structures as we expect.
    THEME_ASSERTD(sz_field.m_type == net_field::data_type::e_buffer_size);
    THEME_ASSERTD(sz_field.m_count == 1);

    const uint8_t* ptr = buf - sz_field.m_elementsz;
    size_t result = extract_integer(sz_field, ptr);

    // Yikes - redundant buffers include the size of the size parameter in their size.
    // So, ugh, we have to compensate for that.
    if (buf_field.m_type == net_field::data_type::e_buffer_redundant) {
        THEME_ASSERTD((sz_field.m_elementsz % buf_field.m_elementsz) == 0);
        result -= sz_field.m_elementsz / buf_field.m_elementsz;
    }

    if (result > kMaxBufCount) {
        s_log.warning("{}: extract_elementcount yielded too many elements ({}), over the limit {}",
                      m_socket.to_string(), result, kMaxBufCount);
        return -1;
    }
    return result;
}

size_t theme::client_base::extract_integer(const theme::net_field& field, const uint8_t* const buf) const
{
    switch (field.m_elementsz) {
    case 1:
        return *buf;
    case 2:
        return THEME_LE16(*(const uint16_t*)buf);
    case 4:
        return THEME_LE32(*(const uint32_t*)buf);
    case 8:
        return THEME_LE64(*(const uint64_t*)buf);
    default:
        THEME_ASSERTD_V(false, "field {} has an unexpected element size {}",
                        field.m_name, field.m_elementsz);
        return 0;
    }
}

void theme::client_base::debug_field(const theme::net_field& field, const uint8_t* const buf) const
{
    switch (field.m_type) {
    case net_field::data_type::e_blob:
    case net_field::data_type::e_buffer:
    case net_field::data_type::e_buffer_redundant:
        s_log.debug("{}: FIELD {} [BUFFER]", m_socket.to_string(), field.m_name);
        break;
    case net_field::data_type::e_integer:
    case net_field::data_type::e_buffer_size:
        s_log.debug("{}: FIELD {} [INT]: {}", m_socket.to_string(), field.m_name,
                    extract_integer(field, buf));
        break;
    case net_field::data_type::e_string_utf16:
        s_log.debug("{}: FIELD {} [STRING]: {}", m_socket.to_string(), field.m_name,
                   (char16_t*)buf);
        break;
    case net_field::data_type::e_uuid:
        s_log.debug("{}: FIELD {} [UUID]: {}", m_socket.to_string(), field.m_name,
                    ((uuid*)buf)->as_string());
        break;
    default:
        s_log.debug("{}: FIELD {} [UNKNOWN]", m_socket.to_string(), field.m_name);
        break;
    }
}

// =================================================================================

bool theme::client_base::resume_read(io_state& state)
{
    do {
        // How much have we read?
        size_t memsz = 0;
        size_t wiresz = 0;
        for (size_t i = 0; i < state.m_read.m_field; ++i) {
            auto result = calc_field_sz(state.m_read.m_struct, i, state.m_buf.get() + memsz);
            THEME_ASSERTD(std::get<0>(result));
            memsz += std::get<1>(result);
            wiresz += std::get<2>(result);
        }

        // How much more do we need to read?
        bool fixedsz = true;
        size_t allocsz = memsz;
        size_t readsz = 0;

        for (size_t i = state.m_read.m_field; i < state.m_read.m_struct->m_size; ++i) {
            // For the first field ONLY, it is safe to examine the read buffer. Past here, we have
            // undefined/uninitialized data. Yikes.
            const uint8_t* ptr = (i == state.m_read.m_field) ? state.m_buf.get() + memsz : nullptr;
            auto sizes = calc_field_sz(state.m_read.m_struct, i, ptr);
            if (std::get<2>(sizes) == (size_t)-1) {
                // too large of a request...
                m_socket.shutdown();
                return false;
            }
            fixedsz &= std::get<0>(sizes);
            allocsz += std::get<1>(sizes);

            // If we are into a variable read, we can't read that yet. But, we do want to try to
            // allocate close to the complete message, so keep counting.
            if (fixedsz)
                readsz += std::get<2>(sizes);
        }

        if (!alloc_buf(state, allocsz)) {
            m_socket.shutdown();
            return false;
        }

        readsz -= std::min(readsz, state.m_read.m_offset);
        if (readsz > 0) {
            // All this work just to get a buffer that might be on the stack... Sigh
            std::unique_ptr<uint8_t[]> heapbuf;
            uint8_t* buf;
            if (readsz <= kMaxStackBufSize) {
                buf = (uint8_t*)alloca(readsz);
            } else {
                heapbuf = std::make_unique<uint8_t[]>(readsz);
                buf = heapbuf.get();
            }

            auto result = m_socket.read(readsz, buf);
            if (!std::get<0>(result)) {
                if (std::get<1>(result) == (size_t)-1)
                    m_socket.shutdown();
                return false;
            }

            // OK, we read something. Now, we need to iterate through the fields we had left to
            // read and see how far along we got into that... Decrypting the data along the way,
            // of course. *grumble, grumble*
            size_t nread = std::get<1>(result);
            size_t mempos = memsz;
            size_t wirepos = 0;
            do {
                auto sizes = calc_field_sz(state.m_read.m_struct, state.m_read.m_field,
                                           state.m_buf.get() + mempos);
                THEME_ASSERTD(std::get<0>(sizes));
                size_t field_memsz = std::get<1>(sizes);
                size_t field_wiresz = std::get<2>(sizes);

                // Add offset NOW so that we don't affect the above field inspection...
                mempos += state.m_read.m_offset;

                // Decrypt smaller of: field size on the wire or read amount remaining
                int decsz;
                size_t field_nread = std::min(field_wiresz, nread);
                THEME_ASSERTD(EVP_DecryptUpdate(m_decrypt.get(),
                                                state.m_buf.get() + mempos, &decsz,
                                                buf + wirepos, field_nread) != 0);
                THEME_ASSERTD(decsz == field_nread);
#ifdef THEME_PROTOCOL_DEBUG
                debug_field(state.m_read.m_struct->m_fields[state.m_read.m_field],
                            state.m_buf.get() + mempos);
#endif

                nread -= field_nread;
                wirepos += field_nread;

                // Now, check to see if we left off in the middle of a field.
                if (field_nread < field_wiresz) {
                    state.m_read.m_offset += field_nread;
                    THEME_ASSERTD(nread == 0);
                    // break out of this processing loop and try to request more data. ideally we
                    // will get EAGAIN and return to processing other clients -- there might be more
                    // data available though, so we could keep going as well.
                    break;
                } else {
                    // reset the offset to zero since we read a complete field f'sho
                    state.m_read.m_field++;
                    // if we read all the fields, we are done woo
                    if (state.m_read.m_field == state.m_read.m_struct->m_size) {
                        THEME_ASSERTD(nread == 0);
                        return true;
                    }
                    state.m_read.m_offset = 0;
                    mempos += field_memsz;
                }
            } while (true);
        }
    } while(true);
}

std::unique_ptr<uint8_t[]> theme::client_base::handle_read()
{
    bool complete;
    if (!m_read.m_buf) {
#ifdef THEME_PROTOCOL_DEBUG
        s_log.debug("{}: BEGIN READ '{}'", m_socket.to_string(), m_read.m_read.m_struct->m_name);
#endif

        complete = resume_read(m_read);
    } else {
        complete = resume_read(m_read);
    }

    if (complete) {
#ifdef THEME_PROTOCOL_DEBUG
        s_log.debug("{}: END READ '{}'", m_socket.to_string(), m_read.m_read.m_struct->m_name);
#endif

        // Reset state
        m_read.m_read.m_field = 0;
        m_read.m_read.m_offset = 0;
        m_read.m_read.m_struct = nullptr;

        // Release the buffer to the higher level processor. Unless someone explicitly holds onto
        // it, the buffer will be destroyed after processing completes.
        return std::move(m_read.m_buf);
    }

    // prevents a warning
    return std::unique_ptr<uint8_t[]>(nullptr);
}

// =================================================================================

bool theme::client_base::resume_write(theme::client_base::io_state& state)
{
    // Due to encryption, this is a fairly "dumb" send
    const uint8_t* ptr = state.m_buf.get() + state.m_write.m_bufoffs;
    size_t sendsz = state.m_bufsz - state.m_write.m_bufoffs;

    // TODO: sendfile()
    auto result = m_socket.write(sendsz, ptr);
    if (!std::get<0>(result)) {
        // error
        if (std::get<1>(result) == (size_t)-1)
            m_socket.shutdown();
        return false;
    }

    // how much did we read?
    state.m_write.m_bufoffs += std::get<1>(result);
    return state.m_write.m_bufoffs == state.m_bufsz;
}

void theme::client_base::enqueue_write(const theme::net_struct* const ns, const uint8_t* const buf)
{
    io_state state;

    // Bad news old bean. While it would be nice to avoid copying, we need to send out an encrypted
    // message. That means while we're writing we no longer have access to the decrypted contents.
    // So, we'll instead perform userspace buffering here. At least the code is cleaner...
    size_t wiresz = 0;
    for (size_t i = 0; i < ns->m_size; ++i) {
        auto result = calc_field_sz(ns, i, buf + wiresz);
        THEME_ASSERTD(std::get<0>(result));
        wiresz += std::get<2>(result);
    }

    if (!alloc_buf(state, wiresz, true)) {
        m_socket.shutdown();
        return;
    }

#ifdef THEME_PROTOCOL_DEBUG
    s_log.debug("{}: ENQUEUE WRITE '{}' wiresz:{x}", m_socket.to_string(), ns->m_name,wiresz);
#endif

    const uint8_t* mem_ptr = buf;
    uint8_t* wire_ptr = state.m_buf.get();
    for (size_t i = 0; i < ns->m_size; ++i) {
        auto result = calc_field_sz(ns, i, mem_ptr);
        THEME_ASSERTD(std::get<0>(result));

        int encsz;
        EVP_EncryptUpdate(m_encrypt.get(), wire_ptr, &encsz, mem_ptr, std::get<2>(result));
        THEME_ASSERTD(encsz == std::get<2>(result));
#ifdef THEME_PROTOCOL_DEBUG
        debug_field(ns->m_fields[i], mem_ptr);
#endif

        mem_ptr += std::get<1>(result);
        wire_ptr += std::get<2>(result);
    }

#ifdef THEME_PROTOCOL_DEBUG
    s_log.debug("{}: END WRITE '{}'", m_socket.to_string(), ns->m_name);
#endif

    if (!has_pending_write()) {
        if (resume_write(state)) {
#ifdef THEME_PROTOCOL_DEBUG
            s_log.debug("{}: WROTE '{}' SYNCHRONOUSLY", m_socket.to_string(), ns->m_name);
#endif
            return;
        }
    }
    m_writes.emplace_back(std::move(state));
}

bool theme::client_base::handle_write()
{
    if (m_writes.empty())
        return false;

    io_state& state = m_writes.front();
    if (resume_write(state)) {
        m_writes.pop_front();
        return true;
    }
    return false;
}
