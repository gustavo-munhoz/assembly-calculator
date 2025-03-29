//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <string>
#include <sstream>

std::string generatePowAssembly() {
    std::stringstream ss;

    ss << ";---------------------------------------------------\n";
    ss << "; int_pow: Exponenciação rápida (unsigned 16 bits)\n";
    ss << ";   Entrada:\n";
    ss << ";     r25:r24 = base  (r25 alto, r24 baixo)\n";
    ss << ";     r23:r22 = exponent (16 bits)\n";
    ss << ";   Saída:\n";
    ss << ";     r25:r24 = base^exponent (16 bits)\n";
    ss << ";   Utiliza:\n";
    ss << ";     int_mul  (rotina de multiplicacao 16 bits)\n";
    ss << ";     r19:r18  -> copia de base\n";
    ss << ";     r21:r20  -> acumulador de result\n";
    ss << ";   Destrói:\n";
    ss << ";     r19:r18, r21:r20, r24, r25\n";
    ss << ";---------------------------------------------------\n";
    ss << "int_pow:\n";
    ss << "    ; Copia base (r25:r24) para r19:r18\n";
    ss << "    mov   r19, r25\n";
    ss << "    mov   r18, r24\n\n";
    ss << "    ; result = 1 (r21:r20)\n";
    ss << "    ldi   r21, 0\n";
    ss << "    ldi   r20, 1\n\n";
    ss << "pow_loop:\n";
    ss << "    ; Se exponent == 0 => sai\n";
    ss << "    tst   r22         ; testa parte baixa\n";
    ss << "    brne  exponent_not_zero\n";
    ss << "    tst   r23         ; parte alta\n";
    ss << "    breq  pow_done\n";
    ss << "exponent_not_zero:\n\n";
    ss << "    ; Se (exponent & 1) != 0, ou seja, se for ímpar => result *= base\n";
    ss << "    sbrs  r22, 0      ; se bit 0 de r22 = 0, pula a multiplicacao\n";
    ss << "    rjmp  do_mult\n\n";
    ss << "skip_mult:\n";
    ss << "    ; exponent >>= 1\n";
    ss << "    lsr   r23\n";
    ss << "    ror   r22\n\n";
    ss << "    ; base = base * base\n";
    ss << "    mov   r24, r18   ; carrega base em r24:r25\n";
    ss << "    mov   r25, r19\n";
    ss << "    mov   r22, r18   ; multiplicador (mesmo valor)\n";
    ss << "    mov   r23, r19\n";
    ss << "    rcall int_mul    ; produto sai em r25:r24\n";
    ss << "    ; atualiza base (r19:r18)\n";
    ss << "    mov   r18, r24\n";
    ss << "    mov   r19, r25\n";
    ss << "    rjmp  pow_loop\n\n";
    ss << "do_mult:\n";
    ss << "    ; result = result * base\n";
    ss << "    mov   r24, r20   ; r21:r20 => result\n";
    ss << "    mov   r25, r21\n";
    ss << "    mov   r22, r18   ; r19:r18 => base\n";
    ss << "    mov   r23, r19\n";
    ss << "    rcall int_mul    ; produto sai em r25:r24\n";
    ss << "    ; salva em result\n";
    ss << "    mov   r20, r24\n";
    ss << "    mov   r21, r25\n";
    ss << "    rjmp  skip_mult\n\n";
    ss << "pow_done:\n";
    ss << "    ; resultado final esta em r21:r20\n";
    ss << "    mov   r24, r20\n";
    ss << "    mov   r25, r21\n";
    ss << "    ret\n";

    return ss.str();
}
