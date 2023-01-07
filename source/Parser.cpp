#include "Parser.hpp"

std::string getParseErrorString(LoxCompilerErrorCode errorCode) noexcept
{
    switch (errorCode)
    {
    case LoxCompilerErrorCode::ExpectedTokenNotFound:
        return "Expected token not found";
    case LoxCompilerErrorCode::UnclosedBrackets:
        return "Unclosed brackets found";
    case LoxCompilerErrorCode::UnclosedParentheses:
        return "Unclosed parentheses found";
    case LoxCompilerErrorCode::InvalidTokenOrdering:
        return "Invalid token ordering";
    case LoxCompilerErrorCode::MissingPrimaryToken:
        return "Missing primary token";
    default:
        return "Invalid error code for ParseError class";
    }
}

ParseError::ParseError(LoxCompilerErrorCode ec, LoxToken _token) noexcept :
    errorCode(ec), token(_token), std::runtime_error(getParseErrorString(ec)) {}

Parser::Parser(const std::vector<LoxToken>& _tokens) : tokens(_tokens) {}

Expression Parser::expression(Parser::TokenIter iter) const
{
    return equality(iter);
}

Expression Parser::equality(Parser::TokenIter iter) const
{
    const static std::vector<TokenType> equalityTokens
    {
        TokenType::EqualEqual,
        TokenType::LogicalNotEqual
    };

    Expression lhs = comparison(iter);
    Expression result;

    // now a while loop to follow down
    while (match(equalityTokens, iter))
    {
        LoxToken operatorToken = previous(iter);
        Expression rhs = comparison(iter);
        result = BinaryExpression{ lhs, operatorToken, rhs };
    }

    return result;
}

Expression Parser::comparison(TokenIter iter) const
{
    const static std::vector<TokenType> comparisonTokens
    {
        TokenType::Greater,
        TokenType::GreaterEqual,
        TokenType::Less,
        TokenType::LessEqual
    };

    Expression lhs = term(iter);
    Expression result = lhs;
    
    while (match(comparisonTokens, iter))
    {
        LoxToken operatorToken = previous(iter);
        Expression rhs = term(iter);
        result = BinaryExpression{ lhs, operatorToken, rhs };
    }

    return result;
}

Expression Parser::term(TokenIter iter) const
{
    const static std::vector<TokenType> termTokens
    {
        TokenType::Minus,
        TokenType::Plus
    };

    Expression lhs = factor(iter);
    Expression result = lhs;

    while (match(termTokens, iter))
    {
        LoxToken operatorToken = previous(iter);
        Expression rhs = factor(iter);
        result = BinaryExpression{ lhs, operatorToken, rhs };
    }

    return result;
}

Expression Parser::factor(TokenIter iter) const
{
    const static std::vector<TokenType> factorTokens
    {
        TokenType::Slash,
        TokenType::Star
    };

    Expression lhs = unary(iter);
    Expression result = lhs;

    while (match(factorTokens, iter))
    {
        LoxToken operatorToken = previous(iter);
        Expression rhs = unary(iter);
        result = BinaryExpression{ lhs, operatorToken, rhs };
    }

    return result;
}

Expression Parser::unary(TokenIter iter) const
{
    const static std::vector<TokenType> unaryTokens
    {
        TokenType::LogicalNot,
        TokenType::Minus
    };

    if (match(unaryTokens, iter))
    {
        LoxToken operatorToken = previous(iter);
        Expression rhs = unary(iter);
        return UnaryExpression{ operatorToken, rhs };
    }

    return primary(iter);
}

Expression Parser::primary(TokenIter iter) const
{
    if (match({ TokenType::False }, iter))
    {
        return LanguageLiteralExpression{ TokenType::False, *iter };
    }
    else if (match({ TokenType::True }, iter))
    {
        return LanguageLiteralExpression{ TokenType::True, *iter };
    }
    else if (match({ TokenType::Nil }, iter))
    {
        return LanguageLiteralExpression{ TokenType::Nil, *iter };
    }
    else if (match({ TokenType::NumberLiteral }, iter))
    {
        return NumericLiteralExpression{ iter->numericLiteral };
    }
    else if (match({ TokenType::StringLiteral }, iter))
    {
        return StringLiteralExpression{ iter->strLiteral };
    }
    else if (match({ TokenType::LeftParen }, iter))
    {
        Expression expr = expression(iter);
        consume(TokenType::RightParen, "Expected a ')' after expression.");
        return GroupingExpression{ expr };
    }
    else
    {
        // uh oh
        throw std::runtime_error("Oops");
    }
}

bool Parser::isAtEnd(TokenIter iter) const
{
    if (iter != tokens.cend() && iter->type == TokenType::EndOfFile)
    {
        return true;
    }
    else
    {
        if (iter == tokens.cend())
        {
            // error? we shouldn't be able to do this. means
            // scanner didn't generate/find valid EOF token
            throw ParseError(LoxCompilerErrorCode::MissingEOF, LoxToken());
        }
        return false;
    }
}

const LoxToken& Parser::advance(TokenIter iter) const noexcept
{
    if (!isAtEnd(iter))
    {
        ++iter;
    }
    
    return previous(iter);
}

const LoxToken& Parser::previous(TokenIter iter) const noexcept
{
    TokenIter prevIter = iter - 1;
    return *prevIter;
}

bool Parser::match(const std::vector<TokenType>& types, TokenIter iter) const noexcept
{
    for (const auto& tokenType : types)
    {
        if (tokenType == iter->type)
        {
            advance(iter);
            return true;
        }
    }
    return false;
}

void Parser::consume(TokenType type, std::string failureString) const
{

}
