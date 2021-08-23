
cmake_minimum_required(VERSION 3.8)

project("Zed")

# SET(CMAKE_CXX_STANDARD 11)

# https://stackoverflow.com/questions/22140520/how-to-enable-assert-in-cmake-release-mode
string( REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string( REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

add_subdirectory("Zed")
