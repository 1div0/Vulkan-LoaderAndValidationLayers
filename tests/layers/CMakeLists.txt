cmake_minimum_required (VERSION 2.8.11)

set(LAYER_JSON_FILES
    VkLayer_wrap_objects
    VkLayer_device_profile_api
    VkLayer_test
    )

set(VK_LAYER_RPATH /usr/lib/x86_64-linux-gnu/vulkan/layer:/usr/lib/i386-linux-gnu/vulkan/layer)
set(CMAKE_INSTALL_RPATH ${VK_LAYER_RPATH})

if (WIN32)
    if (NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
        foreach (config_file ${LAYER_JSON_FILES})
            FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/windows/${config_file}.json src_json)
            if (CMAKE_GENERATOR MATCHES "^Visual Studio.*")
                FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/${config_file}.json dst_json)
            else()
                FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${config_file}.json dst_json)
            endif()
            add_custom_target(${config_file}-json ALL
                COMMAND copy ${src_json} ${dst_json}
                VERBATIM
                )
            add_dependencies(${config_file}-json ${config_file})
            set_target_properties(${config_file}-json PROPERTIES FOLDER ${LVL_TARGET_FOLDER})
        endforeach(config_file)
    endif()
elseif(APPLE)
    # extra setup for out-of-tree builds
    if (NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
        foreach (config_file ${LAYER_JSON_FILES})
            add_custom_target(${config_file}-json ALL
                COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/macos/${config_file}.json
                VERBATIM
                )
        endforeach(config_file)
    endif()
else()
    # extra setup for out-of-tree builds
    if (NOT (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR))
        foreach (config_file ${LAYER_JSON_FILES})
            add_custom_target(${config_file}-json ALL
                COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/linux/${config_file}.json
                VERBATIM
                )
        endforeach(config_file)
    endif()
endif()

if (WIN32)
    macro(add_vk_layer target)
    FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/VkLayer_${target}.def DEF_FILE)
    add_custom_target(copy-${target}-def-file ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DEF_FILE} VkLayer_${target}.def
        VERBATIM
    )
    set_target_properties(copy-${target}-def-file PROPERTIES FOLDER ${LVL_TARGET_FOLDER})
    add_library(VkLayer_${target} SHARED ${ARGN} VkLayer_${target}.def)
    add_dependencies(VkLayer_${target} generate_helper_files VkLayer_utils)
    endmacro()
elseif(APPLE)
    macro(add_vk_layer target)
    add_library(VkLayer_${target} SHARED ${ARGN})
    add_dependencies(VkLayer_${target} generate_helper_files VkLayer_utils)
    set_target_properties(VkLayer_${target} PROPERTIES LINK_FLAGS "-Wl")
    endmacro()
else()
    macro(add_vk_layer target)
    add_library(VkLayer_${target} SHARED ${ARGN})
    add_dependencies(VkLayer_${target} generate_helper_files VkLayer_utils)
    set_target_properties(VkLayer_${target} PROPERTIES LINK_FLAGS "-Wl,-Bsymbolic")
    endmacro()
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../layers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../loader
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/vulkan
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}
)

if (WIN32)
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_CRT_SECURE_NO_WARNINGS")
    set (CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} -D_CRT_SECURE_NO_WARNINGS")
    set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -D_CRT_SECURE_NO_WARNINGS /bigobj")
    set (CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG} -D_CRT_SECURE_NO_WARNINGS /bigobj")
else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith -Wno-unused-function")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith -Wno-unused-function")
endif()

set (WRAP_SRCS
       wrap_objects.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../../layers/vk_layer_table.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../../layers/vk_layer_extension_utils.cpp
       )
add_vk_layer(wrap_objects ${WRAP_SRCS})

set (TEST_SRCS
       test.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../../layers/vk_layer_table.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../../layers/vk_layer_extension_utils.cpp
       )

set (DEVICE_PROFILE_API_SRCS
       device_profile_api.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../../layers/vk_layer_table.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/../../layers/vk_layer_extension_utils.cpp
       )

add_vk_layer(device_profile_api ${DEVICE_PROFILE_API_SRCS})
add_vk_layer(test ${TEST_SRCS})

if (WIN32)
    # For Windows, copy necessary gtest DLLs to the right spot for the vk_layer_tests...
    FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/*device_profile_api.* SRC_LAYER)
    FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/windows/VkLayer_device_profile_api.json SRC_JSON)
    FILE(TO_NATIVE_PATH ${PROJECT_BINARY_DIR}/layers/$<CONFIGURATION> DST_LAYER)
    add_custom_command(TARGET VkLayer_device_profile_api POST_BUILD
        COMMAND xcopy /Y /I ${SRC_LAYER} ${DST_LAYER}
        COMMAND xcopy /Y /I ${SRC_JSON} ${DST_LAYER})
elseif(APPLE)
    if (CMAKE_GENERATOR MATCHES "^Xcode.*")
        add_custom_command(TARGET VkLayer_device_profile_api POST_BUILD
            COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/VkLayer_device_profile_api.json ${CMAKE_BINARY_DIR}/layers/$<CONFIG>
            COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/libVkLayer_device_profile_api.dylib ${CMAKE_BINARY_DIR}/layers/$<CONFIG>
            VERBATIM
            )
    else()
        add_custom_command(TARGET VkLayer_device_profile_api POST_BUILD
            COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/VkLayer_device_profile_api.json ${CMAKE_BINARY_DIR}/layers
            COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/libVkLayer_device_profile_api.dylib ${CMAKE_BINARY_DIR}/layers
            VERBATIM
            )
    endif()
else()
    add_custom_command(TARGET VkLayer_device_profile_api POST_BUILD
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/VkLayer_device_profile_api.json ${PROJECT_BINARY_DIR}/layers
        COMMAND ln -sf ${CMAKE_CURRENT_BINARY_DIR}/libVkLayer_device_profile_api.so ${PROJECT_BINARY_DIR}/layers
        VERBATIM
        )
endif()
