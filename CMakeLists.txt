cmake_minimum_required(VERSION 3.22)

project(NoitaMinidump
    VERSION 1.1.3
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)

include(CTest)

add_subdirectory(src)
install(
    DIRECTORY minidump
    DESTINATION "."
    COMPONENT NoitaMinidump
)

add_subdirectory(packaging)
