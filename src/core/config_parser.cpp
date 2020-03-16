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

#include "config_parser.h"
#include "errors.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <string>

// ============================================================================

theme::config_parser::sectionmap_t::iterator theme::config_parser::find_item(const ST::string& section, const ST::string& key)
{
    auto section_it = m_config.find(section);
    THEME_ASSERTD(section_it != m_config.end());
    auto item_it = section_it->second.find(key);
    THEME_ASSERTD(item_it != section_it->second.end());
    return item_it;
}

theme::config_parser::sectionmap_t::const_iterator theme::config_parser::find_item(const ST::string& section, const ST::string& key) const
{
    auto section_it = m_config.find(section);
    THEME_ASSERTD(section_it != m_config.end());
    auto item_it = section_it->second.find(key);
    THEME_ASSERTD(item_it != section_it->second.end());
    return item_it;
}

template<>
bool theme::config_parser::get<bool>(const ST::string& section, const ST::string& key) const
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_boolean);
    return item_it->second.m_value.to_bool();
}

template<>
float theme::config_parser::get<float>(const ST::string& section, const ST::string& key) const
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_float);
    return item_it->second.m_value.to_float();
}

template<>
int theme::config_parser::get<int>(const ST::string& section, const ST::string& key) const
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_integer);
    return item_it->second.m_value.to_int();
}

template<>
const char* theme::config_parser::get<const char*>(const ST::string& section, const ST::string& key) const
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_string);
    return item_it->second.m_value.c_str();
}

template<>
const ST::string& theme::config_parser::get<const ST::string&>(const ST::string& section, const ST::string& key) const
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_string);
    return item_it->second.m_value;
}

template<>
std::u16string theme::config_parser::get<std::u16string>(const ST::string& section, const ST::string& key) const
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_string);
    ST::utf16_buffer buf = item_it->second.m_value.to_utf16();
    return std::u16string(buf.begin(), buf.end());
}

template<>
unsigned int theme::config_parser::get<unsigned int>(const ST::string& section, const ST::string& key) const
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_integer);
    return item_it->second.m_value.to_uint();
}

// ============================================================================

template<>
void theme::config_parser::set<bool>(const ST::string& section, const ST::string& key, bool value)
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_boolean);
    item_it->second.m_value = ST::string::from_bool(value);
}

template<>
void theme::config_parser::set<float>(const ST::string& section, const ST::string& key, float value)
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_float);
    item_it->second.m_value = ST::string::from_float(value);
}

template<>
void theme::config_parser::set<int>(const ST::string& section, const ST::string& key, int value)
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_integer);
    item_it->second.m_value = ST::string::from_int(value);
}

template<>
void theme::config_parser::set<const ST::string&>(const ST::string& section, const ST::string& key, const ST::string& value)
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_string);
    item_it->second.m_value = value;
}

template<>
void theme::config_parser::set<unsigned int>(const ST::string& section, const ST::string& key, unsigned int value)
{
    auto item_it = find_item(section, key);
    THEME_ASSERTD(item_it->second.m_def->m_type == config_item::value_type::e_integer);
    item_it->second.m_value = ST::string::from_uint(value);
}

// ============================================================================

void theme::config_parser::set(size_t lineno, const configmap_t::iterator& section, const ST::string& key, const ST::string& value)
{
    if (section == m_config.end()) {
        std::cerr << "CONFIG: (Line: " << lineno << ") Found config item '" << key.c_str();
        std::cerr << "' outside of a valid section, ignoring." << std::endl;
        return;
    }

    auto item = section->second.find(key);
    if (item != section->second.end()) {
        if (!value.empty() || item->second.m_def->m_type == config_item::value_type::e_string)
            item->second.m_value = value;
        else
            item->second.m_value = ST_LITERAL("0");
    } else {
        std::cerr << "CONFIG: (Line: " << lineno << ") Found unknown config item '" << section->first.c_str();
        std::cerr << "." << key.c_str() << "' -- is it in the right section?" << std::endl;
    }
}

// ============================================================================
bool theme::config_parser::read(const std::filesystem::path& filename)
{
    if (!std::filesystem::exists(filename))
        return false;

    std::ifstream stream;
    stream.open(filename, std::ios_base::in);

    std::regex section_test("\\[(.*?)\\]");
    std::regex value_test("(\\w+)\\s*=\\s*([^]+(?!\\+{3}))");
    std::regex empty_test("(\\w+)\\s*=");

    size_t lineno = 0;
    configmap_t::iterator section = m_config.end();
    for (std::string buf; std::getline(stream, buf); ++lineno) {
        ST::string line = ST::string::from_std_string(buf).trim();
        if (line.empty())
            continue;
        if (line.starts_with(";") || line.starts_with("#") || line.starts_with("//"))
            continue;

        std::cmatch match;
        if (std::regex_search(line.c_str(), match, section_test)) {
            ST::string name = ST::string::from_std_string(match[1].str());
            section = m_config.find(name);
            if (section == m_config.end()) {
                std::cerr << "CONFIG: (Line: " << lineno << ") Found invalid section '" << name.c_str();
                std::cerr << "', ignoring." << std::endl;
            }
        } else if (std::regex_search(line.c_str(), match, value_test)) {
            ST::string key = ST::string::from_std_string(match[1].str());
            ST::string value = ST::string::from_std_string(match[2].str());
            set(lineno, section, key, value);
        } else if (std::regex_search(line.c_str(), match, empty_test)) {
            ST::string key = ST::string::from_std_string(match[1].str());
            set(lineno, section, key, ST::null);
        } else {
            std::cerr << "CONFIG: (Line: " << lineno << ") WTF?!?!?!" << std::endl;
        }
    }
    return true;
}

bool theme::config_parser::write(const std::filesystem::path& filename)
{
    std::ofstream stream;
    stream.open(filename, std::ios_base::out | std::ios_base::trunc);
    if (!stream.is_open())
        return false;
    write(stream);
    return true;
}

void theme::config_parser::write(std::ostream& stream)
{
    for (auto section : m_config) {
        stream << "[" << section.first.c_str() << "]" << std::endl;
        for (auto item = section.second.begin(); item != section.second.end(); ++item) {
            std::vector<ST::string> comments = item->second.m_def->m_description.tokenize("\r\n");
            if (!comments.empty() && item != section.second.begin())
                stream << std::endl;
            for (auto comment : comments)
                stream << "; " << comment.c_str() << std::endl;
            stream << item->first.c_str() << " = " << item->second.m_value.c_str() << std::endl;
        }
        stream << std::endl;
    }
}
