//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <sstream>
#include <string>

std::string generateSerialCommAssembly() {
    std::stringstream ss;
    ss << "; Rotina para transmitir um caractere via Serial.\n";
    ss << "tx_R16:\n";
    ss << "    push    R16\n";
    ss << "tx_R16_aguarda:\n";
    ss << "    lds     R16, UCSR0A\n";
    ss << "    sbrs    R16, 5\n";
    ss << "    rjmp    tx_R16_aguarda\n";
    ss << "    pop     R16\n";
    ss << "    sts     UDR0, R16\n";
    ss << "    ret\n";
    return ss.str();
}
