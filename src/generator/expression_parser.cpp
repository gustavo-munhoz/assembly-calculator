//
// Created by Gustavo Munhoz Correa on 02/04/25.
//

#include <string>
#include <set>
#include "../../include/generator/assembly_generator.hpp"
#include "../../include/generator/parse_helpers.hpp"
#include "../../include/generator/temp_var_manager.hpp"

namespace ExpressionParser {
    using namespace std;
    using namespace ParseHelpers;
    using namespace AssemblyGenerator;

    string parseParenExpression(const string &s, int &pos, string &result, int lineIndex);
    string parseParenSingleOperator(const string &s, int &pos, const string &left, string &result, int lineIndex);
    string parseParenBinary(const string &s, int &pos, const string &left, string &result, int lineIndex);
    string parseParenUnary(const string &s, int &pos, string &result, int lineIndex);

    bool isOperator(const string &token) {
        static const set<string> ops = { "+", "-", "*", "|", "/", "%", "^", "RES", "MEM" };
        return ops.find(token) != ops.end();
    }

    void expectChar(const string &s, int &pos, char expected) {
        if (pos >= s.size() || s[pos] != expected)
            throw runtime_error(string("Expected '") + expected + "' at position " + to_string(pos));
        pos++;
    }

    string parseExpression(const string &s, int &pos, string &result, int lineIndex) {
        ParseHelpers::skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == '(') {
            return parseParenExpression(s, pos, result, lineIndex);
        } else {
            string token = ParseHelpers::parseToken(s, pos);
            if (token.empty())
                throw runtime_error("Unexpected end of expression.");
            result = token;
            return "";
        }
    }

    string parseParenExpression(const string &s, int &pos, string &result, int lineIndex) {
        pos++; // consome '('
        ParseHelpers::skipWhitespace(s, pos);

        int lookaheadPos = pos;
        string token = ParseHelpers::parseToken(s, lookaheadPos);

        if (ExpressionParser::isOperator(token)) {
            return parseParenUnary(s, pos, result, lineIndex);
        } else {
            string left;
            string leftCode = parseExpression(s, pos, left, lineIndex);
            ParseHelpers::skipWhitespace(s, pos);

            lookaheadPos = pos;
            string nextToken = ParseHelpers::parseToken(s, lookaheadPos);

            if (ExpressionParser::isOperator(nextToken)) {
                string opCode = parseParenSingleOperator(s, pos, left, result, lineIndex);
                return leftCode + opCode;
            } else {
                string binaryCode = parseParenBinary(s, pos, left, result, lineIndex);
                return leftCode + binaryCode;
            }
        }
    }

    string parseParenUnary(const string &s, int &pos, string &result, int lineIndex) {
        string op = ParseHelpers::parseToken(s, pos);
        ParseHelpers::skipWhitespace(s, pos);
        expectChar(s, pos, ')');
        string tempVar = TempVarManager::newTemp();
        result = tempVar;
        return AssemblyGenerator::generateOperationAssembly(op, "", "", tempVar, lineIndex);
    }

    string parseParenSingleOperator(const string &s, int &pos, const string &left, string &result, int lineIndex) {
        string op = ParseHelpers::parseToken(s, pos);
        ParseHelpers::skipWhitespace(s, pos);
        expectChar(s, pos, ')');
        string tempVar = TempVarManager::newTemp();
        result = tempVar;
        return AssemblyGenerator::generateOperationAssembly(op, left, "", tempVar, lineIndex);
    }

    string parseParenBinary(const string &s, int &pos, const string &left, string &result, int lineIndex) {
        string right;
        string rightCode = parseExpression(s, pos, right, lineIndex);
        ParseHelpers::skipWhitespace(s, pos);
        string op = ParseHelpers::parseToken(s, pos);
        if (op.empty())
            throw runtime_error("Operator expected.");
        ParseHelpers::skipWhitespace(s, pos);
        expectChar(s, pos, ')');
        string tempVar = TempVarManager::newTemp();
        result = tempVar;
        string opCode = AssemblyGenerator::generateOperationAssembly(op, left, right, tempVar, lineIndex);
        return rightCode + opCode;
    }

}