cmake_minimum_required(VERSION 3.30)

project(winyl VERSION 0.1 DESCRIPTION "Wii network library")

set(CMAKE_SYSTEM_NAME NintendoWii)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS_DEBUG "-Og -ggdb3")
set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(winyl STATIC
        src/header.c
        src/request.c
        src/version.c
        src/winyl.c
)
set_target_properties(winyl PROPERTIES VERSION ${PROJECT_VERSION})

target_include_directories(winyl
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

string(LENGTH "0.1" VERSION_LEN)

target_compile_definitions(winyl
        PRIVATE
        _WINYL_VERSION="0.1"
        _WINYL_VERSION_LEN=${VERSION_LEN}
)

install(DIRECTORY include/ DESTINATION include)
install(TARGETS winyl
        EXPORT winylTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)
install(EXPORT winylTargets
        FILE winylTargets.cmake
        NAMESPACE winyl::
        DESTINATION lib/cmake/winyl
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/winylConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/winylConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/winylConfig.cmake" @ONLY
)

install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/winylConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/winylConfigVersion.cmake"
        DESTINATION lib/cmake/winyl
)