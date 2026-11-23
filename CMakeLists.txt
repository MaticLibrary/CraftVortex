cmake_minimum_required(VERSION 3.20)
project(CraftVortex LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Statycznie
set(BUILD_SHARED_LIBS OFF)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "Use MSVC runtime library DLL" FORCE)
endif()

# GLFW
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
add_subdirectory(include/GLFW)

# GLAD
add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC include)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

# Gra
file(GLOB_RECURSE CV_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(CraftVortex ${CV_SOURCES})
target_include_directories(CraftVortex PRIVATE include src)
target_link_libraries(CraftVortex PRIVATE glfw glad)

include(CheckLinkerFlag)
option(CV_INSTALL_DEPS "Install missing dependencies (requires sudo)" OFF)

if(MINGW)
    target_link_options(CraftVortex PRIVATE -static -static-libgcc -static-libstdc++)
elseif(UNIX AND NOT APPLE)
    unset(CV_HAS_STATIC_LIBSTDCXX CACHE)
    check_linker_flag(CXX "-static-libstdc++" CV_HAS_STATIC_LIBSTDCXX)

    if(NOT CV_HAS_STATIC_LIBSTDCXX)
        message(STATUS "Static g++ libraries are missing, launching install_cpp.sh")
        execute_process(
            COMMAND bash ./install_cpp.sh
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE CV_INSTALL_RESULT)
        if(NOT CV_INSTALL_RESULT EQUAL 0)
            message(WARNING "ERROR: install_cpp.sh failed: (${CV_INSTALL_RESULT}).")
        endif()

        unset(CV_HAS_STATIC_LIBSTDCXX CACHE)
        check_linker_flag(CXX "-static-libstdc++" CV_HAS_STATIC_LIBSTDCXX)
    endif()

    if(NOT CV_HAS_STATIC_LIBSTDCXX)
        message(FATAL_ERROR
            "ERROR: libstdc++ is not installed or cannot be linked statically. "
            "Please install it manually and run cmake again.")
    endif()

    target_link_options(CraftVortex PRIVATE -static-libgcc -static-libstdc++)
endif()

if(MSVC)
    target_compile_options(CraftVortex PRIVATE /W4)
else()
    target_compile_options(CraftVortex PRIVATE -Wall -Wextra)
endif()

add_custom_command(TARGET CraftVortex POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:CraftVortex>/assets)