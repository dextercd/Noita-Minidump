cmake_minimum_required(VERSION 3.22)

project(NoitaMinidump
    VERSION 1.2.1
    DESCRIPTION "Noita Crash Handler"
    HOMEPAGE_URL "https://github.com/dextercd/Noita-Minidump"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)

# Don't add lib suffix when crosscompiling from Linux
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_MODULE_PREFIX "")

include(FetchContent)
FetchContent_Declare(
    Handtokening_CMake
    GIT_REPOSITORY https://github.com/dextercd/Handtokening-CMake.git
    GIT_TAG bd8a1d54cfa8e84763399163dd09ca27d0a63fb8
)
FetchContent_MakeAvailable(Handtokening_CMake)

include(CTest)

add_subdirectory(src)
install(
    DIRECTORY minidump
    DESTINATION "."
    COMPONENT NoitaMinidump
)

list(APPEND HT_SIGN_PATTERNS
    "\\.dll" description="Noita Crash Detector"
    "\\.exe" description="Noita Crash Dumper"
)
include(Handtokening)

add_subdirectory(packaging)
