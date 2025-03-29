//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <string>
#include <sstream>

std::string generateAddAssembly() {
    std::stringstream ss;
    ss << "; int_add: Soma inteira simples de 16 bits.\n";
    ss << "; Entrada:\n";
    ss << ";   r25:r24 = operando esquerdo (16 bits)\n";
    ss << ";   r23:r22 = operando direito (16 bits)\n";
    ss << "; Saída:\n";
    ss << ";   r25:r24 = resultado da soma\n";
    ss << "int_add:\n";
    ss << "    add   r24, r22    ; soma o byte inferior\n";
    ss << "    adc   r25, r23    ; soma o byte superior com carry\n";
    ss << "    ret\n";
    return ss.str();
}