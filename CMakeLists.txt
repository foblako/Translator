cmake_minimum_required(VERSION 3.16)
project(SetLangTranslator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure FetchContent for ANTLR4 runtime
include(FetchContent)

set(ANTLR4_VERSION "4.13.1")

# Use a stable download location for the C++ runtime
FetchContent_Declare(
    antlr4_runtime
    URL "https://github.com/antlr/antlr4/archive/refs/tags/${ANTLR4_VERSION}.tar.gz"
)

# Download ANTLR tool (Java jar) for generating C++ files
set(ANTLR_TOOL_URL "https://www.antlr.org/download/antlr-${ANTLR4_VERSION}-complete.jar")
set(ANTLR_TOOL_JAR "${CMAKE_CURRENT_BINARY_DIR}/antlr-${ANTLR4_VERSION}-complete.jar")

if(NOT EXISTS "${ANTLR_TOOL_JAR}")
    message(STATUS "Downloading ANTLR ${ANTLR4_VERSION} Tool...")
    file(DOWNLOAD "${ANTLR_TOOL_URL}" "${ANTLR_TOOL_JAR}")
endif()

# We need Java to run the ANTLR tool
find_package(Java REQUIRED COMPONENTS Runtime)

FetchContent_MakeAvailable(antlr4_runtime)

# We just need to build the cpp runtime
set(ANTLR4_INSTALL OFF CACHE BOOL "Disable installation of antlr4-runtime")
set(ANTLR_BUILD_CPP_TESTS OFF CACHE BOOL "Disable tests")
set(WITH_DEMO OFF CACHE BOOL "Disable demo")
add_subdirectory(${antlr4_runtime_SOURCE_DIR}/runtime/Cpp ${CMAKE_CURRENT_BINARY_DIR}/antlr4_runtime-build EXCLUDE_FROM_ALL)

# Paths for grammar and generated files
set(GRAMMAR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/grammar")
set(GRAMMAR_FILE "${GRAMMAR_DIR}/DialogueScript.g4")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_DIR}")

set(GENERATED_SRCS
    ${GENERATED_DIR}/DialogueScriptLexer.cpp
    ${GENERATED_DIR}/DialogueScriptParser.cpp
    ${GENERATED_DIR}/DialogueScriptBaseVisitor.cpp
    ${GENERATED_DIR}/DialogueScriptVisitor.cpp
)

set(GENERATED_HEADERS
    ${GENERATED_DIR}/DialogueScriptLexer.h
    ${GENERATED_DIR}/DialogueScriptParser.h
    ${GENERATED_DIR}/DialogueScriptBaseVisitor.h
    ${GENERATED_DIR}/DialogueScriptVisitor.h
)

# Custom command to run ANTLR tool
add_custom_command(
    OUTPUT ${GENERATED_SRCS} ${GENERATED_HEADERS}
    COMMAND ${Java_JAVA_EXECUTABLE} -jar "${ANTLR_TOOL_JAR}"
            -Dlanguage=Cpp -no-listener -visitor -o "${GENERATED_DIR}" "${GRAMMAR_FILE}"
    DEPENDS "${GRAMMAR_FILE}"
    COMMENT "Generating C++ lexer/parser from ${GRAMMAR_FILE}"
)

# Main executable target
set(SRC_FILES
    src/main.cpp
    src/DialogueInterpreter.cpp
)

add_executable(main ${SRC_FILES} ${GENERATED_SRCS})

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GENERATED_DIR}
    ${antlr4_runtime_SOURCE_DIR}/runtime/Cpp/runtime/src
)

target_link_libraries(main PRIVATE antlr4_shared)
