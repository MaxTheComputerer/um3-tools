cmake_minimum_required(VERSION 3.16)
cmake_policy(VERSION 3.9)

project(UM3_Tools VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)

configure_file(um3tools_config.h.in um3tools_config.h)

add_executable(UM3_Tools um3tools.cpp)

target_include_directories(UM3_Tools PUBLIC "${PROJECT_BINARY_DIR}")

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 0817715923c9705e68994eb52ef9df3f6845beba) # The commit hash for 1.10.x. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

add_subdirectory(third-party/mx)
target_include_directories(UM3_Tools PRIVATE mx/Sourcecode/include)

target_link_libraries(UM3_Tools PRIVATE
    cpr::cpr
    asound
    pthread
    mx
)
