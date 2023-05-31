cmake_minimum_required(VERSION 3.16)

project(um3tools VERSION 1.2)
set(CMAKE_CXX_STANDARD 17)
set(CPP_VERSION 17)

if(WIN32) # Install dlls in the same directory as the executable on Windows
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

configure_file(um3tools_config.h.in um3tools_config.h)

add_executable(um3tools um3tools.cpp)

target_include_directories(um3tools PUBLIC "${PROJECT_BINARY_DIR}")

include(FetchContent)

# CPR
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 32fd145)
FetchContent_MakeAvailable(cpr)

# MX
add_subdirectory(third-party/mx)
target_include_directories(um3tools PRIVATE mx/Sourcecode/include)

# JSON
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

# Subprocess
FetchContent_Declare(subprocess GIT_REPOSITORY https://github.com/arun11299/cpp-subprocess.git
                                GIT_TAG af23f33)
FetchContent_MakeAvailable(subprocess)

# Argparse
FetchContent_Declare(argparse GIT_REPOSITORY https://github.com/p-ranav/argparse.git
                                GIT_TAG 557948f)
FetchContent_MakeAvailable(argparse)

# Linking libraries

target_link_libraries(um3tools PRIVATE
    cpr::cpr
    mx
    nlohmann_json::nlohmann_json
    subprocess
    argparse
)

# Packaging

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "um3tools")

if(UNIX AND NOT APPLE) # Create a .deb package for Linux
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Max Johnson")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS YES)

    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

elseif(WIN32) # Create a zip file for Windows
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

    file(GLOB DLL_FILES "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/*.dll")
    file(GLOB EXE_FILES "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/*.exe")
    install(FILES ${DLL_FILES} ${EXE_FILES} DESTINATION . COMPONENT um3install)
    
    set(CPACK_COMPONENTS_ALL um3install)
    set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
endif()

include(CPack)
