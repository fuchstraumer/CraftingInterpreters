#pragma once
#ifndef LOX_EXPRESSION_HPP
#define LOX_EXPRESSION_HPP
#include "Utility.hpp"
#include "Token.hpp" 
#include <cstdint>
#include <limits>
#include <string>
#include <variant>
#include <concepts>

/*
Initial grammar:

expression -> literal | unary | binary | grouping ;
literal -> NUMBER | STRING | "true" | "false" | "nil" ;
grouping -> "(" expression ")" ;
unary -> ( "-" | "!" ) expression ;
binary -> expression operator expression
operator -> "==" | "!=" | "<" | "<=" | ">" | ">=" | "+" | "-" | "*" | "/" ;
*/

struct SourceLocation
{
    size_t line{ 0u };
    size_t column{ 0u };
};

struct Expression
{
    Expression() noexcept = default;
    ~Expression() noexcept = default;
    Expression(const Expression&) noexcept = default;
    Expression(Expression&&) noexcept = default;
    Expression& operator=(const Expression&) noexcept = default;
    Expression& operator=(Expression&&) noexcept = default;
    SourceLocation loc;
};

// lowest to highest
enum class PrecedenceLevel
{
    Expression,
    Equality,
    Comparison,
    Term,
    Factor,
    Unary,
    Primary
};

struct NumericLiteralExpression : public Expression
{
    explicit NumericLiteralExpression(float val) noexcept : value{ val } {}
    float value{ std::numeric_limits<float>::max() };
};

struct StringLiteralExpression : public Expression
{
    explicit StringLiteralExpression(std::string_view sv) noexcept :
        value{ sv } {}
    std::string value{};
};

struct IdentifierLiteralExpression : public Expression
{
    explicit IdentifierLiteralExpression(std::string_view sv) noexcept :
        identifier{ sv } {}
    std::string identifier{};
};

struct UnaryExpression : public Expression
{
    explicit UnaryExpression(LoxToken op, Expression _rhs) noexcept :
        operatorToken(std::move(op)), rhs(std::move(_rhs)) {}
    LoxToken operatorToken;
    Expression rhs;
};

struct BinaryExpression : public Expression
{
    explicit BinaryExpression(Expression _lhs, LoxToken op, Expression _rhs) noexcept :
        lhs(std::move(_lhs)), operatorToken(std::move(op)), rhs(std::move(_rhs)) {}
    Expression lhs;
    LoxToken operatorToken;
    Expression rhs;
};

template<typename T>
concept IsExpressionType = std::is_base_of_v<Expression, T>;

struct PrettyPrinterVisitor
{
    template<IsExpressionType ExpressionType>
    std::string Visit(const ExpressionType& expr)
    {
        // doing this ourself ensures that recursive calls parenthetize as well
        std::string result("(");

        if constexpr (std::is_same_v<ExpressionType, NumericLiteralExpression>)
        {
            result += std::to_string(expr.value);
        }
        else if constexpr (std::is_same_v<ExpressionType, StringLiteralExpression>)
        {
            result += expr.value;
        }
        else if constexpr (std::is_same_v<ExpressionType, IdentifierLiteralExpression>)
        {
            result += expr.identifier;
        }
        else if constexpr (std::is_same_v<ExpressionType, UnaryExpression>)
        {
            result += TokenTypeToString(expr.operatorToken);
            result += " ";
            result += Visit(expr.rhs);
        }
        else if constexpr (std::is_same_v<ExpressionType, BinaryExpression>)
        {
            result += Visit(expr.lhs);
            result += " ";
            result += TokenTypeToString(expr.operatorToken);
            result += " ";
            result += Visit(expr.rhs);
        }
        else
        {
            result += "INVALID_EXPRESSION_TYPE";
        }

        result += ")";

        return result;
    }
};


#endif //!LOX_EXPRESSION_HPP
