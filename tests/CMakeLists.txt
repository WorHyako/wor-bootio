set(CMockConfig ${CMAKE_CURRENT_SOURCE_DIR}/cmock.yml)

get_target_property(Includes wor_bootio HEADER_SET_public_headers)
list(APPEND WorBootio_Headers ${Includes})
get_target_property(Includes wor_bootio HEADER_SET_private_headers)
list(APPEND WorBootio_Headers ${Includes})
unset(Includes)

# Coz of libusb_device pointer
list(REMOVE_ITEM WorBootio_Headers ${CMAKE_SOURCE_DIR}/include/configuration.h)
set(MockOutputDir ${CMAKE_CURRENT_SOURCE_DIR}/mocks)
file(MAKE_DIRECTORY ${MockOutputDir})

foreach (WorBootio_Header ${WorBootio_Headers})
    get_filename_component(HeaderName ${WorBootio_Header} NAME_WE)
    add_custom_command(OUTPUT
            ${MockOutputDir}/mock_${HeaderName}.h
            ${MockOutputDir}/mock_${HeaderName}.c
            COMMAND ruby ${cmock_SOURCE_DIR}/lib/cmock.rb
            -o${CMockConfig}
            ${WorBootio_Header}
            DEPENDS
            ${WorBootio_Header}
            ${CMockConfig}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating CMock mocks for ${WorBootio_Header}")
    list(APPEND WorBootio_TestHeaders ${MockOutputDir}/mock_${HeaderName}.h)
    list(APPEND WorBootio_TestSources ${MockOutputDir}/mock_${HeaderName}.c)
endforeach ()

add_custom_target(generate_mocks
        DEPENDS
        ${WorBootio_TestHeaders}
        ${WorBootio_TestSources})

add_executable(wor_bootio_tests)

target_sources(wor_bootio_tests
        PRIVATE
        unit/test_upload.c
        ${WorBootio_TestSources})

add_dependencies(wor_bootio_tests generate_mocks)

target_link_libraries(wor_bootio_tests
        PRIVATE
        wor_bootio
        cmock::cmock
        unity)

get_target_property(WorBootio_AllIncludeDirs
        wor_bootio INCLUDE_DIRECTORIES)

target_include_directories(wor_bootio_tests
        PRIVATE
        ${WorBootio_AllIncludeDirs}
        ${MockOutputDir})
