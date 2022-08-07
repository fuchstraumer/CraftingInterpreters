#include "Utility.hpp"
#include "Token.hpp"
#include <unordered_map>
#include <string>

std::unordered_map<TokenType, std::string> tokenStrings
{
    { TokenType::Invalid, "Invalid" },
    { TokenType::LeftParen, "Left Parentheses" },
    { TokenType::RightParen, "Right Parentheses" },
    { TokenType::LeftBrace, "Left Bracket" },
    { TokenType::RightBrace, "Right Bracket" },
    { TokenType::Comma, "Comma" },
    { TokenType::Dot, "Dot" },
    { TokenType::Minus, "Minus" },
    { TokenType::Plus, "Plus" },
    { TokenType::Semicolon, "Semicolon" },
    { TokenType::Slash, "Slash" },
    { TokenType::Star, "Star" },
    { TokenType::LogicalNot, "Logical Not (!)" },
    { TokenType::LogicalNotEqual, "Logical Not Equal (!=)" },
    { TokenType::Equal, "Equal" },
    { TokenType::EqualEqual, "Equality Operator (==)" },
    { TokenType::Greater, "Greater" },
    { TokenType::GreaterEqual, "Greater-Equal" },
    { TokenType::Less, "Less" },
    { TokenType::LessEqual, "Less-Equal" },
    // literals
    { TokenType::Identifier, "Identifier" },
    { TokenType::StringLiteral, "String Literal" },
    { TokenType::NumberLiteral, "Numeric Literal" },
    // special items
    { TokenType::CommentBegin, "Comment Begin (//)" },
    { TokenType::CommentString, "Comment String" },
    { TokenType::EndOfFile, "End-Of-File" },
    // keywords
    { TokenType::And, "And" },
    { TokenType::Class, "Class" },
    { TokenType::Else, "Else" },
    { TokenType::False, "False" },
    { TokenType::Fun, "Fun" },
    { TokenType::For, "For" },
    { TokenType::If, "If" },
    { TokenType::Nil, "Nil" },
    { TokenType::Or, "Or" },
    { TokenType::Print, "Print" },
    { TokenType::Return, "Return" },
    { TokenType::Super, "Super" },
    { TokenType::This, "This" },
    { TokenType::True, "True" },
    { TokenType::Var, "Var" },
    { TokenType::While, "While" },
};

const char* TokenTypeToString(const TokenType& type)
{
    auto iter = tokenStrings.find(type);
    if (iter != tokenStrings.end())
    {
        return iter->second.c_str();
    }
    else
    {
        return "TokenTypeUnfound:NotInTokenToStrMap";
    }
}