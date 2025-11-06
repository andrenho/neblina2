#-------------------------
# General configuration
#------------------------

cmake_minimum_required(VERSION 3.16)
project(neblina
    VERSION 0.0.2
    DESCRIPTION "A tiny multi-protocol server"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDES src os)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_COMPILE_OPTIONS_NO_RTTI ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

#-------------------------
# Libraries
#------------------------

if(NOT WIN32)
    find_package(Threads)
endif ()
find_package(OpenSSL COMPONENTS SSL REQUIRED)


#-------------------------
# Compiler specific code
#------------------------

if(MSVC)
    include(cmake/msvc.cmake)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    include(cmake/clang.cmake)
    include(cmake/common.cmake)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    include(cmake/gcc.cmake)
    include(cmake/common.cmake)
endif()

include_directories(${INCLUDES} SYSTEM src/contrib)

#-------------------------
# Determine compilation files
#------------------------

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        os/*.h
        src/*.cc src/*.hh)

if(WIN32)
    file(GLOB_RECURSE ADDITIONAL_FILES CONFIGURE_DEPENDS *.win32.cpp)
elseif(APPLE)
    file(GLOB_RECURSE ADDITIONAL_FILES CONFIGURE_DEPENDS *.posix.cpp *.bsd.cpp)
elseif(CMAKE_SYSTEM_NAME MATCHES "BSD")
    file(GLOB_RECURSE ADDITIONAL_FILES CONFIGURE_DEPENDS *.posix.cpp *.bsd.cpp)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    file(GLOB_RECURSE ADDITIONAL_FILES CONFIGURE_DEPENDS *.posix.cpp *.linux.cpp)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(SRC_FILES "${SRC_FILES};${ADDITIONAL_FILES}")

#-------------------------
# Main build
#------------------------

# main executable
add_executable(${PROJECT_NAME} main.cc ${SRC_FILES})
add_executable(${PROJECT_NAME}-echo EXCLUDE_FROM_ALL demo/echo.cc ${SRC_FILES})
add_executable(${PROJECT_NAME}-echossl EXCLUDE_FROM_ALL demo/echossl.cc ${SRC_FILES})

# tests
file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS tests/*.cc tests/doctest.h)
add_executable(${PROJECT_NAME}-tests EXCLUDE_FROM_ALL ${TEST_FILES} ${SRC_FILES})
add_executable(${PROJECT_NAME}-load-test EXCLUDE_FROM_ALL loadtest/loadtest.cc ${SRC_FILES})

# with sanitizers
add_executable(${PROJECT_NAME}-tests-san-leaks EXCLUDE_FROM_ALL ${TEST_FILES} ${SRC_FILES})
add_executable(${PROJECT_NAME}-tests-san-threads EXCLUDE_FROM_ALL ${TEST_FILES} ${SRC_FILES})
target_link_options(${PROJECT_NAME}-tests-san-leaks PRIVATE -fsanitize=address -fsanitize=undefined)
target_link_options(${PROJECT_NAME}-tests-san-threads PRIVATE -fsanitize=thread -fPIE)

#TSAN_OPTIONS="halt_on_error=0 suppressions=xxx.supp"
#ASAN_OPTIONS="detect_leaks=1:halt_on_error=1"
#leaks --atExit -- ./xxxx"

#add_executable(tests-infloop EXCLUDE_FROM_ALL tests/watchdog/infloop.c)
#add_executable(tests-error EXCLUDE_FROM_ALL tests/watchdog/error.c)
#add_executable(tests-nonrecoverable EXCLUDE_FROM_ALL tests/watchdog/nonrecoverable.c)
#add_dependencies(${PROJECT_NAME}-tests tests-infloop tests-error tests-nonrecoverable)

# add special targets
set(FINAL_TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-tests ${PROJECT_NAME}-echo ${PROJECT_NAME}-echossl ${PROJECT_NAME}-load-test)
include(cmake/special-targets.cmake)

# add common things to all final targets
foreach (TARGET ${FINAL_TARGETS})
    target_link_libraries(${TARGET} PRIVATE OpenSSL::SSL)

    if (NOT MSVC)
        # add POSIX libraries
        target_link_libraries(${TARGET} PRIVATE Threads::Threads)

        # strip executable
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
            add_custom_command(TARGET ${TARGET} POST_BUILD
                    COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${TARGET}>
            )
        endif ()
    endif ()
endforeach ()
