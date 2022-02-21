#include "Lexer.hpp"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <charconv>
#include <algorithm>
#include <iostream>
#include "MurmurHash.hpp"
#include "LoxErrors.hpp"
#include "Token.hpp"

namespace
{
    constexpr size_t k_maxErrorsInScanSession = 16u;
    static const std::unordered_map<TokenType, size_t> k_keywordLengthMap
    {
        { TokenType::And, 3u },
        { TokenType::Class, 4u },
        { TokenType::Else, 4u },
        { TokenType::False, 5u },
        { TokenType::For, 3u },
        { TokenType::Fun, 3u },
        { TokenType::If, 2u },
        { TokenType::Nil, 3u },
        { TokenType::Or, 2u },
        { TokenType::Print, 5u },
        { TokenType::Return, 6u },
        { TokenType::Super, 5u },
        { TokenType::This, 4u },
        { TokenType::True, 4u },
        { TokenType::Var, 3u },
        { TokenType::While, 5u }
    };

    static const std::unordered_map<char, TokenType> singleCharLexemes
    {
        { '(', TokenType::LeftParen },
        { ')', TokenType::RightParen },
        { '{', TokenType::LeftBrace },
        { '}', TokenType::RightBrace },
        { ',', TokenType::Comma },
        { '.', TokenType::Dot },
        { '-', TokenType::Minus },
        { '+', TokenType::Plus },
        { ';', TokenType::Semicolon },
        { '*', TokenType::Star }
    };

    static const std::unordered_set<char> dualCharPrefixes
    {
        '!', '=', '<', '>', '/' 
    };

    constexpr size_t k_keywordStrArraySz = static_cast<size_t>(TokenType::KeywordCount);
    static const std::string k_keywordStrings[k_keywordStrArraySz]
    {
        "and",
        "class",
        "else",
        "false",
        "for",
        "fun",
        "if",
        "nil",
        "or",
        "print",
        "return",
        "super",
        "this",
        "true",
        "var",
        "while"
    };

    static const std::unordered_map<std::string_view, TokenType> k_keywordTokenTypeMap
    {
        { k_keywordStrings[0], TokenType::And },
        { k_keywordStrings[1], TokenType::Class },
        { k_keywordStrings[2], TokenType::Else },
        { k_keywordStrings[3], TokenType::False },
        { k_keywordStrings[4], TokenType::For },
        { k_keywordStrings[5], TokenType::Fun },
        { k_keywordStrings[6], TokenType::If },
        { k_keywordStrings[7], TokenType::Nil },
        { k_keywordStrings[8], TokenType::Or },
        { k_keywordStrings[9], TokenType::Print },
        { k_keywordStrings[10], TokenType::Return },
        { k_keywordStrings[11], TokenType::Super },
        { k_keywordStrings[12], TokenType::This },
        { k_keywordStrings[13], TokenType::True },
        { k_keywordStrings[14], TokenType::Var },
        { k_keywordStrings[15], TokenType::While }
    };

    constexpr bool IsNumericDigit(const char c) noexcept
    {
        return c >= '0' && c <= '9';
    }

    constexpr bool IsAlpha(const char c) noexcept
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'); 
    }

    constexpr bool IsAlphaNumeric(const char c) noexcept
    {
        return IsAlpha(c) || IsNumericDigit(c);
    }

    constexpr bool IsKeywordTokenType(const TokenType type)
    {
        // TokenType::KeywordsEndRange is itself a valid value, it's "while"
        return (type > TokenType::KeywordsBeginRange) &&
            (type <= TokenType::KeywordsEndRange);
    }
}

struct LoxScannerErrorInfo
{
    LoxCompilerErrorCode errorCode = static_cast<LoxCompilerErrorCode>(0);
    size_t line = 0;
    size_t offset = 0;
    std::string_view str;
};

struct LoxScanSession
{
    size_t currentLineNumber = 0;
    size_t offsetInCurrentLine = 0;
    size_t line = 1;
    std::string sourceText;
    std::string_view sourceTextView;
    std::vector<LoxToken> tokens;
    std::vector<LoxScannerErrorInfo> errors;
    
