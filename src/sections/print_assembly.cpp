//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <sstream>
#include <string>

/// Cria o código em assembly para o envio de valores
/// pelo monitor serial, com baud rate de 2400.
std::string generatePrintF16Assembly() {
    std::stringstream ss;

    ss << "; ---------------------------------------------------------------\n";
    ss << "; Rotina para imprimir o valor em R25:R24 como HEX (f16 IEEE 754)\n";
    ss << "; ---------------------------------------------------------------\n\n";

    ss << "print_f16:\n";
    ss << " push R16\n";
    ss << " push R17\n";
    ss << " push R18\n";
    ss << " push R19\n";
    ss << " push R24\n";
    ss << " push R25\n\n";

    ss << " ldi R16, '0'\n";
    ss << " rcall tx_R16\n";
    ss << " ldi R16, 'x'\n";
    ss << " rcall tx_R16\n\n";

    ss << " mov R16, R25\n";
    ss << " rcall print_hex8\n\n";

    ss << " mov R16, R24\n";
    ss << " rcall print_hex8\n\n";

    ss << " ldi R16, 0x0D ; CR\n";
    ss << " rcall tx_R16\n";
    ss << " ldi R16, 0x0A ; LF\n";
    ss << " rcall tx_R16\n\n";

    ss << " pop R25\n";
    ss << " pop R24\n";
    ss << " pop R19\n";
    ss << " pop R18\n";
    ss << " pop R17\n";
    ss << " pop R16\n";
    ss << " ret\n\n";

    ss << "print_hex8:\n";
    ss << " push R17\n\n";

    ss << " mov R17, R16\n\n";

    ss << " swap R16\n";
    ss << " andi R16, 0x0F\n";
    ss << " rcall nibble2hex\n\n";

    ss << " mov R16, R17\n";
    ss << " andi R16, 0x0F\n";
    ss << " rcall nibble2hex\n\n";

    ss << " pop R17\n";
    ss << " ret\n\n";

    ss << "nibble2hex:\n";
    ss << " cpi R16, 10\n";
    ss << " brlt nibble_is_dec\n";
    ss << " subi R16, 10\n";
    ss << " subi R16, -'A'\n";
    ss << " rjmp nibble_print\n\n";

    ss << "nibble_is_dec:\n";
    ss << " subi R16, -'0'\n";
    ss << "nibble_print:\n";
    ss << " rcall tx_R16\n";
    ss << " ret\n\n";

    return ss.str();
}
