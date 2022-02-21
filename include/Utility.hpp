#pragma once
#ifndef LOX_UTILITY_HPP
#define LOX_UTILITY_HPP
#include <cstdint>

enum class TokenType : uint32_t;
// returns the literal string representation of the token type:
// used for pretty printing only pretty much
const char* TokenTypeToString(const TokenType& type);

#endif //!LOX_UTILITY_HPP