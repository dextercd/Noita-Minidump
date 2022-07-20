add_library(noita_minidump SHARED
    main.cpp
)

set_target_properties(noita_minidump PROPERTIES
    OUTPUT_NAME "minidump"
)

target_link_libraries(noita_minidump PRIVATE Dbghelp)

install(TARGETS noita_minidump
    EXPORT NoitaDearImGui
    RUNTIME
        COMPONENT NoitaMinidump
        DESTINATION "minidump"
)
