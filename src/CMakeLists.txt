add_library(noita_minidump_injected SHARED
    minidump_injected.cpp
)

target_compile_options(noita_minidump_injected
    PRIVATE /GS-
)

target_link_options(noita_minidump_injected
    PRIVATE /NODEFAULTLIB /NOENTRY /MANIFEST:NO
)

set_target_properties(noita_minidump_injected PROPERTIES
    OUTPUT_NAME "minidump"
)


add_executable(noita_dumper WIN32
    noita_dumper.cpp
)


target_link_libraries(noita_dumper PRIVATE Dbghelp)

install(TARGETS noita_minidump_injected noita_dumper
    RUNTIME
        COMPONENT NoitaMinidump
        DESTINATION "minidump"
)
