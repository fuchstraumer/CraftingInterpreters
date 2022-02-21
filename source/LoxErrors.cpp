#include "LoxErrors.hpp"
#include <string>

namespace
{
    struct LoxScannerErrorCategory : std::error_category
    {
        const char* name() const noexcept final;
        std::string message(int errorValue) const final;
        bool equivalent(const std::error_code& ec, int condition) const noexcept final;
    };

    const char* LoxScannerErrorCategory::name() const noexcept
    {
        // return the name of the error category - no need for whole object/struct name
        return "LoxScanner";
    }

    std::string LoxScannerErrorCategory::message(int errorValue) const
    {
        switch (static_cast<LoxCompilerErrorCode>(errorValue))
        {
        case LoxCompilerErrorCode::ForbiddenToken:
            return std::string("Used a forbidden token/character in input source.");
            break;
        case LoxCompilerErrorCode::UnrecognizedLexeme:
            return std::string("Found an unrecognized lexeme when processing tokens");
            break;
        case LoxCompilerErrorCode::ReservedWord:
            return std::string("Used a word reserved by the language in input source.");
            break;
        case LoxCompilerErrorCode::InvalidInputString:
            return std::string("Input source given to the scanner was invalid and could not be parsed.");
            break;
        case LoxCompilerErrorCode::UnknownError:
            [[fallthrough]];
        default:
            return std::string("Unknown error: LoxCompilerErrorCode value did not match any existing scanner");
        }
    }

    bool LoxScannerErrorCategory::equivalent(
        const std::error_code& ec, int condition) const noexcept
    {
        return false;
    }

    const LoxScannerErrorCategory loxScannerErrorCategory;

    struct LoxFailureSourceCategory : std::error_category
    {
        const char* name() const noexcept final;
        std::string message(int errorValue) const final;
        bool equivalent(const std::error_code& ec, int condition) const noexcept final;
    };

    const char* LoxFailureSourceCategory::name() const noexcept
    {
        return "FailureSource";
    }

    std::string LoxFailureSourceCategory::message(int errorValue) const
    {
        
    }
}

std::error_code make_error_code(LoxCompilerErrorCode errorCode)
{
    return { static_cast<int>(errorCode), loxScannerErrorCategory };
}
