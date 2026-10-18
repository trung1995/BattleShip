include(${PROJECT_SOURCE_DIR}/cmake/GoogleTest.cmake)

file(GLOB BATTLESHIP_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/*.cpp")

add_executable(battleship_tests ${BATTLESHIP_TEST_SOURCES})
target_include_directories(battleship_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/fakes")
target_link_libraries(battleship_tests PRIVATE
    battleship_app GTest::gtest_main GTest::gmock battleship_warnings)
target_compile_definitions(battleship_tests PRIVATE
    BATTLESHIP_RESOURCES_DIR="${PROJECT_SOURCE_DIR}/Resources")
gtest_discover_tests(battleship_tests DISCOVERY_TIMEOUT 60)

file(GLOB BATTLESHIP_SIM_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/sim/*.cpp")
if(BATTLESHIP_SIM_SOURCES)
    add_executable(battleship_sim ${BATTLESHIP_SIM_SOURCES})
    target_include_directories(battleship_sim PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/fakes")
    target_link_libraries(battleship_sim PRIVATE battleship_core GTest::gtest_main battleship_warnings)
    gtest_discover_tests(battleship_sim PROPERTIES LABELS sim DISCOVERY_TIMEOUT 60)
endif()

add_test(NAME architecture.no_engine_in_core_or_app
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
            -P ${PROJECT_SOURCE_DIR}/cmake/CheckNoEngineIncludes.cmake)
