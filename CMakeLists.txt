cmake_minimum_required(VERSION 3.24)

project(traffic_microsim
    VERSION 0.1.0
    DESCRIPTION "Traffic microsimulation skeleton in C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
include(CTest)

set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(SFML)

add_library(traffic_sim_core
    src/domain/ScenarioFactory.cpp
    src/domain/SimulationState.cpp
    src/simulation/ScenarioConfigIO.cpp
    src/simulation/SignalSystem.cpp
    src/simulation/SpawnSystem.cpp
    src/simulation/VehicleBehaviorSystem.cpp
    src/simulation/LifecycleSystem.cpp
    src/simulation/SimulationEngine.cpp
)

target_include_directories(traffic_sim_core
    PUBLIC
        include
)

if(MSVC)
    target_compile_options(traffic_sim_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(traffic_sim_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(traffic_microsim
    src/main.cpp
    src/app/Application.cpp
    src/core/AssetRegistry.cpp
    src/core/Logger.cpp
    src/rendering/SceneRenderer.cpp
    src/ui/ControlPanel.cpp
)

target_include_directories(traffic_microsim
    PRIVATE
        include
)

target_link_libraries(traffic_microsim
    PRIVATE
        traffic_sim_core
        sfml-graphics
        sfml-window
        sfml-system
)

function(configure_runtime_bundle target_name)
    get_filename_component(COMPILER_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${target_name}>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/scenarios"
            "$<TARGET_FILE_DIR:${target_name}>/scenarios"
    )

    if(MINGW)
        foreach(RUNTIME_DLL
            libstdc++-6.dll
            libstdc++_64-6.dll
            libgcc_s_seh-1.dll
            libgcc_s_seh_64-1.dll
            libgcc_s_dw2-1.dll
            libgcc_s_sjlj-1.dll
            winpthread-1.dll
            libwinpthread-1.dll
            libwinpthread_64-1.dll)
            if(EXISTS "${COMPILER_BIN_DIR}/${RUNTIME_DLL}")
                add_custom_command(TARGET ${target_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${COMPILER_BIN_DIR}/${RUNTIME_DLL}"
                        "$<TARGET_FILE_DIR:${target_name}>/${RUNTIME_DLL}"
                )
            endif()
        endforeach()
    endif()
endfunction()

configure_runtime_bundle(traffic_microsim)

if(MSVC)
    target_compile_options(traffic_microsim PRIVATE /W4 /permissive-)
else()
    target_compile_options(traffic_microsim PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    add_executable(traffic_domain_tests
        tests/test_main.cpp
        tests/traffic_light_tests.cpp
        tests/vehicle_rules_tests.cpp
        tests/simulation_state_tests.cpp
        tests/simulation_engine_tests.cpp
        tests/scenario_config_tests.cpp
    )

    target_include_directories(traffic_domain_tests
        PRIVATE
            include
            tests
    )

    target_link_libraries(traffic_domain_tests
        PRIVATE
            traffic_sim_core
    )

    configure_runtime_bundle(traffic_domain_tests)

    if(MSVC)
        target_compile_options(traffic_domain_tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(traffic_domain_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME traffic_domain_tests COMMAND traffic_domain_tests)
    set_tests_properties(traffic_domain_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()
