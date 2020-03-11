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


# This file is based largely off the work of Matt Keeter
# See: https://www.mattkeeter.com/blog/2018-01-06-versioning/
# Updated to use FindGit :)
find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} log --pretty=format:'%h' -n 1
                    OUTPUT_VARIABLE GIT_REV
                    ERROR_QUIET)
else()
    set(GIT_REV "")
endif()

# Check whether we got any revision (which isn't always the case, e.g. when someone downloaded a
# zip file from Github instead of a checkout)
if ("${GIT_REV}" STREQUAL "")
    set(GIT_REV "")
    set(GIT_DIFF "")
    set(GIT_TAG "")
    set(GIT_BRANCH "")
else()
    execute_process(
        COMMAND git diff --quiet --exit-code
        RESULT_VARIABLE GIT_DIFF)
    execute_process(
        COMMAND git describe --exact-match --tags
        OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE GIT_BRANCH)

    if(${GIT_DIFF} STREQUAL "1")
        set(GIT_DIRTY "+")
    else()
        set(GIT_DIRTY "")
    endif()

    string(STRIP "${GIT_REV}" GIT_REV)
    string(SUBSTRING "${GIT_REV}" 1 7 GIT_REV)
    string(STRIP "${GIT_TAG}" GIT_TAG)
    string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
endif()

execute_process(COMMAND date "+%Y-%m-%d" OUTPUT_VARIABLE BUILD_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND date "+%H:%M:%S" OUTPUT_VARIABLE BUILD_TIME OUTPUT_STRIP_TRAILING_WHITESPACE)

set(VERSION
"namespace theme
{
    namespace buildinfo
    {
        const char* BUILD_HASH = \"${GIT_REV}${GIT_DIRTY}\";
        const char* BUILD_TAG = \"${GIT_TAG}\";
        const char* BUILD_BRANCH = \"${GIT_BRANCH}\";
        const char* BUILD_DATE = \"${BUILD_DATE}\";
        const char* BUILD_TIME = \"${BUILD_TIME}\";
    };
};
")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/buildinfo.cpp)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/buildinfo.cpp VERSION_)
else()
    set(VERSION_ "")
endif()

if (NOT "${VERSION}" STREQUAL "${VERSION_}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/buildinfo.cpp "${VERSION}")
endif()
