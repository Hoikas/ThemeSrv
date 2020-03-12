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

cmake_minimum_required(VERSION 3.2)
project(ThemeSrv)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ThemeSrv_BINARY_DIR}/bin)

# Required third party libraries
find_package(gflags REQUIRED)
find_package(LibUUID REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(string_theory REQUIRED)

# Compile time config
include(TestBigEndian)
TEST_BIG_ENDIAN(THEME_BIG_ENDIAN)

include_directories("${PROJECT_BINARY_DIR}/include")
configure_file("${PROJECT_SOURCE_DIR}/src/theme_config.h.in" "${PROJECT_BINARY_DIR}/include/theme_config.h")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
