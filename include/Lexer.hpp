#pragma once
#ifndef LOX_INTERPRETER_LEXER_HPP
#define LOX_INTERPRETER_LEXER_HPP
#include <cstddef>
#include <vector>
#include <string>

struct LoxToken;
struct LoxScanSession;

class Lexer
{
    Lexer();
    ~Lexer();
    Lexer(const Lexer&) = delete;
    Lexer& operator=(const Lexer&) = delete;
public:

    static Lexer& GetLexerInstance(size_t hash = 0u);

    using OutputHandle = size_t;
    
    // Returns size_t handle 
    OutputHandle ParseScript(std::string sourceStr);
    void GetTokensForHandle(const OutputHandle handle, size_t& numTokens, LoxToken* tokensDest);
    static void SetAllowableErrorCount(size_t count);

private:

    void processLine(std::string_view line, LoxScanSession& session);
    
    void extractDualCharToken(
        std::string_view& line,
        const char firstChar,
        LoxScanSession& session);
    void extractStringLiteral(std::string_view& line, LoxScanSession& session);
    void extractNumericLiteral(std::string_view& line, LoxScanSession& session);
    void extractKeywordOrIdentifier(
        std::string_view& line,
        LoxScanSession& session);

    static size_t s_allowableErrorCount;
};


#endif //!LOX_INTERPRETER_LEXER_HPP
