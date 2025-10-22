cmake_minimum_required(VERSION 3.20)
project(fort VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wcast-align
        -Wcast-qual
        -Wwrite-strings
        -Wundef
        -Wunreachable-code
        -Wformat=2
        -Wstrict-prototypes
        -Wmissing-prototypes
        -Wold-style-definition
        -Wnull-dereference
        -Wdouble-promotion
        -Wfloat-equal
        -Wpointer-arith
        -g
    )
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Directories
set(FORT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FORT_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

# Sanitizer options
option(FORT_ASAN_ENABLED "Enable AddressSanitizer" OFF)
option(FORT_MSAN_ENABLED "Enable MemorySanitizer" OFF)
option(FORT_UBSAN_ENABLED "Enable UndefinedBehaviorSanitizer" OFF)

# Check that only one sanitizer is enabled at a time
set(SANITIZER_COUNT 0)
if(FORT_ASAN_ENABLED)
    math(EXPR SANITIZER_COUNT "${SANITIZER_COUNT} + 1")
endif()
if(FORT_MSAN_ENABLED)
    math(EXPR SANITIZER_COUNT "${SANITIZER_COUNT} + 1")
endif()
if(FORT_UBSAN_ENABLED)
    math(EXPR SANITIZER_COUNT "${SANITIZER_COUNT} + 1")
endif()

if(SANITIZER_COUNT GREATER 1)
    message(FATAL_ERROR "Only one sanitizer can be enabled at a time")
endif()

# Sanitizer flags
set(SANITIZER_FLAGS "")
set(SANITIZER_LINK_FLAGS "")

if(FORT_ASAN_ENABLED)
    list(APPEND SANITIZER_FLAGS "-fsanitize=address" "-fno-omit-frame-pointer")
    list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=address")
    message(STATUS "AddressSanitizer enabled")
endif()

if(FORT_MSAN_ENABLED)
    list(APPEND SANITIZER_FLAGS "-fsanitize=memory" "-fno-omit-frame-pointer" "-fsanitize-memory-track-origins")
    list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=memory")
    message(STATUS "MemorySanitizer enabled")
endif()

if(FORT_UBSAN_ENABLED)
    list(APPEND SANITIZER_FLAGS "-fsanitize=undefined" "-fno-omit-frame-pointer")
    list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=undefined")
    message(STATUS "UndefinedBehaviorSanitizer enabled")
endif()

# Fort compiler executable
add_executable(fort
    ${FORT_SRC_DIR}/fort.c
    ${FORT_SRC_DIR}/lex.c
)

# Apply sanitizer flags if any are enabled
if(SANITIZER_FLAGS)
    target_compile_options(fort PRIVATE ${SANITIZER_FLAGS})
    target_link_options(fort PRIVATE ${SANITIZER_LINK_FLAGS})
endif()

file(GLOB_RECURSE 
    HDR_FILES
    "${FORT_SRC_DIR}/*.h"
    "${FORT_TEST_DIR}/*.h"
)

add_custom_target(lint
  COMMAND /usr/bin/clang-tidy -p "${CMAKE_BINARY_DIR}" ${HDR_FILES}
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-clang-tidy.py -allow-no-checks -p=${CMAKE_BINARY_DIR}
  COMMENT "Running clang-tidy..."
)

add_custom_target(fmt
  COMMAND git clang-format --diff
  COMMENT "Running clang-format..."
)

add_custom_target(iwyu
  COMMAND iwyu_tool -p ${CMAKE_BINARY_DIR} --
  -Xiwyu --check_also=${FORT_SRC_DIR}/*.h
  -Xiwyu --check_also=${FORT_TEST_DIR}/*.h
  -Xiwyu --update_comments
  -Xiwyu --error=1
  COMMENT "Running iwyu..."
)

add_custom_target(iwyu-fix
  COMMAND iwyu_tool -p ${CMAKE_BINARY_DIR} --
  -Xiwyu --check_also=${FORT_SRC_DIR}/*.h
  -Xiwyu --check_also=${FORT_TEST_DIR}/*.h
  -Xiwyu --update_comments
  | fix_include --update_comments --nosafe_headers --noreorder
  COMMENT "Running iwyu and applying suggestions..."
)

# Enable testing
enable_testing()
add_subdirectory(test)
