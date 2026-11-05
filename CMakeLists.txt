cmake_minimum_required(VERSION 3.22)
project(agenda_contacte LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3
    GIT_SHALLOW TRUE)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(httplib json)

add_executable(server src/main.cpp)
target_include_directories(server PRIVATE src)
target_link_libraries(server PRIVATE httplib::httplib nlohmann_json::nlohmann_json)
target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
target_link_options(server PRIVATE -static-libstdc++ -static-libgcc)

option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(doctest)

    enable_testing()
    add_executable(tests tests/test_app.cpp)
    target_include_directories(tests PRIVATE src)
    target_link_libraries(tests PRIVATE doctest::doctest nlohmann_json::nlohmann_json)
    target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME unit_tests COMMAND tests)
endif()
