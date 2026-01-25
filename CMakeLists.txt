
cmake_minimum_required(VERSION 3.10)

project(netsim VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

add_executable(netsim 
    src/main.cpp
    src/factory.cpp
    src/helpers.cpp
    src/nodes.cpp
    src/package.cpp
    src/storage_types.cpp
        src/tests.cpp
)
target_include_directories(netsim PUBLIC ${CMAKE_SOURCE_DIR}/include)

# linkuje, testy, ja mam msys mingw64, jak wam wywala cmake to poprostu to zakomentujcie
target_include_directories(netsim PRIVATE /mingw64/include)
target_link_libraries(netsim PRIVATE gtest gtest_main pthread)
