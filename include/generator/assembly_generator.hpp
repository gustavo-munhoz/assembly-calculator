//
// Created by Gustavo Munhoz Correa on 20/03/25.
//

#ifndef ASSEMBLY_GENERATOR_HPP
#define ASSEMBLY_GENERATOR_HPP

#include <string>

std::string parseExpression(const std::string &s, int &pos, std::string &result, int lineIndex);
std::vector<std::string> extractRawCommandsFromFile(const std::string &filename);
std::string generateFullHeaderAssembly();

namespace AssemblyGenerator {
    std::pair<uint8_t, uint8_t> floatToIEEE754Half(float value);

    std::string generateLoadOperandAssembly(
            const std::string &operand,
            const std::string &regLow,
            const std::string &regHigh
    );

    std::string generateArithmeticOperationCallAssembly(
            const std::string &left,
            const std::string &right,
            const std::string &label,
            const std::string &temp
    );

    std::string generateOperationAssembly(
        const std::string &op,
        const std::string &left,
        const std::string &right,
        const std::string &temp,
        int lineIndex
    );
}

#endif // ASSEMBLY_GENERATOR_HPP
