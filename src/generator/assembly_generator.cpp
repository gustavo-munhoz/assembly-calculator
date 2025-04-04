//
// Created by Gustavo Munhoz Correa on 20/03/25.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <set>
#include "../../include/generator/assembly_generator.hpp"
#include "../../include/sections/config_assembly.hpp"
#include "../../include/sections/print_assembly.hpp"
#include "../../include/sections/serial_comm_assembly.hpp"
#include "../../include/operations/add_assembly.hpp"
#include "../../include/operations/subtract_assembly.hpp"
#include "../../include/operations/mult_assembly.hpp"
#include "../../include/operations/real_division_assembly.hpp"
#include "../../include/operations/int_division_assembly.hpp"
#include "../../include/operations/mod_assembly.hpp"
#include "../../include/operations/pow_assembly.hpp"
#include "../../include/operations/res_assembly.hpp"
#include "../../include/operations/mem_assembly.hpp"
#include "../../include/generator/parse_helpers.hpp"

using namespace std;

namespace AssemblyGenerator {

    pair<uint8_t, uint8_t> floatToIEEE754Half(float value) {
        uint16_t result = 0;
        if (value == 0.0f) {
            return {0, 0};
        }
        if (value < 0) {
            result |= 0x8000;
            value = -value;
        }
        int exponent = 0;
        float mantissa = value;
        if (mantissa >= 1.0f) {
            while (mantissa >= 2.0f) {
                mantissa /= 2.0f;
                exponent++;
            }
        } else {
            while (mantissa < 1.0f) {
                mantissa *= 2.0f;
                exponent--;
            }
        }
        exponent += 15;
        if (exponent <= 0) {
            return {0, 0};
        } else if (exponent >= 31) {
            return value < 0 ? make_pair(0, 0x7C) : make_pair(0, 0xFC);
        }

        mantissa -= 1.0f;
        auto mantissaBits = static_cast<uint16_t>(mantissa * 1024.0f + 0.5f);
        result |= (exponent << 10) | mantissaBits;
        uint8_t low = result & 0xFF;
        uint8_t high = (result >> 8) & 0xFF;
        return {low, high};
    }

    string generateLoadOperandAssembly(
            const string &operand,
            const string &regLow,
            const string &regHigh
    ) {
        stringstream ss;
        if (ParseHelpers::isNumber(operand)) {
            float value = stof(operand);
            auto [low, high] = floatToIEEE754Half(value);
            ss << "    LDI " << regLow << ", " << int(low) << "\n";
            ss << "    LDI " << regHigh << ", " << int(high) << "\n";
        } else {
            ss << "    LDS " << regLow << ", " << operand << "_L\n";
            ss << "    LDS " << regHigh << ", " << operand << "_H\n";
        }
        return ss.str();
    }

    string generateArithmeticOperationCallAssembly(
            const string &left,
            const string &right,
            const string &label,
            const string &temp
    ) {
        stringstream ss;
        ss << generateLoadOperandAssembly(left, "r24", "r25");
        ss << generateLoadOperandAssembly(right, "r22", "r23");
        ss << "    RCALL " << label << "\n";
        ss << "    STS " << temp << "_L, r24\n";
        ss << "    STS " << temp << "_H, r25\n";
        return ss.str();
    }

    string generateOperationAssembly(
            const string &op,
            const string &left,
            const string &right,
            const string &temp,
            int lineIndex
    ) {
        stringstream ss;
        if (right.empty()) {
            if (op == "RES") {
                ss << "; Operação de resultado N linhas atrás: " << left << " RES" << "\n";
                ss << "    LDI r24, " << lineIndex - stoi(left) << "\n";
                ss << "    RCALL res_op\n";
                ss << "    STS " << temp << "_L, r24\n";
                ss << "    STS " << temp << "_H, r25\n";
            } else if (op == "MEM") {
                if (left.empty()) {
                    ss << "; Operação de ler valor da memória\n";
                    ss << "    RCALL get_mem\n";
                    ss << "    STS " << temp << "_L, r24\n";
                    ss << "    STS " << temp << "_H, r25\n";
                } else {
                    ss << "; Operação de escrever valor na memória: " << left << " MEM" << "\n";
                    ss << generateLoadOperandAssembly(left, "r24", "r25");
                    ss << "    RCALL set_mem\n";
                    ss << "    STS " << temp << "_L, r24\n";
                    ss << "    STS " << temp << "_H, r25\n";
                }
            }
        } else {
            if (op == "+") {
                ss << "; Operação de adição: " << left << " + " << right << "\n";
                ss << generateArithmeticOperationCallAssembly(left, right, "add_f16", temp);
            } else if (op == "-") {
                ss << "; Operação de subtração: " << left << " - " << right << "\n";
                ss << generateArithmeticOperationCallAssembly(left, right, "sub_f16", temp);
            } else if (op == "|") {
                ss << "; Operação de divisão real: " << left << " | " << right << "\n";
                ss << generateArithmeticOperationCallAssembly(left, right, "div_f16", temp);
            } else if (op == "*") {
                ss << "; Operação de multiplicação: " << left << " * " << right << "\n";
                ss << generateArithmeticOperationCallAssembly(left, right, "mul_f16", temp);
            } else if (op == "/") {
                ss << "; Operação de divisão inteira: " << left << " / " << right << "\n";
                ss << generateArithmeticOperationCallAssembly(left, right, "div_int_f16", temp);
            } else if (op == "%") {
                ss << "; Operação de resto: " << left << " % " << right << "\n";
                ss << generateArithmeticOperationCallAssembly(left, right, "mod_f16", temp);
            } else if (op == "^") {
                ss << "; Operação de potenciação: " << left << " ^ " << right << "\n";
                ss << generateArithmeticOperationCallAssembly(left, right, "pow_f16", temp);
            }
        }
        return ss.str();
    }
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
    ss << "\trjmp main\n";
    ss << generatePrintF16Assembly() << "\n";
    ss << generateSerialCommAssembly() << "\n";
    ss << generateAddAssembly() << "\n";
    ss << generateSubAssembly() << "\n";
    ss << generateMulAssembly() << "\n";
    ss << generateRealDivisionAssembly() << "\n";
    ss << generateIntDivisionAssembly() << "\n";
    ss << generateModAssembly() << "\n";
    ss << generatePowAssembly() << "\n";
    ss << generateResAssembly() << "\n";
    ss << generateValueMemAssembly() << "\n";
    ss << generateMemAssembly() << "\n";
    ss << "main:\n";
    return ss.str();
}