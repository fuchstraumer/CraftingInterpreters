#include "Utility.hpp"
#include "Token.hpp"
#include <array>
#include <string>

std::array<std::string, static_cast<size_t>(TokenType::TokenCount)> tokenStrings
{
    "Invalid",
    "(",
    ")",
    "{",
    "}",
    ",",
    ".",
    "-",
    "+",
    ";",
    "/",
    "*"
    "!",
    "!=",
    "=",
    "==",
    ">=",
    "<",
    "<="
    "Identifier",
    "StringLiteral",
    "NumberLiteral",
    "//",
    "CommentString",
    "EOF",
    "and",
    "class",
    "else",
    "false",
    "fun",
    "for",
    "if",
    "Nil",
    "or",
    "print",
    "return",
    "super",
    "this",
    "true",
    "var",
    "while"
};

const char* TokenTypeToString(const TokenType& type)
{
    return tokenStrings[static_cast<size_t>(type)].c_str();
}