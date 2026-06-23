cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(runtime CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(ExternalProject)

find_package(Vulkan REQUIRED)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/sol3/CMakeLists.txt")
    file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/sol3")
    execute_process(COMMAND git clone --branch v3.3.0 https://github.com/ThePhD/sol2.git "${CMAKE_CURRENT_SOURCE_DIR}/vendor/sol3")
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit/include/luajit-2.1")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/vendor/luajit/src")

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

add_library(runtime SHARED ${SOURCES} ${HEADERS})
add_dependencies(runtime liblua)

set_target_properties(runtime PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        PREFIX ""
        OUTPUT_NAME "runtime"
)

target_include_directories(runtime PUBLIC
        include
        src
        src/vm/api
        src/vm/core
        ${Vulkan_INCLUDE_DIRS}
)

if(WIN32)
    target_link_libraries(runtime PRIVATE sol3 liblua Vulkan::Vulkan userenv winmm)
    target_link_options(runtime PRIVATE
            /WHOLEARCHIVE:$<TARGET_FILE:liblua>
    )
elseif(APPLE)
    target_link_libraries(runtime PRIVATE sol3 liblua Vulkan::Vulkan m dl pthread)
    target_link_options(runtime PRIVATE
            -Wl,-force_load,$<TARGET_FILE:liblua>
    )
else()
    target_link_libraries(runtime PRIVATE sol3 liblua Vulkan::Vulkan m dl pthread)
    target_link_options(runtime PRIVATE
            -Wl,--whole-archive
            $<TARGET_FILE:liblua>
            -Wl,--no-whole-archive
    )
endif()
