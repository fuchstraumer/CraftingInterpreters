#include "LexerTests.hpp"
#include "LoxErrors.hpp"
#include "Token.hpp"
#include "Lexer.hpp"
#include "Utility.hpp"
#include <format>
#include <sstream>
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <string>

constexpr bool TokenComparator(const LoxToken& lhs, const LoxToken& rhs)
{
    const bool basicMembersMatch = (lhs.type == rhs.type) && (lhs.line == rhs.line) && (lhs.offset == rhs.offset);
    const bool strLiteralMatch = lhs.strLiteral == rhs.strLiteral;
    const bool numLiteralMatch = lhs.numericLiteral == rhs.numericLiteral;
    return basicMembersMatch && strLiteralMatch && numLiteralMatch;
}

const char* CommentPrintAndStringLiteralSource =
R"(
    // Your first lox program
    print "Hello, world!";
)";

static const std::string commentStr = "Your first lox program";
static const std::string literalStr = "Hello, world!";

std::array<LoxToken, 6> CommentPrintAndStringLiteralTokens
{
    LoxToken{ TokenType::CommentBegin, 1, 4 },
    LoxToken{ TokenType::CommentString, 1, 7, commentStr },
    LoxToken{ TokenType::Print, 2, 4 },
    LoxToken{ TokenType::StringLiteral, 2, 11, literalStr },
    LoxToken{ TokenType::Semicolon, 2, 25 },
    LoxToken{ TokenType::EndOfFile, 3, 0 }
};

const char* VarsAndLiteralsTestSource =
R"(
    var TestValue0_ = 1.234;
    var Test_Value_2 = "Test!";
)";

static const std::string TestValue0_("TestValue0_");
static const std::string Test_Value_2("Test_Value_2");
static const std::string TestEXCLAIMS("Test!");
std::array<LoxToken, 11> VarsAndLiteralsTestTokens
{
    LoxToken{ TokenType::Var, 1, 4 },
    LoxToken{ TokenType::Identifier, 1, 8, TestValue0_ },
    LoxToken{ TokenType::Equal, 1, 20 },
    LoxToken{ TokenType::NumberLiteral, 1, 22, 1.234f },
    LoxToken{ TokenType::Semicolon, 1, 27 },
    LoxToken{ TokenType::Var, 2, 4 },
    LoxToken{ TokenType::Identifier, 2, 8, Test_Value_2 },
    LoxToken{ TokenType::Equal, 2, 21 },
    LoxToken{ TokenType::StringLiteral, 2, 24, TestEXCLAIMS },
    LoxToken{ TokenType::Semicolon, 2, 30 },
    LoxToken{ TokenType::EndOfFile, 3, 0 }
};

std::string GetTokenString(const size_t idx, const LoxToken& token)
{
    std::string result;
    result += "IDX: " + std::to_string(idx) + " | ";
    result += "Type: " + std::string(TokenTypeToString(token.type)) + " | ";
    result += "Line: " + std::to_string(token.line) + " | ";
    result += "Column: " + std::to_string(token.offset) + " | ";
    if (!token.strLiteral.empty())
    {
        result += "String Value: " + std::string(token.strLiteral);
    }
    else if (token.numericLiteral != std::numeric_limits<float>::max())
    {
        result += "Numeric Value: " + std::to_string(token.numericLiteral);
    }
    result += "\n";
    return result;
}

std::string GetLoxTokensString(const size_t numTokens, const LoxToken* tokens)
{
    std::string result;
    for (size_t i = 0; i < numTokens; ++i)
    {
        result += GetTokenString(i, tokens[i]);
    }
    return result;
}

struct LoxTestFailError
{
    LoxCompilerErrorCode errorCode;
    std::string message;
};

LoxTestFailError HandleTestFailure(const LoxToken& knownGood, const size_t knownGoodCount, const LoxToken& runtime, const size_t runtimeCount, const size_t indexOfFailedToken)
{
    LoxTestFailError result;
    if (knownGoodCount != runtimeCount)
    {
        result.errorCode = LoxCompilerErrorCode::TestFailTokenCountMismatch;
        result.message = "Known good token count: ";
        result.message += std::to_string(knownGoodCount);
        result.message += " Runtime test token count: ";
        result.message += std::to_string(runtimeCount);
        return result;
    }

    // Construct message string by setting index of where failure occured.
    result.message = " failed tokens are at index: ";
    result.message += std::to_string(indexOfFailedToken);
    // newline, so further detailed information follows this and we keep columns small
    result.message += '\n';

    auto writeOutTokens = [&indexOfFailedToken, &knownGood, &runtime]()->std::string
    {
        std::string result = "Known-good token is:     ";
        result += GetTokenString(indexOfFailedToken, knownGood);
        result += "Runtime-parsed token is: ";
        result += GetTokenString(indexOfFailedToken, runtime);
        result += '\n';
        return result;
    };

    // More complex cases now. Let's dive into the token bit by bit.
    if (knownGood.type != runtime.type)
    {
        result.errorCode = LoxCompilerErrorCode::TestFailTokenTypeMismatch;
        result.message += writeOutTokens();
        return result;
    }

    if (knownGood.offset != runtime.offset)
    {
        result.errorCode = LoxCompilerErrorCode::TestFailTokenPositionMismatch;
        result.message += writeOutTokens();
        return result;
    }

    bool stringContentMismatch =
        (!knownGood.strLiteral.empty() && !runtime.strLiteral.empty()) &&
        !strcmp(knownGood.strLiteral.data(), runtime.strLiteral.data());
    bool numericContentMismatch =
        knownGood.numericLiteral != std::numeric_limits<decltype(knownGood.numericLiteral)>::max() &&
        runtime.numericLiteral != std::numeric_limits<decltype(runtime.numericLiteral)>::max() &&
        knownGood.numericLiteral != runtime.numericLiteral;
  
    if (stringContentMismatch || numericContentMismatch)
    {
        result.errorCode = LoxCompilerErrorCode::TestFailTokenContentMismatch;
        result.message += writeOutTokens();
        return result;
    }

    return result;
}

