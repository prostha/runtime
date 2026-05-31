cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(polyvec CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(ExternalProject)

find_package(Vulkan REQUIRED)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/uv/CMakeLists.txt")
    file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/uv")
    execute_process(COMMAND git clone --branch v1.48.0 https://github.com/libuv/libuv.git "${CMAKE_CURRENT_SOURCE_DIR}/vendor/uv")
endif()
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet/CMakeLists.txt")
    file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet")
    execute_process(COMMAND git clone --depth 1 https://github.com/lsalzman/enet.git "${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet")
endif()
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lws/CMakeLists.txt")
    file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lws")
    execute_process(COMMAND git clone --branch v4.3-stable https://github.com/warmcat/libwebsockets.git "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lws")
endif()
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/sol3/CMakeLists.txt")
    file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/sol3")
    execute_process(COMMAND git clone --branch v3.3.0 https://github.com/ThePhD/sol2.git "${CMAKE_CURRENT_SOURCE_DIR}/vendor/sol3")
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit/include/luajit-2.1")

if(WIN32)
    set(EXT "lib")
    set(UVPATH "${CMAKE_CURRENT_SOURCE_DIR}/vendor/uv/lib/uv_a.lib")
else()
    set(EXT "a")
    set(UVPATH "${CMAKE_CURRENT_SOURCE_DIR}/vendor/uv/lib/libuv.a")
endif()

if(WIN32)
    ExternalProject_Add(luajit
            PREFIX "${CMAKE_BINARY_DIR}/vendor/luajit"
            GIT_REPOSITORY https://github.com/LuaJIT/LuaJIT.git
            GIT_TAG v2.1
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit"
            BUILD_IN_SOURCE TRUE
            CONFIGURE_COMMAND ""
            BUILD_COMMAND cmd /c src/msvcbuild.bat static
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy src/lua51.lib "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit/lib/lua51.lib"
            BUILD_BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit/lib/lua51.lib"
    )
elseif(APPLE)
    ExternalProject_Add(luajit
            PREFIX "${CMAKE_BINARY_DIR}/vendor/luajit"
            GIT_REPOSITORY https://github.com/LuaJIT/LuaJIT.git
            GIT_TAG v2.1
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit"
            BUILD_IN_SOURCE TRUE
            CONFIGURE_COMMAND ""
            BUILD_COMMAND env MACOSX_DEPLOYMENT_TARGET=11.0 make -j4 PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit BUILDMODE=static CFLAGS=-fPIC
            INSTALL_COMMAND env MACOSX_DEPLOYMENT_TARGET=11.0 make install PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit
            BUILD_BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit/lib/libluajit-5.1.a"
    )
else()
    ExternalProject_Add(luajit
            PREFIX "${CMAKE_BINARY_DIR}/vendor/luajit"
            GIT_REPOSITORY https://github.com/LuaJIT/LuaJIT.git
            GIT_TAG v2.1
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit"
            BUILD_IN_SOURCE TRUE
            CONFIGURE_COMMAND ""
            BUILD_COMMAND make -j4 PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit BUILDMODE=static CFLAGS=-fPIC
            INSTALL_COMMAND make install PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit
            BUILD_BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit/lib/libluajit-5.1.a"
    )
endif()

ExternalProject_Add(uv
        PREFIX "${CMAKE_BINARY_DIR}/vendor/uv"
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/uv"
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/vendor/uv -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_TESTING=OFF -DLIBUV_BUILD_SHARED=OFF
        BUILD_BYPRODUCTS "${UVPATH}"
)

ExternalProject_Add(enet
        PREFIX "${CMAKE_BINARY_DIR}/vendor/enet"
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet"
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        BUILD_BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet/lib/libenet.${EXT}"
)

ExternalProject_Add(lws
        PREFIX "${CMAKE_BINARY_DIR}/vendor/lws"
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lws"
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/vendor/lws
        -DLWS_WITH_SHARED=OFF
        -DLWS_WITH_SSL=OFF
        -DLWS_WITHOUT_TESTAPPS=ON
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        BUILD_BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lws/lib/libwebsockets.${EXT}"
)

add_library(liblua STATIC IMPORTED GLOBAL)
if(WIN32)
    set_target_properties(liblua PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit/lib/lua51.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit/src"
    )
else()
    set_target_properties(liblua PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit/lib/libluajit-5.1.a"
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit/include/luajit-2.1"
    )
endif()
add_dependencies(liblua luajit)

add_library(libuv STATIC IMPORTED GLOBAL)
set_target_properties(libuv PROPERTIES
        IMPORTED_LOCATION "${UVPATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/uv/include"
)
add_dependencies(libuv uv)

add_library(libenet STATIC IMPORTED GLOBAL)
set_target_properties(libenet PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet/lib/libenet.${EXT}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/enet/include"
)
add_dependencies(libenet enet)

add_library(liblws STATIC IMPORTED GLOBAL)
set_target_properties(liblws PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lws/lib/libwebsockets.${EXT}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lws/include"
)
add_dependencies(liblws lws)

add_library(sol3 INTERFACE)
target_include_directories(sol3 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/sol3/include")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

set(ENTRY "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM SOURCES ${ENTRY})

add_library(lib SHARED ${SOURCES} ${HEADERS})
add_dependencies(lib liblua libuv libenet liblws)

set_target_properties(lib PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        OUTPUT_NAME "polyvec"
)

target_include_directories(lib PUBLIC
        include
        src
        src/vm/api
        src/vm/core
        ${Vulkan_INCLUDE_DIRS}
)

if(WIN32)
    target_link_libraries(lib PRIVATE sol3 liblua libuv libenet liblws Vulkan::Vulkan ws2_32 iphlpapi userenv winmm)
    target_link_options(lib PRIVATE
            /WHOLEARCHIVE:$<TARGET_FILE:liblua>
            /WHOLEARCHIVE:$<TARGET_FILE:libuv>
            /WHOLEARCHIVE:$<TARGET_FILE:libenet>
            /WHOLEARCHIVE:$<TARGET_FILE:liblws>
    )
elseif(APPLE)
    target_link_libraries(lib PRIVATE sol3 liblua libuv libenet liblws Vulkan::Vulkan m dl pthread)
    target_link_options(lib PRIVATE
            -Wl,-force_load,$<TARGET_FILE:liblua>
            -Wl,-force_load,$<TARGET_FILE:libuv>
            -Wl,-force_load,$<TARGET_FILE:libenet>
            -Wl,-force_load,$<TARGET_FILE:liblws>
    )
else()
    target_link_libraries(lib PRIVATE sol3 liblua libuv libenet liblws Vulkan::Vulkan m dl pthread)
    target_link_options(lib PRIVATE
            -Wl,--whole-archive
            $<TARGET_FILE:liblua>
            $<TARGET_FILE:libuv>
            $<TARGET_FILE:libenet>
            $<TARGET_FILE:liblws>
            -Wl,--no-whole-archive
    )
endif()