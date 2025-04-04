//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <string>
#include<sstream>

/// Cria o código em assembly para fazer a operação de subtração
/// de números float 16 bits no formato IEEE754.
std::string generateSubAssembly() {
    std::stringstream ss;
    ss << ";-----------------------------------------------------\n";
    ss << "; sub_f16: Subtração de dois half-precision IEEE-754\n";
    ss << "; Entradas: Operando A em R25:R24, Operando B em R23:R22\n";
    ss << "; Saída: Resultado em R25:R24\n";
    ss << ";-----------------------------------------------------\n\n";
    ss << "sub_f16:\n";
    ss << "    ldi R16, 0x80\n";
    ss << "    eor R23, R16\n";
    ss << "    \n";
    ss << "    rcall add_f16\n";
    ss << "    ret\n";

    return ss.str();
}