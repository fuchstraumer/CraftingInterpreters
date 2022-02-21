#pragma once
#ifndef LOX_TOKEN_HPP
#define LOX_TOKEN_HPP
#include <cstdint>
#include <limits>
#include <string_view>

enum class TokenType : uint32_t
{
    Invalid = 0,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Comma,
    Dot,
    Minus,
    Plus,
    Semicolon,
    Slash,
    Star,
    LogicalNot,
    LogicalNotEqual,
    Equal,
    EqualEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    // literals
    Identifier,
    StringLiteral,
    NumberLiteral,
    // special items
    CommentBegin,
    CommentString,
    EndOfFile,
    // keywords
    KeywordsBeginRange,
    And,
    Class,
    Else,
    False,
    Fun,
    For,
    If,
    Nil,
    Or,
    Print,
    Return,
    Super,
    This,
    True,
    Var,
    While,
    KeywordsEndRange = While,
    KeywordCount = KeywordsEndRange - KeywordsBeginRange,
    TokenCount = KeywordsEndRange
};

struct LoxToken
{
    explicit LoxToken(TokenType _type, size_t _line, size_t _offset) :
        type(_type), line(_line), offset(_offset) {}
    explicit LoxToken(TokenType _type, size_t _line, size_t _offset, std::string_view sv) :
        type(_type), line(_line), offset(_offset), strLiteral(sv) {}
    explicit LoxToken(TokenType t, size_t _line, size_t _offset, float num) :
        type(t), line(_line), offset(_offset), numericLiteral(num) {}
    
    // noexcept things so this is maximally cheap to swap and move around...
    // might actually help us, lol
    LoxToken() noexcept = default;
    ~LoxToken() noexcept = default;
    LoxToken(const LoxToken&) noexcept = default;
    LoxToken(LoxToken&&) noexcept = default;
    LoxToken& operator=(const LoxToken&) noexcept = default;
    LoxToken& operator=(LoxToken&&) noexcept = default;

    TokenType type = TokenType::Invalid;
    size_t line = 0;
    // distance (in characters) to this token in the line
    size_t offset = 0;
    std::string_view strLiteral{};
    float numericLiteral = std::numeric_limits<float>::max();
};

#endif //!LOX_TOKEN_HPP
