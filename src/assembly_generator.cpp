//
// Created by Gustavo Munhoz Correa on 20/03/25.
//

#include "../include/assembly_generator.hpp"
#include "../include/dsegManager.hpp"
#include "../include/sections/config_assembly.hpp"
#include "../include/sections/print_assembly.hpp"
#include "../include/sections/serial_comm_assembly.hpp"

#include "../include/operations/add_assembly.hpp"
#include "../include/operations/subtract_assembly.hpp"
#include "../include/operations/mult_assembly.hpp"
#include "../include/operations/int_division_assembly.hpp"
#include "../include/operations/pow_assembly.hpp"
#include "../include/operations/res_assembly.hpp"
#include "../include/operations/mem_assembly.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cctype>
#include <string>
#include <set>

using namespace std;

static int tempCount = 0;

static string newTemp() {
    string tempName = "T" + to_string(++tempCount);

    dsegDeclarations << tempName << "_L: .byte 1\n";
    dsegDeclarations << tempName << "_H: .byte 1\n";
    return tempName;
}

static void skipWhitespace(const string &s, int &pos) {
    while (pos < s.size() && isspace(s[pos])) { pos++; }
}

static string parseToken(const string &s, int &pos) {
    skipWhitespace(s, pos);
    int start = pos;
    while (pos < s.size() && !isspace(s[pos]) && s[pos] != '(' && s[pos] != ')') {
        pos++;
    }

    return s.substr(start, pos - start);
}

static bool isNumber(const string &s) {
    if (s.empty()) return false;

    int i = 0;
    if (s[0] == '-' || s[0] == '+') i = 1;

    for (; i < s.size(); i++) {
        if (!isdigit(s[i])) return false;
    }
    return true;
}

static pair<uint8_t, uint8_t> splitIntoBytes(int16_t value) {
    auto uvalue = static_cast<uint16_t>(value);
    uint8_t low = uvalue & 0xFF;
    uint8_t high = (uvalue >> 8) & 0xFF;
    return {low, high};
}

static string generateLoadOperandAssembly(
    const string &operand,
    const string &regLow,
    const string &regHigh
) {
    stringstream ss;
    if (isNumber(operand)) {
        int value = stoi(operand);
        if (
            value < numeric_limits<int16_t>::min()
            || value > numeric_limits<int16_t>::max()
        ) {
            throw out_of_range("Valor fora do intervalo de int16_t");
        }

        auto num = static_cast<int16_t>(value);

        auto [low, high] = splitIntoBytes(num);
        ss << "    LDI " << regLow << ", " << int(low) << "\n";
        ss << "    LDI " << regHigh << ", " << int(high) << "\n";
    } else {
        ss << "    LDS " << regLow << ", " << operand << "_L\n";
        ss << "    LDS " << regHigh << ", " << operand << "_H\n";
    }
    return ss.str();
}

static string generateOperationAssembly(
        const string &op,
        const string &left,
        const string &right,
        const string &temp,
        const int lineIndex
) {
    stringstream code;
    if (right.empty()) {
        if (op == "RES") {
            code << "; Operação de resultado N linhas atrás: " << left << " RES" << "\n";
            code << "    LDI r24, " << lineIndex - stoi(left) << "\n";
            code << "    RCALL res_op\n";
            code << "    STS " << temp << "_L, r24\n";
            code << "    STS " << temp << "_H, r25\n";

        } else if (op == "MEM") {
            if (left.empty()) {
                code << "; Operação de ler valor da memória\n";
                code << "    RCALL get_mem\n";
                code << "    STS " << temp << "_L, r24\n";
                code << "    STS " << temp << "_H, r25\n";
            } else {
                code << "; Operação de escrever valor na memória: " << left << " MEM" << "\n";
                code << generateLoadOperandAssembly(left, "r24", "r25");
                code << "    RCALL set_mem\n";
                code << "    STS " << temp << "_L, r24\n";
                code << "    STS " << temp << "_H, r25\n";
            }
        }
    } else {
        if (op == "+") {
            code << "; Operação de adição: " << left << " + " << right << "\n";
            code << generateLoadOperandAssembly(left, "r24", "r25");
            code << generateLoadOperandAssembly(right, "r22", "r23");
            code << "    RCALL int_add\n";
            code << "    STS " << temp << "_L, r24\n";
            code << "    STS " << temp << "_H, r25\n";
        } else if (op == "-") {
            code << "; Operação de subtração: " << left << " - " << right << "\n";
            code << generateLoadOperandAssembly(left, "r24", "r25");
            code << generateLoadOperandAssembly(right, "r22", "r23");
            code << "    RCALL int_sub\n";
            code << "    STS " << temp << "_L, r24\n";
            code << "    STS " << temp << "_H, r25\n";
        } else if (op == "*") {
            code << "; Operação de multiplicação: " << left << " * " << right << "\n";
            code << generateLoadOperandAssembly(left, "r24", "r25");
            code << generateLoadOperandAssembly(right, "r22", "r23");
            code << "    RCALL int_mul\n";
            code << "    STS " << temp << "_L, r24\n";
            code << "    STS " << temp << "_H, r25\n";
        } else if (op == "/") {
            code << "; Operação de divisão inteira: " << left << " / " << right << "\n";
            code << generateLoadOperandAssembly(left, "r24", "r25");
            code << generateLoadOperandAssembly(right, "r22", "r23");
            code << "    RCALL int_div\n";
            code << "    STS " << temp << "_L, r24\n";
            code << "    STS " << temp << "_H, r25\n";
        } else if (op == "^") {
            code << "; Operação de potenciação: " << left << " ^ " << right << "\n";
            code << generateLoadOperandAssembly(left, "r24", "r25");
            code << generateLoadOperandAssembly(right, "r22", "r23");
            code << "    RCALL int_pow\n";
            code << "    STS " << temp << "_L, r24\n";
            code << "    STS " << temp << "_H, r25\n";
        }
    }
    return code.str();
}

