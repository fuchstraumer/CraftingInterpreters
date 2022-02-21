#include "tests/LexerTests.hpp"
#include <iostream>
#include <string_view>

#include "Interpreter.hpp"

int main(int argc, char* argv[])
{
    std::string_view results = RunBasicLexerTests();
    std::cerr << results;
    UnaryExpression expr;
    return 0;
}
