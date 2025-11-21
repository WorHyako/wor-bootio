option(WorBootio_Enable_ClangTidy "Enable clang-tidy analyze" OFF)
option(WorBootio_Enable_AddressSanitizer "Enable AddressSanitizer" OFF)

find_program(ClangTidy_Exe NAMES clang-tidy clang-tidy-15 clang-tidy-16)

function(CreateAnalyzeTargetFor TargetName)
    if (NOT ClangTidy_Exe AND WorBootio_Enable_ClangTidy)
        message(WARNING "Error to create analyze target. Can't find clang-tidy.")
        return()
    endif ()

    set(CMAKE_C_CLANG_TIDY "${ClangTidy_Exe}")

    if (WorBootio_Enable_AddressSanitizer)
        if (C_COMPILER_ID STREQUAL "MSVC")
            set(SanitizerFlags [=[/fsanitize=address]=])
        elseif (C_COMPILER_ID STREQUAL "Clang")
            set(SanitizerFlags [=[-fsanitize=address,undefined]=])
        endif ()
        target_compile_options(${TargetName}
                PRIVATE
                ${SanitizerFlags})
        target_link_options(${TargetName}
                PRIVATE
                ${SanitizerFlags})
    endif ()

    add_custom_target(${TargetName}-analyze
            COMMAND ${ClangTidy_Exe}
            $<TARGET_PROPERTY:${TargetName},SPECIFIED_SOURCES>
            -p ${CMAKE_BINARY_DIR}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "clang-tidy analyze for '${TargetName}' target"
            VERBATIM)
endfunction()