bool isOperator(const string &token) {
    static const set<string> ops = { "+", "-", "*", "|", "/", "%", "^", "RES", "MEM" };
    return ops.find(token) != ops.end();
}

string parseExpression(const string &s, int &pos, string &result, int lineIndex) {
    skipWhitespace(s, pos);
    string code;

    if (pos < s.size() && s[pos] == '(') {
        pos++;
        skipWhitespace(s, pos);

        int tempPos = pos;
        string firstToken = parseToken(s, tempPos);

        if (isOperator(firstToken)) {
            string op = parseToken(s, pos);
            skipWhitespace(s, pos);
            if (pos >= s.size() || s[pos] != ')')
                throw runtime_error("Expected ')' at end of expression.");
            pos++;
            string tempVar = newTemp();
            result = tempVar;
            code += generateOperationAssembly(op, "", "", tempVar, lineIndex);
        } else {
            string first;
            code += parseExpression(s, pos, first, lineIndex);
            skipWhitespace(s, pos);

            tempPos = pos;
            string nextToken = parseToken(s, tempPos);

            if (isOperator(nextToken)) {
                string op = parseToken(s, pos);
                skipWhitespace(s, pos);
                if (pos >= s.size() || s[pos] != ')')
                    throw runtime_error("Expected ')' at end of expression.");
                pos++;
                string tempVar = newTemp();
                result = tempVar;
                code += generateOperationAssembly(op, first, "", tempVar, lineIndex);
            } else {
                string second;
                code += parseExpression(s, pos, second, lineIndex);
                skipWhitespace(s, pos);
                string op = parseToken(s, pos);
                if (op.empty())
                    throw runtime_error("Operator expected.");
                skipWhitespace(s, pos);
                if (pos >= s.size() || s[pos] != ')')
                    throw runtime_error("Expected ')' at end of expression.");
                pos++;
                string tempVar = newTemp();
                result = tempVar;
                code += generateOperationAssembly(op, first, second, tempVar, lineIndex);
            }
        }
    } else {
        string token = parseToken(s, pos);
        if (token.empty()) { throw runtime_error("Unexpected end of expression."); }
        result = token;
    }

    return code;
}

vector<string> extractRawCommandsFromFile(const string &filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "[ERROR] Could not open file: " << filename << endl;
        return {};
    }

    vector<string> commands;
    string line;
    while (getline(file, line)) {
        commands.push_back(line);
    }

    file.close();
    return commands;
}

string generateFullHeaderAssembly() {
    stringstream ss;
    ss << ".include \"m328Pdef.inc\"\n\n";
    ss << ";-----------------------------------------------------------\n";
    ss << "; Macros e Configurações\n";
    ss << ";-----------------------------------------------------------\n";
    ss << ".macro store\n";
    ss << "    ldi     R16, @1\n";
    ss << "    sts     @0, R16\n";
    ss << ".endm\n\n";
    ss << "; Configuração da USART\n";
    ss << generateConfigAssembly() << "\n";
    ss << "\trjmp comandos\n";
    ss << generatePrint16Assembly() << "\n";
    ss << generatePrint16DivisorAssembly() << "\n";
    ss << generateSerialCommAssembly() << "\n";
    ss << generateAddAssembly() << "\n";
    ss << generateSubAssembly() << "\n";
    ss << generateMultAssembly() << "\n";
    ss << generateIntDivisionAssembly() << "\n";
    ss << generatePowAssembly() << "\n";
    ss << generateResAssembly() << "\n";
    ss << generateValueMemAssembly() << "\n";
    ss << generateMemAssembly() << "\n";
    ss << "comandos:\n";
    return ss.str();
}