cmake_minimum_required(VERSION 3.10)

project(Game VERSION 1.0.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor CACHE FILEPATH "Path to vendor directory.")
set(RAYLIB_DIR ${VENDOR_DIR}/raylib)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "Configuration" FORCE);

# Set this to the minimal version you want to support
find_package(raylib 3.7.0 QUIET) # Let CMake search for a raylib-config.cmake

add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})

#find_library(RAYLIB raylib REQUIRED PATHS ${RAYLIB_DIR}/lib)

INCLUDE_DIRECTORIES(
   ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
)
LINK_DIRECTORIES(
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib
) 

add_compile_definitions(GAME_DEBUG)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
file(GLOB_RECURSE HEADER_FILES src/*.h)
add_executable(Game ${SRC_FILES} ${HEADER_FILES})

target_include_directories(Game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${RAYLIB_DIR}/include)
target_link_libraries(Game PUBLIC ${RAYLIB} GameNetworkingSockets)