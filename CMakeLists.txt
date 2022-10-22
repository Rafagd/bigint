cmake_minimum_required(VERSION 3.10)

project(bigint VERSION 0.0.1 DESCRIPTION "A bigint implementation in C")
include(GNUInstallDirs)
include(CTest)

add_custom_target(test_memcheck
    COMMAND ${CMAKE_CTEST_COMMAND} 
    --force-new-ctest-process --test-action memcheck
    COMMAND cat "${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.*.log"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

add_compile_options(-Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wconversion)

configure_file(bigint.pc.in bigint.pc @ONLY)

add_library(bigint_lib
    src/array.c
    src/bigint.c)
target_include_directories(bigint_lib PRIVATE include)

set_target_properties(bigint_lib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER include/bigint.h)

enable_testing()


find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(Check REQUIRED IMPORTED_TARGET check)

add_executable(array_tests_exe tests/array.c)
target_include_directories(array_tests_exe PRIVATE include)
target_link_libraries(array_tests_exe bigint_lib PkgConfig::Check Threads::Threads)

add_executable(bigint_tests_exe tests/bigint.c)
target_include_directories(bigint_tests_exe PRIVATE include)
target_link_libraries(bigint_tests_exe bigint_lib PkgConfig::Check Threads::Threads)

add_test(array_tests array_tests_exe)
add_test(bigint_tests bigint_tests_exe)

install(TARGETS bigint_lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES
    ${CMAKE_BINARY_DIR}/bigint.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

