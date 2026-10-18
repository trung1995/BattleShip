# Fails if any file under src/core or src/app includes cocos2d-x (Principle I).
# Usage: cmake -DSOURCE_DIR=<repo root> -P CheckNoEngineIncludes.cmake
if(NOT SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR is not set")
endif()

file(GLOB_RECURSE files
    "${SOURCE_DIR}/src/core/*.h" "${SOURCE_DIR}/src/core/*.cpp"
    "${SOURCE_DIR}/src/app/*.h" "${SOURCE_DIR}/src/app/*.cpp")

set(violations "")
foreach(file IN LISTS files)
    file(STRINGS "${file}" lines REGEX "^[ \t]*#[ \t]*include")
    foreach(line IN LISTS lines)
        string(TOLOWER "${line}" lower)
        if(lower MATCHES "cocos")
            list(APPEND violations "${file}: ${line}")
        endif()
    endforeach()
endforeach()

if(violations)
    string(REPLACE ";" "\n" report "${violations}")
    message(FATAL_ERROR "Engine includes found in core/app:\n${report}")
endif()
message(STATUS "No engine includes in src/core or src/app")
