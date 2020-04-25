
#-------------------------------------------------------------------------------
# Checks
#-------------------------------------------------------------------------------

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR OR EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    message(SEND_ERROR "In-source builds are not allowed. Please create a separate build directory and run 'cmake /path/to/folder [options]' there.")
    message(FATAL_ERROR "You can remove the file \"CMakeCache.txt\" and directory \"CMakeFiles\" in ${CMAKE_SOURCE_DIR}.")
endif()

#-------------------------------------------------------------------------------
# Platform Detection
#-------------------------------------------------------------------------------
get_directory_property(hasParent PARENT_DIRECTORY)

if(APPLE)
    if(hasParent)
        set(SMTG_MAC TRUE PARENT_SCOPE)
    else()
        set(SMTG_MAC TRUE)
    endif()
elseif(UNIX OR ANDROID_PLATFORM)
    if(hasParent)
        set(SMTG_LINUX TRUE PARENT_SCOPE)
    else()
        set(SMTG_LINUX TRUE)
    endif()
elseif(WIN32)
    if(hasParent)
        set(SMTG_WIN TRUE PARENT_SCOPE)
    else()
        set(SMTG_WIN TRUE)
    endif()
endif()

#-------------------------------------------------------------------------------
# Global Settings
#-------------------------------------------------------------------------------

if(SMTG_WIN)
    option(SMTG_USE_STATIC_CRT "use static CRuntime on Windows (option /MT)" OFF)
endif()

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")

# Export no symbols by default
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

if(SMTG_LINUX)
    set(common_linker_flags "-Wl,--no-undefined")
    set(CMAKE_MODULE_LINKER_FLAGS "${common_linker_flags}" CACHE STRING "Module Library Linker Flags")
    set(CMAKE_SHARED_LINKER_FLAGS "${common_linker_flags}" CACHE STRING "Shared Library Linker Flags")
endif()

# Output directories
# XCode is creating the "Debug/Release" folder on its own and does not need to be added.
if(SMTG_WIN OR (SMTG_MAC AND CMAKE_GENERATOR STREQUAL Xcode))
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
else ()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif()

if(SMTG_MAC)
    if(NOT DEFINED ENV{XCODE_VERSION})
        execute_process(COMMAND xcodebuild -version OUTPUT_VARIABLE XCODE_VERSION ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(REGEX MATCH "Xcode [0-9\\.]+" XCODE_VERSION "${XCODE_VERSION}")
        string(REGEX REPLACE "Xcode ([0-9\\.]+)" "\\1" XCODE_VERSION "${XCODE_VERSION}")
    endif()
    message(STATUS "Building with Xcode version: ${XCODE_VERSION}")
    # macOS defaults
    if(NOT DEFINED ENV{MACOSX_DEPLOYMENT_TARGET})
        if(XCODE_VERSION VERSION_GREATER "7.9")
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "macOS deployment target")
        else()
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8" CACHE STRING "macOS deployment target")
        endif()
        message(STATUS "macOS Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
    if(NOT DEFINED ENV{SDKROOT})
        execute_process(COMMAND xcrun --sdk macosx --show-sdk-path OUTPUT_VARIABLE CMAKE_OSX_SYSROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
endif()

# Generation Settings
#set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE STRING "Generate compile commands" FORCE)

# Put predefined targets like "ALL_BUILD" and "ZERO_CHECK" into a separate folder within the IDE.
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER Predefined)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_compile_options($<$<CONFIG:Debug>:-DDEVELOPMENT=1>)
add_compile_options($<$<CONFIG:Release>:-DRELEASE=1>)
add_compile_options($<$<CONFIG:RelWithDebInfo>:-DRELEASE=1>)

if(SMTG_WIN)
    add_compile_options(/MP)                            # Multi-processor Compilation
    if(NOT ${CMAKE_GENERATOR} MATCHES "ARM")
        add_compile_options($<$<CONFIG:Debug>:/ZI>)     # Program Database for Edit And Continue
    endif()
    if(SMTG_USE_STATIC_CRT)
        add_compile_options($<$<CONFIG:Debug>:/MTd>)    # Runtime Library: /MTd = MultiThreaded Debug Runtime
        add_compile_options($<$<CONFIG:Release>:/MT>)   # Runtime Library: /MT  = MultiThreaded Runtime
    else()
        add_compile_options($<$<CONFIG:Debug>:/MDd>)    # Runtime Library: /MDd = MultiThreadedDLL Debug Runtime
        add_compile_options($<$<CONFIG:Release>:/MD>)   # Runtime Library: /MD  = MultiThreadedDLL Runtime
    endif()
endif()

# Add colors to clang output when using Ninja
# See: https://github.com/ninja-build/ninja/wiki/FAQ
if (UNIX AND CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_GENERATOR STREQUAL "Ninja")
    add_compile_options(-fcolor-diagnostics)
endif()
