#pragma once
#ifndef LOX_INTERPRETER_ERRORS_HPP
#define LOX_INTERPRETER_ERRORS_HPP
#include <system_error>

// An error condition, effectively. Individual error codes from
// systems have a root cause that is effectively one of these.
// Allows introspection on what the actual source of an error code was 
// (thus, the name)
enum class LoxFailureSource : int
{
    // 0 is a reserved value
    // BadUserInput: user did something wrong they must correct. 
    // Most of these are highly recoverable. Some can be massaged.
    BadUserInput = 1,
    // Systemic failure - our code did something wrong, and the user
    // is not at fault and cannot be expected to salvage this. User
    // is also not expected to understand these errors, but we can't continue
    SystemFailure = 2,
    UnknownFailure = 3,
};

enum class LoxFailureSeverity : int
{
    // Something was done incorrectly, but it can either be corrected by the system or by the
    // user and we can continue on as if nothing bad really happened.
    Recoverable = 1,
    // Something has broken entirely, and the program state is probably trashed. We can't recover
    // and will crash
    Unrecoverable = 2,
};


enum class LoxCompilerErrorCode : int
{
    // 0 is reserved for compatability with std::error_code
    // Initial range of values all maps to BadUserInput source: root cause of this is incorrect user input
    ForbiddenToken = 1,
    UnrecognizedLexeme, // Singular lexeme that couldn't be matched to valid set of lexemes
    UnrecognizedDualCharacterLexeme, // Specialized error code, so that we know to print out/handle a dual character lexeme error :)
    ReservedWord, // current token uses a language keyword or reserved word
    InvalidInputString,
    StringLiteralMissingEndQuote, // no end quotation, can't create a valid string literal at all
    NumericLiteralParseFailure, // couldn't extract literal from string
    NumericLiteralConversionFailure, // conversion of literal to result num failed
    InvalidKeywordUsage, // keyword followed by keyword, usually
    // Start of interpreter failures. Root cause is within our system, and user has no ability to stop this
    ScannerFailure = 30,
    // Internal failure: emplace into container of sessions failed. Not a good sign!
    UnableToSaveSessionResults,
    // Failure to extract token.
    TokenExtractionFailed,


    // Start of internal unknown failures
    UnknownError = 80,

    ParserError = 120,
    ExpectedTokenNotFound,
    UnclosedBrackets,
    UnclosedParentheses,
    InvalidTokenOrdering,
    MissingPrimaryToken,
    MissingEOF,

    // Start of failures coming from tests
    TestFailError = 160,
    // Count of tokens parsed doesn't match "known good" token count
    TestFailTokenCountMismatch,
    // A token type mismatch occurred between runtime and compile-time "known good" token arrays
    TestFailTokenTypeMismatch,
    // Position of token was parsed incorrectly in test content
    TestFailTokenPositionMismatch,
    // Content of a token is incorrect
    TestFailTokenContentMismatch,
};

namespace std
{
    template<>
    struct is_error_condition_enum<LoxFailureSource> : true_type {};
    template<>
    struct is_error_code_enum<LoxCompilerErrorCode> : true_type {};
}

std::error_condition make_error_condition(LoxFailureSource);
std::error_code make_error_code(LoxCompilerErrorCode errorCode);

#endif //!LOX_INTERPRETER_ERRORS_HPP