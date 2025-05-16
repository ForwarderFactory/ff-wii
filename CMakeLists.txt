cmake_minimum_required(VERSION 3.10)
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

add_executable(ff-wii main.cpp)
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