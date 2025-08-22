cmake_minimum_required(VERSION 3.22)

project(NoitaMinidump
    VERSION 1.2.0
    DESCRIPTION "Noita Crash Handler"
    HOMEPAGE_URL "https://github.com/dextercd/Noita-Minidump"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)

# Don't add lib suffix when crosscompiling from Linux
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_MODULE_PREFIX "")

include(CTest)

add_subdirectory(src)
install(
    DIRECTORY minidump
    DESTINATION "."
    COMPONENT NoitaMinidump
)

add_subdirectory(packaging)
