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

#pragma pack(push,1)

#define THEME_NET_STRUCT_BEGIN_COMMON(protocol_name, msg_name) \
    namespace theme { namespace protocol { \
        struct protocol_name##_##msg_name final { \
            static const theme::net_struct* net_struct; \

#define THEME_NET_STRUCT_BEGIN(protocol_name, msg_name) \
    THEME_NET_STRUCT_BEGIN_COMMON(protocol_name, msg_name) \
            static constexpr uint16_t id() { return ::theme::protocol::protocol_name::e_##msg_name; }

#define THEME_NET_FIELD_BLOB(name, size) \
    uint8_t m_##name[size]; \
    \
    uint32_t get_##name##sz() const { return size; } \
    const uint8_t* get_##name() const { return m_##name; } \
    uint8_t* get_##name() { return m_##name; }

#define THEME_NET_FIELD_BUFFER(name) \
    uint32_t m_##name##sz; \
    uint8_t m_##name[]; \
    \
    uint32_t get_##name##sz() const { return THEME_LE32(m_##name##sz); } \
    void set_##name##sz(uint32_t value) { m_##name##sz = THEME_LE32(value); } \
    const uint8_t* get_##name() const { return m_##name; } \
    uint8_t* get_##name() { return m_##name; }

#define THEME_NET_FIELD_BUFFER_REDUNDANT(name) \
    uint32_t m_##name##sz; \
    uint8_t m_##name[]; \
    \
    uint32_t get_##name##sz() const { return THEME_LE32(m_##name##sz - sizeof(uint32_t)); } \
    void set_##name##sz(uint32_t value) { m_##name##sz = THEME_LE32(value + sizeof(uint32_t)); } \
    const uint8_t* get_##name() const { return m_##name; } \
    uint8_t* get_##name() { return m_##name; }

#define THEME_NET_FIELD_UINT8(name) \
    uint8_t m_##name; \
    \
    uint8_t get_##name() const { return m_##name; } \
    void set_##name(uint8_t value) { m_##name = value; }

#define THEME_NET_FIELD_UINT16(name) \
    uint16_t m_##name; \
    \
    uint16_t get_##name() const { return THEME_LE16(m_##name); } \
    void set_##name(uint16_t value) { m_##name = THEME_LE16(value); }

#define THEME_NET_FIELD_UINT32(name) \
    uint32_t m_##name; \
    \
    uint32_t get_##name() const { return THEME_LE32(m_##name); } \
    void set_##name(uint32_t value) { m_##name = THEME_LE32(value); }

#define THEME_NET_FIELD_STRING_UTF16(name, szval) \
    uint16_t m_##name##sz; \
    char16_t m_##name[szval]; \
    \
    std::u16string_view get_##name() const { return std::u16string_view(m_##name, m_##name##sz); } \
    void set_##name(const std::u16string_view& value) \
    { \
        auto size = std::min((size_t)szval, value.size()); \
        value.copy(m_##name, size); \
        m_##name[std::min((size_t)szval-1, value.size())] = 0; \
        m_##name##sz = THEME_LE16(size); \
    }

#define THEME_NET_FIELD_UUID(name) \
    uint8_t m_##name[16]; \
    const theme::uuid* get_##name() const { return (const theme::uuid*)m_##name; } \
    theme::uuid* get_##name() { return (theme::uuid*)m_##name; }

#define THEME_NET_STRUCT_END(protocol_name, msg_name) \
    }; }; };
