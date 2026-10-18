# BattleShip: the cocos2d-x presentation layer (thin). Modelled on the cocos2d-x 4.0 cpp-template.
set(APP_NAME BattleShip)

set(COCOS2DX_ROOT_PATH "${PROJECT_SOURCE_DIR}/third_party/cocos2d-x")
if(NOT EXISTS "${COCOS2DX_ROOT_PATH}/cocos/cocos2d.h")
    message(FATAL_ERROR "cocos2d-x not found. Run: git submodule update --init third_party/cocos2d-x "
                        "and install its dependencies (see specs/001-battleship-game/quickstart.md), "
                        "or configure with -DBATTLESHIP_BUILD_GAME=OFF.")
endif()

if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    # The engine's prebuilt third-party libraries are x86_64 only (research R1).
    set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE STRING "" FORCE)
endif()
if(WIN32 AND NOT CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
    message(FATAL_ERROR "cocos2d-x 4.0 ships 32-bit Windows libraries only: configure with -A Win32.")
endif()

# Local fixes for building cocos2d-x 4.0 with current toolchains, applied once to the submodule.
file(GLOB ENGINE_PATCHES "${PROJECT_SOURCE_DIR}/third_party/patches/*.patch")
foreach(patch IN LISTS ENGINE_PATCHES)
    execute_process(COMMAND git apply --reverse --check "${patch}"
                    WORKING_DIRECTORY "${COCOS2DX_ROOT_PATH}" RESULT_VARIABLE already_applied
                    OUTPUT_QUIET ERROR_QUIET)
    if(NOT already_applied EQUAL 0)
        execute_process(COMMAND git apply "${patch}"
                        WORKING_DIRECTORY "${COCOS2DX_ROOT_PATH}" RESULT_VARIABLE apply_result)
        if(NOT apply_result EQUAL 0)
            message(FATAL_ERROR "Could not apply engine patch ${patch}")
        endif()
        message(STATUS "Applied engine patch ${patch}")
    endif()
endforeach()

list(APPEND CMAKE_MODULE_PATH "${COCOS2DX_ROOT_PATH}/cmake/Modules/")
include(CocosBuildSet)
# CocosBuildSet sets C++11; the constitution requires one standard for the whole project (R2).
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos ${ENGINE_BINARY_PATH}/cocos/core)

set(GAME_RES_FOLDER "${PROJECT_SOURCE_DIR}/Resources")
if(APPLE OR WINDOWS)
    cocos_mark_multi_resources(common_res_files RES_TO "Resources" FOLDERS ${GAME_RES_FOLDER})
endif()

file(GLOB_RECURSE GAME_SOURCE CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
list(FILTER GAME_SOURCE EXCLUDE REGEX "/main/")

if(WINDOWS)
    list(APPEND GAME_SOURCE
        main/win32/main.cpp main/win32/main.h main/win32/resource.h main/win32/game.rc
        ${common_res_files})
elseif(MACOSX)
    set(APP_UI_RES main/mac/Icon.icns)
    list(APPEND GAME_SOURCE main/mac/main.cpp main/mac/Info.plist ${APP_UI_RES} ${common_res_files})
endif()

add_executable(${APP_NAME} ${GAME_SOURCE})
target_link_libraries(${APP_NAME} PRIVATE cocos2d battleship_app)
target_include_directories(${APP_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${APP_NAME} SYSTEM PRIVATE "${COCOS2DX_ROOT_PATH}/cocos/audio/include/")

# Warnings-as-errors for project sources only; engine headers come in as SYSTEM (research R5).
get_target_property(cocos_includes cocos2d INTERFACE_INCLUDE_DIRECTORIES)
if(cocos_includes)
    target_include_directories(${APP_NAME} SYSTEM PRIVATE ${cocos_includes})
endif()
target_link_libraries(${APP_NAME} PRIVATE battleship_warnings)

setup_cocos_app_config(${APP_NAME})

if(APPLE)
    set_target_properties(${APP_NAME} PROPERTIES RESOURCE "${APP_UI_RES}")
    set_xcode_property(${APP_NAME} INFOPLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/main/mac/Info.plist")
    # Local builds are unsigned: Xcode's signer rejects the com.apple.provenance attribute that
    # macOS adds to generated resources, and an x86_64 app runs unsigned under Rosetta.
    set_xcode_property(${APP_NAME} CODE_SIGNING_ALLOWED NO)
elseif(WINDOWS)
    cocos_copy_target_dll(${APP_NAME})
    cocos_get_resource_path(APP_RES_DIR ${APP_NAME})
    cocos_copy_target_res(${APP_NAME} LINK_TO ${APP_RES_DIR} FOLDERS ${GAME_RES_FOLDER})
endif()
