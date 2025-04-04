//
// Created by Gustavo Munhoz Correa on 02/04/25.
//

#ifndef PROJ1_PARSE_HELPERS_HPP
#define PROJ1_PARSE_HELPERS_HPP

#include <string>

namespace ParseHelpers {
    void skipWhitespace(const std::string &s, int &pos);
    std::string parseToken(const std::string &s, int &pos);
    bool isNumber(const std::string &s);
}

#endif //PROJ1_PARSE_HELPERS_HPP