std::string ErrorCodeMessage(const LoxCompilerErrorCode& code)
{
    switch (code)
    {
    case LoxCompilerErrorCode::TestFailTokenCountMismatch:
        return std::string("Mismatch in count of tokens parsed from runtime content vs length of known-good tokens array.");
    case LoxCompilerErrorCode::TestFailTokenTypeMismatch:
        return std::string("Mistmatch in type of tokens parsed from runtime content vs known-good tokens array");
    case LoxCompilerErrorCode::TestFailTokenPositionMismatch:
        return std::string("Mismatch in parsed position of token from runtime vs known-good position of token");
    case LoxCompilerErrorCode::TestFailTokenContentMismatch:
        return std::string("Mismatch in stored content of string or float token, between parsed runtime tokens and known-good stored value");
    default:
        return std::string("Invalid test code??");
    }
}

std::string_view RunBasicLexerTests()
{
    auto& lexer = Lexer::GetLexerInstance();

    Lexer::OutputHandle result = lexer.ParseScript(CommentPrintAndStringLiteralSource);

    std::vector<LoxToken> tokens;
    size_t numTokens = 0u;
    lexer.GetTokensForHandle(result, numTokens, nullptr);
    if (numTokens != 0u)
    {
        tokens.resize(numTokens);
        lexer.GetTokensForHandle(result, numTokens, &tokens[0]);
    }
    else
    {
        throw std::runtime_error("Failed to run test!");
    }

    // check validity of tokens
    auto firstMismatchIter = std::mismatch(
        CommentPrintAndStringLiteralTokens.begin(),
        CommentPrintAndStringLiteralTokens.end(),
        tokens.begin(),
        tokens.end(),
        TokenComparator);
       
    // this just provides a quick thing to check in the debug window when these tests fail
    constexpr size_t test1ArrayLen = std::distance(CommentPrintAndStringLiteralTokens.begin(), CommentPrintAndStringLiteralTokens.end());
    const size_t test1TokensLen = std::distance(tokens.begin(), tokens.end());
    const bool test1LengthMatch = test1ArrayLen == test1TokensLen;

    if (firstMismatchIter.first == CommentPrintAndStringLiteralTokens.end() &&
        firstMismatchIter.second == tokens.end())
    {
        // todo: print tokens
        std::cout << "First test succeeded!\n";
        std::cout << "Input source code: \n";
        std::cout << CommentPrintAndStringLiteralSource << "\n";
        std::cout << "Result tokens: \n";
        std::string resultTokens = GetLoxTokensString(tokens.size(), &tokens[0]);
        std::cout << resultTokens << "\n";
    }
    else
    {
        const size_t testFailPos = std::distance(CommentPrintAndStringLiteralTokens.begin(), firstMismatchIter.first);
        auto testFailure =
            HandleTestFailure(*firstMismatchIter.first, test1ArrayLen, *firstMismatchIter.second, test1TokensLen, testFailPos);
        std::cout << ErrorCodeMessage(testFailure.errorCode) << ',';
        std::cout << testFailure.message << '\n';
        throw std::runtime_error("Failed first test!");
    }

    result = lexer.ParseScript(VarsAndLiteralsTestSource);
    lexer.GetTokensForHandle(result, numTokens, nullptr);
    if (numTokens != 0u || numTokens != VarsAndLiteralsTestTokens.size())
    {
        tokens.resize(numTokens);
        lexer.GetTokensForHandle(result, numTokens, &tokens[0]);
    }
    else
    {
        throw std::runtime_error("Failed to parse second test script!");
    }

    auto secondMismatchIter = std::mismatch(
        VarsAndLiteralsTestTokens.begin(),
        VarsAndLiteralsTestTokens.end(),
        tokens.begin(),
        tokens.end(),
        TokenComparator);

    constexpr size_t test2ArrayLen = std::distance(VarsAndLiteralsTestTokens.begin(), VarsAndLiteralsTestTokens.end());
    const size_t test2TokensLen = std::distance(tokens.begin(), tokens.end());
    const bool test2LengthMatch = test2ArrayLen == test2TokensLen;

    if (secondMismatchIter.first == VarsAndLiteralsTestTokens.end() &&
        secondMismatchIter.second == tokens.end())
    {
        std::cout << "Second test succeeded!\n";
        std::cout << "Input source code:\n";
        std::cout << VarsAndLiteralsTestSource << "\n";
        std::cout << "Result tokens from source code:\n";
        std::string resultTokens = GetLoxTokensString(tokens.size(), &tokens[0]);
        std::cout << resultTokens << "\n";
    }
    else
    {
        const size_t testFailPos = std::distance(VarsAndLiteralsTestTokens.begin(), secondMismatchIter.first);
        auto testFailure =
            HandleTestFailure(*secondMismatchIter.first, test2ArrayLen, *secondMismatchIter.second, test2TokensLen, testFailPos);
        std::cout << ErrorCodeMessage(testFailure.errorCode) << ',';
        std::cout << testFailure.message << '\n';
        throw std::runtime_error("Second test failed!");
    }

    return std::string_view{};
}