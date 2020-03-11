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
include_directories(${OPENSSL_INCLUDE_DIR})

set(THEME_IO_HEADERS
    uru_crypt.h
)

set(THEME_IO_SOURCES
    uru_crypt.cpp
)

add_library(theme_io STATIC ${THEME_IO_HEADERS} ${THEME_IO_SOURCES})
target_link_libraries(theme_io OpenSSL::Crypto)
target_link_libraries(theme_io ${STRING_THEORY_LIBRARIES})
