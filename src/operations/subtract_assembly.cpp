//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <string>
#include<sstream>

std::string generateSubAssembly() {
    std::stringstream ss;

    ss << "; int_sub: Subtrai dois números inteiros de 16 bits.\n";
    ss << "int_sub:\n";
    ss << "    sub r24, r22\n";
    ss << "    sub r25, r23\n";
    ss << "    ret\n";

    return ss.str();
}