# Copyright (c) 2017 Cisco and/or its affiliates.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.2)

set(HTTP_CLIENT_SOURCE_FILES icnet_http_dash_client.cc)
set(HTTP_SERVER_SOURCE_FILES icnet_http_echo_server.cc)
set(IGET_SOURCE_FILES icnet_iget.cc)

add_executable(http-dash-client ${HTTP_CLIENT_SOURCE_FILES})
add_executable(http-echo-server ${HTTP_SERVER_SOURCE_FILES})
add_executable(iget ${IGET_SOURCE_FILES})


target_link_libraries(http-dash-client icnet ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
target_link_libraries(http-echo-server icnet ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
target_link_libraries(iget icnet ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

install(TARGETS http-dash-client DESTINATION bin COMPONENT library)
install(TARGETS iget DESTINATION bin COMPONENT library)
install(TARGETS http-echo-server DESTINATION bin COMPONENT library)