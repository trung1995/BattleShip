# battleship_core: the rules engine. Standard library only, no cocos2d-x (Principle I).
file(GLOB BATTLESHIP_CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(battleship_core STATIC ${BATTLESHIP_CORE_SOURCES})
target_include_directories(battleship_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(battleship_core PRIVATE battleship_warnings)
