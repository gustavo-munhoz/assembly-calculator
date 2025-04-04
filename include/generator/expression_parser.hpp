//
// Created by Gustavo Munhoz Correa on 02/04/25.
//

#ifndef PROJ1_EXPRESSION_PARSER_HPP
#define PROJ1_EXPRESSION_PARSER_HPP

#include <string>

namespace ExpressionParser {
    bool isOperator(const std::string &token);

    std::string parseExpression(
        const std::string &s,
        int &pos,
        std::string &result,
        int lineIndex
    );
}

#endif //PROJ1_EXPRESSION_PARSER_HPP
