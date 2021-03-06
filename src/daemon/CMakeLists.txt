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

include_directories(${GFLAGS_INCLUDE_DIRS})
include_directories(${STRING_THEORY_INCLUDE_DIRS})

set(THEME_DAEMON_HEADERS
    client.h
    gatekeeper.h
    server.h
)

set(THEME_DAEMON_SOURCES
    client_common.cpp
    gatekeeper.cpp
    main.cpp
    server.cpp
)

add_executable(theme_daemon ${THEME_DAEMON_HEADERS} ${THEME_DAEMON_SOURCES})
target_link_libraries(theme_daemon ${GFLAGS_LIBRARIES})
target_link_libraries(theme_daemon theme_core)
target_link_libraries(theme_daemon theme_io)
target_link_libraries(theme_daemon theme_protocol)
