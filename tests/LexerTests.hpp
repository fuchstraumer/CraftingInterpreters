#pragma once
#ifndef LOX_LEXER_TESTS_HPP
#define LOX_LEXER_TESTS_HPP
#include <string_view>

// Based on the first chapter of Crafting Interpreters, checks for failure and functionality in all key
// elements. Writes out results to a string that can be printed.
std::string_view RunBasicLexerTests();

#endif //!LOX_LEXER_TESTS_HPP