    // just adds EOF token
    void finalize()
    {
        tokens.emplace_back(TokenType::EndOfFile, currentLineNumber, 0);
    }

    // add simple single or dual character token
    void addToken(TokenType type, size_t tokenLen, std::string_view& sv)
    {
        tokens.emplace_back(type, currentLineNumber, offsetInCurrentLine);
        offsetInCurrentLine += tokenLen;
        sv.remove_prefix(tokenLen);
    }

    // todo: add support for multi-line comments using /**/
    // will be trickier since those can just fill small chunks of lines
    void addSingleLineCommentToken(std::string_view& currLineView)
    {
        // extract comment from current line
        const size_t commentLen = currLineView.size() - 2u;

        // remove the '//' comment prefix first
        currLineView.remove_prefix(2u);
        tokens.emplace_back(TokenType::CommentBegin, currentLineNumber, offsetInCurrentLine);
        offsetInCurrentLine += 2u;

        // if there's a space, remove it too
        if (currLineView[0] == ' ')
        {
            currLineView.remove_prefix(1u);
            offsetInCurrentLine += 1u;
        }
    
        tokens.emplace_back(TokenType::CommentString, currentLineNumber, offsetInCurrentLine, currLineView);
        // erase whatever is left of the current line, since comments
        // mean nothing else can follow them (if something does, user's loss)
        currLineView.remove_prefix(currLineView.length());
    }

    void addStrLiteralToken(
        std::string_view& currLine,
        std::string_view literal)
    {
        // update offset by +1 to account for the opening quote
        offsetInCurrentLine += 1u;
        currLine.remove_prefix(1u); 
        
        tokens.emplace_back(TokenType::StringLiteral, currentLineNumber, offsetInCurrentLine, literal);

        // offset for this is the length of the literal +1 for end quote
        const size_t offsetAmount = literal.size() + 1u;
        offsetInCurrentLine += offsetAmount;
        // remove by offset amount
        currLine.remove_prefix(offsetAmount);
    }

    void addNumLiteralToken(
        std::string_view& currLine,
        float value,
        size_t literalLen)
    {
        tokens.emplace_back(TokenType::NumberLiteral, currentLineNumber, offsetInCurrentLine, value);
        offsetInCurrentLine += literalLen;
        currLine.remove_prefix(literalLen);
    }

    void addKeywordToken(
        std::string_view& currLine,
        const TokenType type)
    {
        tokens.emplace_back(type, currentLineNumber, offsetInCurrentLine);
        const size_t kwLength = k_keywordLengthMap.at(type);
        currLine.remove_prefix(kwLength);
        offsetInCurrentLine += kwLength;
    }

    void addIdentifierToken(
        std::string_view& currLine,
        std::string_view identifier)
    {
        tokens.emplace_back(TokenType::Identifier, currentLineNumber, offsetInCurrentLine, identifier);
        currLine.remove_prefix(identifier.length());
        offsetInCurrentLine += identifier.length();
    }

    void addError(LoxCompilerErrorCode ec, std::string_view line)
    {
        errors.emplace_back(ec, currentLineNumber, offsetInCurrentLine, line);
    }
};

static std::unordered_map<Lexer::OutputHandle, LoxScanSession> sessions;

std::string_view readLine(LoxScanSession& input)
{
    std::string_view resultView = std::string_view{};
    if (input.sourceTextView.empty())
    {
        return resultView;
    }

    const size_t firstNewline = input.sourceTextView.find_first_of('\n');
    const size_t firstReturn = input.sourceTextView.find_first_of('\r');
    ++input.line;

    
    // Catch cases where current line only contains a newline character or two
    if (firstNewline == 0 || firstReturn == 0)
    {
        // when at 0, the current line is just empty! return an empty string view.
        input.sourceTextView.remove_prefix(1u);
        return resultView;
    }
    else if (firstReturn != std::string::npos)
    {
        resultView = input.sourceTextView.substr(0, firstReturn);
        input.sourceTextView.remove_prefix(firstNewline + 1);
    }
    else
    {
        resultView = input.sourceTextView.substr(0, firstNewline);
        input.sourceTextView.remove_prefix(firstNewline + 1);
    }

    return resultView;
}

