//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <string>
#include <sstream>

std::string generateIntDivisionAssembly() {
    std::stringstream ss;
    ss << "; int_div: Divisão inteira por restauração (unsigned, 16-bit)\n";
    ss << "; Entrada:\n";
    ss << ";   r25:r24 = dividendo (16 bits)\n";
    ss << ";   r23:r22 = divisor   (16 bits)\n";
    ss << "; Saída:\n";
    ss << ";   r25:r24 = quociente  (16 bits)\n";
    ss << ";   r21:r20 = resto      (16 bits)\n\n";

    ss << "int_div:\n";
    ss << "    clr   r21         ; remainder high = 0\n";
    ss << "    clr   r20         ; remainder low  = 0\n";
    ss << "    ldi   r16, 16     ; 16 iteracoes\n";
    ss << "div_loop:\n";
    // Passo 1: shift left (r21:r20 : r25:r24) em 32 bits
    ss << "    rol   r21\n";
    ss << "    rol   r20\n";
    ss << "    rol   r25\n";
    ss << "    rol   r24\n";
    // Passo 2: comparar remainder (r21:r20) com divisor (r23:r22)
    ss << "    cp    r21, r23\n";
    ss << "    cpc   r20, r22\n";
    ss << "    brlo  no_sub\n";
    ss << "    sub   r20, r22\n";
    ss << "    sbc   r21, r23\n";
    ss << "    ori   r24, 0x01\n";
    ss << "no_sub:\n";
    // Decrementa e repete
    ss << "    dec   r16\n";
    ss << "    brne  div_loop\n";
    ss << "    ret\n";

    return ss.str();
}




