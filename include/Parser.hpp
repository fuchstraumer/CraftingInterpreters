#pragma once
#ifndef LOX_PARSER_HPP
#define LOX_PARSER_HPP
#include "Expression.hpp"
#include "LoxErrors.hpp"
#include <vector>
#include <stdexcept>

struct ParseError : public std::runtime_error
{
    ParseError(LoxCompilerErrorCode ec, LoxToken token) noexcept;
    LoxCompilerErrorCode errorCode;
    LoxToken token;
};

class Parser
{
public:
    Parser(const std::vector<LoxToken>& tokens);
    ~Parser() = default;
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

private:
    std::vector<LoxToken> tokens;
    using TokenIter = decltype(tokens)::const_iterator;

    Expression expression(TokenIter iter) const;
    Expression equality(TokenIter iter) const;
    Expression comparison(TokenIter iter) const;
    Expression term(TokenIter iter) const;
    Expression factor(TokenIter iter) const;
    Expression unary(TokenIter iter) const;
    Expression primary(TokenIter iter) const;

    bool isAtEnd(TokenIter iter) const;
    const LoxToken& advance(TokenIter iter) const noexcept;
    const LoxToken& previous(TokenIter iter) const noexcept;
    const LoxToken& peek(TokenIter iter) const noexcept;
    bool match(const std::vector<TokenType>& types, TokenIter iter) const noexcept;
    void consume(TokenType type, std::string failureString) const;
    size_t currentToken = 0u;
};

#endif //!LOX_PARSER_HPP