//
// Created by Gustavo Munhoz Correa on 02/04/25.
//

#include <string>

namespace ParseHelpers {
    void skipWhitespace(const std::string &s, int &pos) {
        while (pos < s.size() && isspace(s[pos])) {
            pos++;
        }
    }

    std::string parseToken(const std::string &s, int &pos) {
        skipWhitespace(s, pos);
        int start = pos;
        while (pos < s.size() && !isspace(s[pos]) && s[pos] != '(' && s[pos] != ')') {
            pos++;
        }
        return s.substr(start, pos - start);
    }

    bool isNumber(const std::string &s) {
        if (s.empty()) return false;
        bool hasDecimal = false;
        bool hasDigit = false;
        int i = 0;
        if (s[0] == '-' || s[0] == '+') {
            i = 1;
        }
        for (; i < s.length(); i++) {
            if (s[i] == '.') {
                if (hasDecimal) return false;
                hasDecimal = true;
            } else if (isdigit(s[i])) {
                hasDigit = true;
            } else {
                return false;
            }
        }
        return hasDigit;
    }
}