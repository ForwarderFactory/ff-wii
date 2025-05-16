cmake_minimum_required(VERSION 3.11)
project(ff-wii)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
if (NOT RUN_BIN)
    set(RUN_BIN On)
endif()

include_directories(include)
include_directories(data-headers)

include(FetchContent)
FetchContent_Declare(
    GRRLIB
    GIT_REPOSITORY https://github.com/GRRLIB/GRRLIB.git
    GIT_TAG origin/master
)

FetchContent_MakeAvailable(GRRLIB)

find_package(Python3 REQUIRED)

set(SOURCE_FILES
    src/main.cpp
)

set(DATA_FILES
    data/img.jpg
)

set(LIBRARIES
    grrlib
    jpeg
)

function(generate_cpp_headers OUT_VAR)
    set(headers "")
    set(SCRIPT "${CMAKE_SOURCE_DIR}/py/bin_to_header.py")

    foreach(input_file IN LISTS ARGN)
        get_filename_component(filename ${input_file} NAME)
        set(output_file "${CMAKE_SOURCE_DIR}/data-headers/${filename}.hpp")

        file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/data-headers")

        add_custom_command(
            OUTPUT ${output_file}
            COMMAND ${Python3_EXECUTABLE} ${SCRIPT} ${CMAKE_SOURCE_DIR}/${input_file} ${output_file}
            DEPENDS ${input_file} ${SCRIPT}
            COMMENT "Generating C++ header for ${input_file} -> ${output_file}"
            VERBATIM
        )

        list(APPEND headers ${output_file})
    endforeach()

    set(${OUT_VAR} "${headers}" PARENT_SCOPE)
endfunction()

generate_cpp_headers(HEADER_FILES ${DATA_FILES})
add_custom_target(data-headers DEPENDS ${HEADER_FILES})

add_executable(ff-wii ${SOURCE_FILES})
target_link_libraries(ff-wii PRIVATE ${LIBRARIES})
add_dependencies(ff-wii data-headers)
ogc_create_dol(ff-wii)

if (RUN_BIN)
    if (NOT DOLPHIN_BIN AND RUN_BIN)
        set(DOLPHIN_BIN "dolphin-emu")
    endif()

    add_custom_command(TARGET ff-wii POST_BUILD
            COMMAND ${DOLPHIN_BIN}
            ARGS "${CMAKE_CURRENT_BINARY_DIR}/ff-wii.dol"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Launching Dolphin with ff-wii.dol"
    )
endif()
