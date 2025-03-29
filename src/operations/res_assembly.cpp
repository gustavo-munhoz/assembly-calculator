//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <string>
#include <sstream>

std::string generateResAssembly() {
    std::stringstream ss;
    ss << ";-----------------------------------------------------\n";
    ss << "; res_op: Recupera o resultado da linha N anterior.\n";
    ss << "; Entrada:\n";
    ss << ";   r24 = índice N (número de linhas anteriores)\n";
    ss << "; Saída:\n";
    ss << ";   r24:r25 = resultado (16 bits) armazenado em 'results' na posição (N*2)\n";
    ss << ";-----------------------------------------------------\n";
    ss << "\n";
    ss << "res_op:\n";
    ss << "    lsl   r24         ; Multiplica N por 2 (offset em bytes)\n";
    ss << "    ldi   r30, lo8_results\n";
    ss << "    ldi   r31, hi8_results\n";
    ss << "    add   r30, r24    ; Adiciona o offset ao endereço base\n";
    ss << "    ld    r24, Z+     ; Carrega o byte baixo do resultado\n";
    ss << "    ld    r25, Z      ; Carrega o byte alto do resultado\n";
    ss << "    ret\n";
    return ss.str();
}
