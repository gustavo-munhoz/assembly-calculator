//
// Created by Gustavo Munhoz Correa on 02/04/25.
//

#import <string>
#import <sstream>

namespace TempVarManager {
    static int count = 0;
    static std::stringstream tempVariablesStream;

    std::string newTemp() {
        std::string tempName = "T" + std::to_string(++count);
        tempVariablesStream << tempName << "_L: .byte 1\n";
        tempVariablesStream << tempName << "_H: .byte 1\n";
        return tempName;
    }

    std::string getTempVariables() {
        return tempVariablesStream.str();
    }
}