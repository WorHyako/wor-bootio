# Copyright (c) 2025 WorHyako
# Licensed under the MIT License

cmake_minimum_required(VERSION 3.23)

option(WOR_BOOTIO_ENABLE_TESTS "Enable tests for Wor-BootIo" OFF)

project(wor-bootio
        LANGUAGES C
        VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(TargetStaticAnalyze)
include(ThirdPartyDeps)

# ---------- #
#   libusb   #
# ---------- #
pkg_check_modules(libusb libusb-1.0 REQUIRED)

add_library(libusb INTERFACE)

target_include_directories(libusb
        INTERFACE
        ${libusb_INCLUDE_DIRS})

target_link_libraries(libusb
        INTERFACE
        ${libusb_LINK_LIBRARIES})

# --------------- #
#   Wor::BootIo   #
# --------------- #
add_library(wor_bootio)
add_library(Wor::BootIo ALIAS wor_bootio)

set(Sources
        src/dfu_file.c
        src/configuration.c
        src/transfer.c
        src/dfu_status.c
        src/load_dfu.c
        src/load_dfuse.c
        src/crc32.c
        src/memory_segment.c
        src/dfuse_command.c)

set(PublicHeaders
        include/bootio_error.h
        include/configuration.h
        include/dfu_file.h
        include/dfu_func_dt.h
        include/dfu_status.h
        include/dfuse_file.h
        include/load.h
        include/portable.h)

set(PrivateHeaders
        src/crc32.h
        src/dfuse_command.h
        src/memory_segment.h
        src/transfer.h)

target_sources(wor_bootio
        PRIVATE
        ${Sources}
        PRIVATE
        FILE_SET private_headers
        TYPE HEADERS
        FILES ${PrivateHeaders}
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        FILES ${PublicHeaders}
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(wor_bootio
        PUBLIC include
        PRIVATE src)

target_link_libraries(wor_bootio
        PUBLIC
        libusb)

target_compile_features(wor_bootio
        PUBLIC
        $<IF:$<C_COMPILER_ID:MSVC>,c_std_17,c_std_23>)

CreateAnalyzeTargetFor(wor_bootio)

# ----------- #
#   Example   #
# ----------- #
add_executable(wor_bootio-example main.c)

target_link_libraries(wor_bootio-example
        PRIVATE
        Wor::BootIo)

if (WOR_BOOTIO_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()
