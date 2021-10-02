cmake_minimum_required(VERSION 3.10)

project(Game VERSION 1.0.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor CACHE FILEPATH "Path to vendor directory.")
set(RAYLIB_DIR ${VENDOR_DIR}/raylib)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "Configuration" FORCE)

# Set this to the minimal version you want to support
find_package(raylib 3.7.0 QUIET) # Let CMake search for a raylib-config.cmake
add_subdirectory("vendor/raylib" "vendor/raylib/bin")
#find_library(RAYLIB raylib REQUIRED PATHS ${RAYLIB_DIR}/lib)

INCLUDE_DIRECTORIES(
   ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
)
LINK_DIRECTORIES(
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib
) 

file(GLOB_RECURSE SRC_FILES src/*.cpp)
file(GLOB_RECURSE HEADER_FILES src/*.h)
add_executable(Game ${SRC_FILES} ${HEADER_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES} ${HEADER_FILES})

target_include_directories(Game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${RAYLIB_DIR}/include)
target_link_libraries(Game PUBLIC raylib GameNetworkingSockets)

if("${CMAKE_CONFIGURATION_TYPES}" STREQUAL "Debug")
    add_compile_definitions(GAME_DEBUG)
elseif("${CMAKE_CONFIGURATION_TYPES}" STREQUAL "Release")
    add_compile_definitions(GAME_RELEASE)
elseif("${CMAKE_CONFIGURATION_TYPES}" STREQUAL "Dist")
    add_compile_definitions(GAME_DIST)
endif()

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Game)
endif()

# For moving assets folder around
#if(MSVC)
#    set_target_properties(Game PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
#else()
#    # Set a symbolic link so exe can find the /assets folder
#    set(link_src "${CMAKE_SOURCE_DIR}/assets")
#    if(UNIX AND NOT APPLE) 
#        set(link_dst "${CMAKE_CURRENT_BINARY_DIR}/assets") 
#    else() 
#        set(link_dst "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/assets") 
#    endif() 
#    add_custom_command( 
#        TARGET Game
#        POST_BUILD COMMAND ${CMAKE_COMMAND} 
#            -E create_symlink ${link_src} ${link_dst} 
#        DEPENDS ${link_dst} 
#        COMMENT "symbolic link ${link_src} => ${link_dst}" 
#    )
#endif()

#add_custom_target(copy_assets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
#add_dependencies(Game copy_assets)