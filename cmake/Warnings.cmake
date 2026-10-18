# Warnings are errors for project code only; engine targets keep their own flags (research R5).
add_library(battleship_warnings INTERFACE)

if(MSVC)
    target_compile_options(battleship_warnings INTERFACE /W4 /WX /permissive- /utf-8)
else()
    target_compile_options(battleship_warnings INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror)
endif()