Lexer::Lexer() {}

Lexer::~Lexer() {}

Lexer& Lexer::GetLexerInstance(size_t hash /*= 0u*/)
{
    static Lexer staticLexer;
    return staticLexer;
}

size_t Lexer::ParseScript(std::string sourceStr)
{
    LoxScanSession session;
    session.sourceText = std::move(sourceStr);
    session.sourceTextView = session.sourceText;

    size_t sessionKey =
        MurmurHash2(session.sourceTextView.data(),
                    session.sourceTextView.length(), 1u);

    // runs as long as there's text left to consume within
    // the source text view
    while (!session.sourceTextView.empty())
    {

        std::string_view currentLine = readLine(session);
        if (currentLine.empty())
        {
            // empty lines still affect line count, incorrect line
            // count would result in confusing debugging
            session.currentLineNumber += 1;
            continue;
        }

        processLine(currentLine, session);

        if (session.errors.size() > k_maxErrorsInScanSession)
        {
            throw std::runtime_error("Surpassed max error count");
        }

        session.currentLineNumber += 1;
        // reset offset in line for new line
        session.offsetInCurrentLine = 0;
    }

    session.finalize();

    auto sessionEmplaced = sessions.emplace(sessionKey, std::move(session));
    return sessionKey;
}

void Lexer::GetTokensForHandle(const Lexer::OutputHandle handle, size_t& numTokens, LoxToken* tokens)
{
    auto sessionIter = sessions.find(handle);
    if (sessionIter != sessions.end())
    {
        numTokens = sessionIter->second.tokens.size();
        if (tokens != nullptr)
        {
            std::copy(sessionIter->second.tokens.begin(), sessionIter->second.tokens.end(), tokens);
        }
    }
    else
    {
        numTokens = 0u;
    }
}

void Lexer::processLine(std::string_view currentLine, LoxScanSession& session)
{

    while (!currentLine.empty())
    {
        const char firstLexeme = currentLine[0];
        // if current token is a space, skip because the rest of this system
        // does not care a bit about that
        if (firstLexeme == ' ')
        {
            currentLine.remove_prefix(1u);
            session.offsetInCurrentLine += 1u;
            continue;
        }

        // if this first lexeme matches this set, it's one of our guaranteed
        // single character lexemes that we can just directly add
        if (auto singleCharIter = singleCharLexemes.find(firstLexeme);
            singleCharIter != singleCharLexemes.end())
        {
            session.addToken(singleCharIter->second, 1u, currentLine);
            continue;
        }
        
        // if first lexeme is in set of dual-character lexemes, enter this
        // logic loop
        if (dualCharPrefixes.find(firstLexeme) != dualCharPrefixes.end())
        {
            extractDualCharToken(currentLine, firstLexeme, session);
            continue;
        }

        if (firstLexeme == '"')
        {
            extractStringLiteral(currentLine, session);
            continue;
        }

        if (IsNumericDigit(firstLexeme))
        {
            extractNumericLiteral(currentLine, session);
            continue;
        }

        if (IsAlphaNumeric(firstLexeme))
        {
            extractKeywordOrIdentifier(currentLine, session);
            continue;
        }

        // Reached here, means our current character isn't being processed at all
        session.addError(LoxCompilerErrorCode::UnrecognizedLexeme, currentLine);
        if (session.errors.size() > k_maxErrorsInScanSession)
        {
            std::cerr << "Reached error limit when parsing, likely bad character in stream.";
            throw std::runtime_error("Reached error limit!");
        }
    }
}

