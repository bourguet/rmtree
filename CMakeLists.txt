# Copyright (c) 2017  Jean-Marc Bourguet
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# * Neither the name of Jean-Marc Bourguet nor the names of the other
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.5)
project(rmtree CXX)
set(CMAKE_CXX_STANDARD 98)

option(DevMode "Development mode" OFF)

if(DevMode)
    list(INSERT CMAKE_PREFIX_PATH 0 "${PROJECT_SOURCE_DIR}/..")
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/..")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
else()
    if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "/opt/bourguet" CACHE PATH "..." FORCE)
    endif()
endif()

add_subdirectory(doc)
add_subdirectory(src)

add_custom_target(install-${PROJECT_NAME}
        $(MAKE) install
        COMMENT "Installing ${PROJECT_NAME}")

add_custom_target(dist
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/.."
        COMMAND tar cjfX ${PROJECT_NAME}-$(VERSION).tar.bz ${PROJECT_NAME}/excluded ${PROJECT_NAME})
