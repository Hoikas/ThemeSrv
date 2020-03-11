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

#ifndef _THEME_CONFIG_PARSER_H
#define _THEME_CONFIG_PARSER_H

#include <filesystem>
#include <iosfwd>
#include <string_theory/string>
#include <unordered_map>

#define THEME_CONFIG_BOOL(section, key, value, desc) \
    THEME_CONFIG(section, key, theme::config_item::value_type::e_boolean, #value, desc)

#define THEME_CONFIG_FLOAT(section, key, value, desc) \
    THEME_CONFIG(section, key, theme::config_item::value_type::e_float, #value, desc)

#define THEME_CONFIG_INT(section, key, value, desc) \
    THEME_CONFIG(section, key, theme::config_item::value_type::e_integer, #value, desc)

#define THEME_CONFIG_STR(section, key, value, desc) \
    { ST_LITERAL(section), ST_LITERAL(key), theme::config_item::value_type::e_string, \
      ST_LITERAL(value), ST_LITERAL(desc) },

#define THEME_CONFIG(section, key, type, value, desc) \
    { ST_LITERAL(section), ST_LITERAL(key), type, ST_LITERAL(value), ST_LITERAL(desc) },

namespace theme
{
    class config_item
    {
    public:
        enum class value_type
        {
            e_boolean,
            e_float,
            e_integer,
            e_string,
        };

        const ST::string m_section;
        const ST::string m_key;
        const value_type m_type;
        const ST::string m_value;
        const ST::string m_description;
    };

    class config_parser
    {
        class _config_item
        {
        public:
            const theme::config_item* m_def;
            ST::string m_value;
        };

        typedef std::unordered_map<ST::string, _config_item,
                                   ST::hash_i, ST::equal_i> sectionmap_t;
        typedef std::unordered_map<ST::string, sectionmap_t,
                                   ST::hash_i, ST::equal_i> configmap_t;
        configmap_t m_config;

        sectionmap_t::iterator find_item(const ST::string& section, const ST::string& key);
        sectionmap_t::const_iterator find_item(const ST::string& section, const ST::string& key) const;
        void set(size_t lineno, const configmap_t::iterator& section, const ST::string& key, const ST::string& value);

    public:
        config_parser() = delete;
        config_parser(const config_parser& copy) = delete;
        config_parser(config_parser&& move) = delete;

        template<size_t _Sz>
        config_parser(const config_item (&config)[_Sz])
        {
            for (size_t i = 0; i < _Sz; ++i) {
                const config_item* def = &config[i];
                m_config[def->m_section][def->m_key] = { def, def->m_value };
            }
        }

        template<typename T>
        T get(const ST::string& section, const ST::string& key) const;

        template<typename T, size_t _SectionSz, size_t _KeySz>
        T get(const char(&section)[_SectionSz], const char(&key)[_KeySz]) const
        {
            return get<T>(ST::string::from_literal(section, _SectionSz-1),
                          ST::string::from_literal(key, _KeySz-1));
        }

        template<typename T>
        void set(const ST::string& section, const ST::string& key, T value);

        template<typename T, size_t _SectionSz, size_t _KeySz>
        void set(const char(&section)[_SectionSz], const char(&key)[_KeySz], T value)
        {
            set<T>(ST::string::from_literal(section, _SectionSz-1),
                   ST::string::from_literal(key, _KeySz-1),
                   value);
        }

        template<size_t _SectionSz, size_t _KeySz, size_t _ValueSz>
        void set(const char(&section)[_SectionSz], const char(&key)[_KeySz], const char(&value)[_ValueSz])
        {
            set<ST::string>(ST::string::from_literal(section, _SectionSz-1),
                            ST::string::from_literal(key, _KeySz-1),
                            ST::string::from_literal(value, _ValueSz-1));
        }

        /**
         * Reads in a configuration file from the given \param filename.
         * \note Any configuration values not defined in the items definition will be discarded.
         */
        bool read(const std::filesystem::path& filename);

        /**
         * Writes a copy of this configuration file to the given \param filename.
         * \note This includes all commentary given in the configuation definitions.
         */
        bool write(const std::filesystem::path& filename);

        void write(std::ostream& stream);
    };
};

#endif // _theme_CORE_CONFIG_PARSER_H