void Lexer::extractDualCharToken(std::string_view& line, const char firstChar, LoxScanSession& session)
{
    const char secondChar = line[1];
    switch (firstChar)
    {
    case '!':
        secondChar == '=' ?
            session.addToken(TokenType::LogicalNotEqual, 2u, line) : session.addToken(TokenType::LogicalNot, 1u, line);
        break;
    case '=':
        secondChar  == '=' ?
            session.addToken(TokenType::EqualEqual, 2u, line) : session.addToken(TokenType::Equal, 1u, line);
        break;
    case '/':
        secondChar == '/' ? session.addSingleLineCommentToken(line) : session.addToken(TokenType::Slash, 1u, line);
        break;
    default:
        session.addError(LoxCompilerErrorCode::UnrecognizedLexeme, line);
        // erase this line, because at the least the line is trashed
        line.remove_prefix(line.size());
        break;
    }
}

void Lexer::extractStringLiteral(std::string_view& line, LoxScanSession& session)
{
    // str literal found. seach for end quote starting at +1 from here
    const size_t endOfLiteral = line.find_first_of('"', 1u);
    if (endOfLiteral == std::string_view::npos)
    {
        session.addError(LoxCompilerErrorCode::StringLiteralMissingEndQuote, line);
        // make the line empty by creating a default ctor empty one
        // breaks from loop, since this error completely trashes the line
        line = std::string_view{};
    }
    else
    {
        // extract part we want
        std::string_view literalString = line.substr(1u, endOfLiteral - 1u);
        session.addStrLiteralToken(line, literalString);
    }
}

void Lexer::extractNumericLiteral(std::string_view& line, LoxScanSession& session)
{
    // End of numeric literal is just the next space, which is
    // what we can just check for since we expect from_chars
    // to handle the input we give it
    size_t endOfNumLiteral = line.find_first_of(' ', 1u);
    if (endOfNumLiteral == std::string_view::npos)
    {
        // this can fail if the numeric literal is the end of this line
        // before the semicolon, so check for that case
        endOfNumLiteral = line.find_first_of(';');
        if (endOfNumLiteral == std::string_view::npos)
        {
            // now we're cooked. add relevant error, continue to next
            // line 
            session.addError(LoxCompilerErrorCode::NumericLiteralParseFailure, line);
        }
    }

    float convertedLiteral = 0.0f;
    std::from_chars_result convertResult = std::from_chars(&line[0], &line[endOfNumLiteral], convertedLiteral);
    
    if (static_cast<int>(convertResult.ec) == 0)
    {
        session.addNumLiteralToken(line, convertedLiteral, endOfNumLiteral);
    }
    else
    {
        session.addError(LoxCompilerErrorCode::NumericLiteralConversionFailure, line);
        line = std::string_view{};
    }
}

void Lexer::extractKeywordOrIdentifier(std::string_view& line, LoxScanSession& session)
{
    // first char that's not alphanumeric indicates end of keyword
    auto endIter = std::find_if_not(line.begin(), line.end(), IsAlphaNumeric);
    if (endIter != line.end())
    {
        std::string_view token = std::string_view{ line.begin(), endIter };
        // see if token matches potential keywords, otherwise it is
        // an identifier
        auto keywordTokenIter = k_keywordTokenTypeMap.find(token);
        if (keywordTokenIter != k_keywordTokenTypeMap.end())
        {
            // if last token type added isn't a keyword, we can add it
            bool validToAddKeyword = true;
            if (!session.tokens.empty())
            {
                const TokenType lastTokenType = session.tokens.back().type;
                validToAddKeyword = !IsKeywordTokenType(lastTokenType);
            }
            // Should be valid in most cases, but this helps us catch potential errors
            if (validToAddKeyword)
            {
                session.addKeywordToken(line, keywordTokenIter->second);
            }
            else
            {
                // last token added was a keyword, and in lox this invalid
                // behavior that won't work. log the error. likely
                // that the user tried to do (keyword) (identifier)
                session.addError(LoxCompilerErrorCode::InvalidKeywordUsage, line);
                const size_t keywordLen = k_keywordLengthMap.at(keywordTokenIter->second);
                line.remove_prefix(keywordLen);
            }
        }
        else
        {
            // probably an identifier token. will add further checks as I think
            // of them and run into them
            session.addIdentifierToken(line, token);
        }
    }
    else
    {
        session.addError(LoxCompilerErrorCode::TokenExtractionFailed, line);
        line = std::string_view{};
    }
}
