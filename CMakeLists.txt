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

include(FetchContent)
FetchContent_Declare(
    GRRLIB
    GIT_REPOSITORY https://github.com/GRRLIB/GRRLIB.git
    GIT_TAG origin/master
)

FetchContent_MakeAvailable(GRRLIB)

set(SOURCE_FILES
    src/main.cpp
)

set(LIBRARIES
    grrlib
)

add_executable(ff-wii ${SOURCE_FILES})
target_link_libraries(ff-wii PRIVATE ${LIBRARIES})
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
