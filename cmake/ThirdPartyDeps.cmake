# Copyright (c) 2025 WorHyako
# Licensed under the MIT License

include(FetchContent)

# --------- #
#   Unity   #
# --------- #
FetchContent_Declare(unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG cbcd08fa7de711053a3deec6339ee89cad5d2697 # v2.6.1
        OVERRIDE_FIND_PACKAGE)

FetchContent_MakeAvailable(unity)

# --------- #
#   CMock   #
# --------- #
FetchContent_Declare(cmock
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/CMock.git
        GIT_TAG 49435d9f3313abb3422252d3d2dc12a7542332ca # v2.6.0
        OVERRIDE_FIND_PACKAGE)

FetchContent_MakeAvailable(cmock)

add_library(cmock INTERFACE)

target_include_directories(cmock
        INTERFACE ${cmock_SOURCE_DIR}/src)

add_library(cmock::cmock ALIAS cmock)
