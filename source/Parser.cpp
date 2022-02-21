#include "Parser.hpp"

namespace
{
    
}

Parser::Parser(const std::vector<LoxToken>& _tokens) : tokens(_tokens) {}

Expression Parser::expression(Parser::TokenIter iter) const
{
    return equality(iter);
}

Expression Parser::equality(Parser::TokenIter iter) const
{
    // get first comparator
    Expression expr = comparison(iter);
    // now a while loop to follow down

    return expr;
}

Expression Parser::comparison(TokenIter iter) const
{
    return Expression();
}

bool Parser::isAtEnd(TokenIter iter) const noexcept
{
    if (iter != tokens.cend() && iter->type == TokenType::EndOfFile)
    {
        return true;
    }
    else
    {
        if (iter == tokens.cend())
        {
            
        }
        return false;
    }
}

bool Parser::checkToken(TokenIter iter, TokenType type) const noexcept
{
    return false;
}

const LoxToken& Parser::advance(TokenIter iter) const noexcept
{
    if (!isAtEnd(iter))
    {
        return *iter;
    }

    return *iter;
}

const LoxToken& Parser::previous(TokenIter iter) const noexcept
{
    return *iter;
}

const LoxToken& Parser::peek(TokenIter iter) const noexcept
{
    return *iter;
}