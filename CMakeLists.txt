cmake_minimum_required(VERSION 3.14)
project(bestreviewer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    TLS_VERIFY OFF
)
FetchContent_MakeAvailable(googletest)

include_directories(include)

enable_testing()

add_executable(TunerTest test/TunerTest.cpp)
target_link_libraries(TunerTest GTest::gtest_main GTest::gmock)

add_executable(TVControllerTest test/TVControllerTest.cpp)
target_link_libraries(TVControllerTest GTest::gtest_main GTest::gmock)

# Golden Master (TextTest-style approval) regression tests
set(TV_GOLDEN_APPROVED_DIR "${CMAKE_SOURCE_DIR}/test/golden/approved")
set(TV_GOLDEN_RECEIVED_DIR "${CMAKE_SOURCE_DIR}/test/golden/received")

add_executable(TVControllerGoldenTest test/TVControllerGoldenTest.cpp)
target_compile_definitions(TVControllerGoldenTest PRIVATE
    GOLDEN_APPROVED_DIR="${TV_GOLDEN_APPROVED_DIR}"
    GOLDEN_RECEIVED_DIR="${TV_GOLDEN_RECEIVED_DIR}")
target_link_libraries(TVControllerGoldenTest GTest::gtest_main)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
    target_link_libraries(TVControllerGoldenTest stdc++fs)
endif()

include(GoogleTest)
gtest_discover_tests(TunerTest)
gtest_discover_tests(TVControllerTest)
gtest_discover_tests(TVControllerGoldenTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Regenerate approved/*.approved.txt after intentional behavior changes
add_custom_target(update_golden
    COMMAND ${CMAKE_COMMAND} -E env TV_UPDATE_GOLDEN=1
        $<TARGET_FILE:TVControllerGoldenTest>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS TVControllerGoldenTest
    COMMENT "Rewrite test/golden/approved/*.approved.txt from current output")
