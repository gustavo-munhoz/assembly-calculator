//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <string>
#include <sstream>

std::string generateMultAssembly() {
    std::stringstream ss;
    ss << "; int_mul: Multiplicação por adição repetida (unsigned, 16-bit)\n";
    ss << "; Entrada:\n";
    ss << ";   r25:r24 = multiplicando (16 bits)\n";
    ss << ";   r23:r22 = multiplicador (16 bits, assume que o byte alto (r23) é 0)\n";
    ss << "; Saída:\n";
    ss << ";   r25:r24 = produto (16 bits)\n";
    ss << "; A rotina calcula o produto somando repetidamente o multiplicando, o número de vezes dado pelo multiplicador.\n";
    ss << "int_mul:\n";
    ss << "    push    r30            ; Salva r30\n";
    ss << "    push    r31            ; Salva r31\n";
    ss << "    mov     r30, r24       ; r30 <- multiplicando (low)\n";
    ss << "    mov     r31, r25       ; r31 <- multiplicando (high)\n";
    ss << "    clr     r24            ; Zera produto (low) em r24\n";
    ss << "    clr     r25            ; Zera produto (high) em r25\n";
    ss << "    mov     r16, r22       ; Usa o multiplicador (byte baixo) como contador\n";
    ss << "mul_loop:\n";
    ss << "    tst     r16            ; Verifica se o contador chegou a 0\n";
    ss << "    breq    end_mul_loop   ; Se sim, sai do loop\n";
    ss << "    mov     r22, r30       ; Carrega multiplicando em r22 (para int_add)\n";
    ss << "    mov     r23, r31       ; Carrega multiplicando em r23 (para int_add)\n";
    ss << "    RCALL   int_add        ; Produto = produto (em r25:r24) + multiplicando (em r23:r22)\n";
    ss << "    dec     r16            ; Decrementa o contador\n";
    ss << "    rjmp    mul_loop       ; Repete o loop\n";
    ss << "end_mul_loop:\n";
    ss << "    pop     r31            ; Restaura r31\n";
    ss << "    pop     r30            ; Restaura r30\n";
    ss << "    ret\n";
    return ss.str();
}

