cmake_minimum_required(VERSION 3.23)

project(wor-bootio
        LANGUAGES C
        VERSION 0.0.0)

find_package(PkgConfig)

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
        src/load_dfu.c
        src/load_dfuse.c
        src/crc32.c
        src/memory_segment.c
        src/dfuse_command.c)

target_sources(wor_bootio
        PRIVATE ${Sources})

target_include_directories(wor_bootio
        PUBLIC src)

target_link_libraries(wor_bootio
        PUBLIC
        libusb)

target_compile_features(wor_bootio
        PUBLIC
        $<IF:$<C_COMPILER_ID:MSVC>,c_std_17,c_std_23>)

# ----------- #
#   Example   #
# ----------- #
add_executable(wor_bootio-example main.c)

target_link_libraries(wor_bootio-example
        PRIVATE
        Wor::BootIo)
