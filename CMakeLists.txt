cmake_minimum_required(VERSION 3.16)
project(search_tree_churn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(CTest)
add_library(churn_trees STATIC src/structA/hibbard_bst.cpp src/structB/scapegoat_tree.cpp)
target_include_directories(churn_trees PUBLIC
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/src/structA"
    "${PROJECT_SOURCE_DIR}/src/structB"
    "${PROJECT_SOURCE_DIR}/adversary"
)

function(tree_program target source)
    add_executable(${target} "${source}")
    target_link_libraries(${target} PRIVATE churn_trees)
endfunction()

function(checked_test target source)
    tree_program(${target} "${source}")
    # Keep assertions active in the focused suites, including in Release builds.
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
    add_test(NAME ${target} COMMAND ${target})
endfunction()

tree_program(churn_simulator app/churn_simulator.cpp)
tree_program(tree_demo app/tree_demo.cpp)
tree_program(bst_baseline bench/bst_baseline.cpp)
tree_program(beginner_demo bench/beginner_demo.cpp)

# The analyst and oracle branches contribute these files when merged.
if(EXISTS "${PROJECT_SOURCE_DIR}/bench/churn_benchmark.cpp")
    tree_program(churn_benchmark bench/churn_benchmark.cpp)
    add_executable(analyst_study bench/analyst_study.cpp)
    add_executable(predict_rate bench/predict_rate.cpp)
endif()

if(BUILD_TESTING)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME repository_layout COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/test_repository_layout.py")
    checked_test(structure_a src/tests/test_struct_A.cpp)
    checked_test(structure_b src/tests/test_struct_B.cpp)
    checked_test(structure_a_churn src/tests/test_struct_A_churn.cpp)
    add_test(NAME simulator_csv COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/test_churn_simulator.py" "$<TARGET_FILE:churn_simulator>")

    if(EXISTS "${PROJECT_SOURCE_DIR}/oracle/differential.cpp")
        checked_test(sorted_array oracle/test_sorted_array.cpp)
        checked_test(oracle_invariants oracle/test_invariants.cpp)
        checked_test(oracle_d4 oracle/test_d4.cpp)
        tree_program(differential oracle/differential.cpp)
        tree_program(shrink oracle/shrink.cpp)
        add_test(NAME oracle_million COMMAND differential mixed 20261117 1000000)
        add_test(NAME oracle_replay COMMAND differential replay
            "${PROJECT_SOURCE_DIR}/oracle/shrink_fixture.txt")
    else()
        message(STATUS "Oracle sources are pending team integration")
    endif()
endif()
