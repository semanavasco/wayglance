# --- Project configuration ---
cmake_minimum_required(VERSION 3.15)

# Project name and language
project(wayglance 
	VERSION 0.0.36
	DESCRIPTION "A modular and customizable desktop widget for Wayland compositors"
	LANGUAGES CXX
)

# Tell CMake to generate the compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Specifying C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type handling
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Add debug options
add_compile_options(-g)


# --- Including our source files ---
file(GLOB_RECURSE SOURCES "src/*.cpp")


# --- Handle dependencies ---
# Telling CMake to use pkg-config to find libraries
find_package(PkgConfig REQUIRED)

# Asking pkg-config to find GTKmm libraries
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0 gtk4-layer-shell-0)

# VCPKG dependencies
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)


# --- Creating the executable ---
# Create 'wayglance' executable from source files
add_executable(wayglance ${SOURCES})


# --- Compiler-specific options --- 
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(wayglance PRIVATE
        -Wall -Wextra -Wpedantic  # More warnings
        $<$<CONFIG:Debug>:-g -O0 -fsanitize=address>  # Debug flags
        $<$<CONFIG:Release>:-O3 -DNDEBUG>  # Release flags
    )
    
    # Link sanitizer in debug mode
    target_link_options(wayglance PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address>
    )
endif()


# --- Linking dependencies and header files ---
# Telling 'wayglance' where to find its include dir
target_include_directories(wayglance PUBLIC include)

# Including GTKmm header files
target_include_directories(wayglance PUBLIC ${GTKMM_INCLUDE_DIRS})

# Linking the executable with GTKmm's libraries
target_link_libraries(wayglance PUBLIC
        ${GTKMM_LIBRARIES}
        nlohmann_json::nlohmann_json
	spdlog::spdlog
)

# Message once CMake is done
message(STATUS "Project configured! Go to 'build' and use 'make' to compile.")
