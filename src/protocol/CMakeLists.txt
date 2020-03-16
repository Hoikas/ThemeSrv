#    This file is part of ThemeSrv.
#
#    ThemeSrv is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    ThemeSrv is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with ThemeSrv.  If not, see <https://www.gnu.org/licenses/>.

set(THEME_PROTOCOL_HEADERS
    common.h
    gatekeeper.h
    net_struct.h
)

set(THEME_PROTOCOL_INLINES
    protocol_fields_begin.inl
    protocol_fields_end.inl
    protocol_objects_begin.inl
    protocol_objects_end.inl
    protocol_structs_begin.inl
    protocol_structs_end.inl
)

set(THEME_PROTOCOL_PROTOCOLS
    common.inl
    gatekeeper.inl
)

set(THEME_PROTOCOL_SOURCES
    net_struct.cpp
    protocol.cpp
)

add_library(theme_protocol STATIC ${THEME_PROTOCOL_HEADERS} ${THEME_PROTOCOL_INLINES}
                                  ${THEME_PROTOCOL_PROTOCOLS} ${THEME_PROTOCOL_SOURCES})
target_link_libraries(theme_protocol theme_io)
