cmake_minimum_required(VERSION 3.6)
project(LoxInterpreterBasic)

set(LoxInterpreterBasicSources
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Interpreter.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Interpreter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Lexer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Lexer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/LoxErrors.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/LoxErrors.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/MurmurHash.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Parser.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Parser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Token.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Utility.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Utility.cpp")

set(LoxInterpreterTestSources
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/LexerTests.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/LexerTests.cpp")

add_executable(LoxInterpreterBasic ${LoxInterpreterBasicSources} ${LoxInterpreterTestSources})
target_include_directories(LoxInterpreterBasic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if (MSVC)
    target_compile_options(LoxInterpreterBasic PRIVATE "/std:c++latest" "/JMC")
endif()
