# battleship_app: application logic and engine ports. Standard library + core only (Principle I).
file(GLOB BATTLESHIP_APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(battleship_app STATIC ${BATTLESHIP_APP_SOURCES})
target_include_directories(battleship_app PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(battleship_app PUBLIC battleship_core PRIVATE battleship_warnings)
