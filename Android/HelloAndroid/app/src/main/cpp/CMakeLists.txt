#
# Copyright (C)  The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.4.1)

# FIXME: add as a second argument the binary build dir
# so that the first argument can link directly to FLTK
# instead of the fltk symlink, which is bad for SVN
add_subdirectory(../../../../../../ "${CMAKE_CURRENT_BINARY_DIR}/fltk" EXCLUDE_FROM_ALL)

# now build app's shared lib
add_library(android-hello SHARED
    HelloAndroid.cxx)

target_include_directories(android-hello PRIVATE 
    /Users/matt/dev/fltk-1.4.svn
    /Users/matt/dev/fltk-1.4.svn/Android)

# Export ANativeActivity_onCreate(), 
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# add lib dependencies
target_link_libraries(android-hello
    fltk
    android
    log
    m)

