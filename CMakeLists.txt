cmake_minimum_required(VERSION 3.30...4.0 FATAL_ERROR)

project(Wandelt VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_COLOR_DIAGNOSTICS ON)

option(COMPILE_COMMANDS "Generate compile_commands.json" ON)

# -----------------------------------------------------------------------------

if(COMPILE_COMMANDS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

if(MSVC)
    add_compile_options(
        /W4
        # /WX
        /permissive-
        /Zc:preprocessor
        /utf-8
        /MP
    )

    add_compile_options(
        $<$<CONFIG:Debug>:/Zi>
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/RTC1>
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/Ob3>
        $<$<CONFIG:Release>:/DNDEBUG>
        $<$<CONFIG:Release>:/GL>
    )

    add_link_options(
        $<$<CONFIG:Release>:/LTCG>
    )
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        # -Werror
        -Wshadow
        -Wcast-align
        -Wunused
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wmisleading-indentation
        -Wextra-semi
        -Wstrict-prototypes
        -Wmissing-prototypes
        -fcolor-diagnostics
        -fansi-escape-codes

        -Wno-keyword-macro
        -Wno-unused-function
    )

    add_compile_options(
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        # $<$<CONFIG:Debug>:-fsanitize=address,undefined>
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-DNDEBUG>
        $<$<CONFIG:Release>:-flto>
    )

    add_link_options(
        # $<$<CONFIG:Debug>:-fsanitize=address,undefined>
        $<$<CONFIG:Release>:-flto>
    )
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        # -Werror
        -Wshadow
        -Wcast-align
        -Wunused
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wextra-semi
        -Wstrict-prototypes
        -Wmissing-prototypes
        -Wjump-misses-init
        -Wstringop-overflow=4
        -fdiagnostics-color=always
    )
    add_compile_options(
        $<$<CONFIG:Debug>:-g3>
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-DNDEBUG>
        $<$<CONFIG:Release>:-flto>
        $<$<CONFIG:Release>:-march=native>
    )
    add_link_options(
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
        $<$<CONFIG:Release>:-flto>
    )
endif()

# -----------------------------------------------------------------------------

add_compile_definitions(
    $<$<CONFIG:Debug>:BUILD_DEBUG>
    $<$<CONFIG:Release>:BUILD_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:BUILD_RELWITHDEBINFO>
    $<$<CONFIG:MinSizeRel>:BUILD_MINSIZEREL>
)

set(DEMO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/demo/)
add_compile_definitions(DEMO_PATH="${DEMO_PATH}")
message(STATUS "Demos path: ${DEMO_PATH}")

# -----------------------------------------------------------------------------

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
