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

# ----------- #
#   example   #
# ----------- #
add_executable(wor-bootio_example)

target_compile_features(wor-bootio_example
        PUBLIC
        c_std_23)

target_include_directories(wor-bootio_example
        PUBLIC
        src)

target_sources(wor-bootio_example
        PRIVATE
        src/dfu_file.c
        src/configuration.c
        src/transfer.c
        src/load_dfu.c
        src/load_dfuse.c
        src/crc32.c
        main.c)

target_link_libraries(wor-bootio_example
        PRIVATE
        libusb)
