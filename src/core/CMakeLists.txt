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

include_directories(${STRING_THEORY_INCLUDE_DIRS})
include_directories(${LibUUID_INCLUDE_DIR})

set(THEME_CORE_HEADERS
    build_info.h
    config_parser.h
    errors.h
    log.h
    uuid.h
)

set(THEME_CORE_SOURCES
    build_info.cpp
    config_parser.cpp
    errors.cpp
    log.cpp
    uuid.cpp
)

add_library(theme_core STATIC ${THEME_CORE_HEADERS} ${THEME_CORE_SOURCES})
target_link_libraries(theme_core buildinfoobj)
target_link_libraries(theme_core LibUUID::LibUUID)
target_link_libraries(theme_core ${STRING_THEORY_LIBRARIES})

# GCC std::filesystem hax
if(CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(theme_core stdc++fs)
endif()
