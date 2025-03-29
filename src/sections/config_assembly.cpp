//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <sstream>
#include <string>

std::string generateConfigAssembly() {
    std::stringstream ss;
    ss << "; Configuração da USART\n";
    ss << "    store   UBRR0H, 0x01\n";
    ss << "    store   UBRR0L, 0xA0\n";
    ss << "    store   UCSR0A, 0b00100000\n";
    ss << "    store   UCSR0C, 0x06\n";
    ss << "    store   UCSR0B, 0x18\n";
    return ss.str();
}