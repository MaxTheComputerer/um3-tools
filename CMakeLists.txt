cmake_minimum_required(VERSION 3.16)
cmake_policy(VERSION 3.9)

project(um3tools VERSION 0.1)
set(CMAKE_CXX_STANDARD 17)
set(CPP_VERSION 17)

configure_file(um3tools_config.h.in um3tools_config.h)

add_executable(um3tools um3tools.cpp)

target_include_directories(um3tools PUBLIC "${PROJECT_BINARY_DIR}")

include(FetchContent)

# CPR
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 0817715923c9705e68994eb52ef9df3f6845beba) # The commit hash for 1.10.x. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

# MX
add_subdirectory(third-party/mx)
target_include_directories(um3tools PRIVATE mx/Sourcecode/include)

# JSON
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

# Subprocess
FetchContent_Declare(subprocess GIT_REPOSITORY https://github.com/arun11299/cpp-subprocess.git GIT_TAG master)
FetchContent_MakeAvailable(subprocess)

# Argparse
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

# Linking libraries

target_link_libraries(um3tools PRIVATE
    cpr::cpr
    mx
    nlohmann_json::nlohmann_json
    subprocess
    argparse
)

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Max Johnson")
endif()


install(TARGETS um3tools DESTINATION bin)
install(FILES $<TARGET_FILE:cpr> DESTINATION lib)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_GENERATOR}")
include(CPack)
