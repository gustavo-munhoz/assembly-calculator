//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <string>
#include <sstream>

/// Cria o código em assembly para fazer a operação de divisão inteira
/// de números float 16 bits no formato IEEE754.
std::string generateIntDivisionAssembly() {
    std::stringstream ss;
    ss << ";-----------------------------------------------------\n";
    ss << "; div_int_f16: Divisão inteira de float16\n";
    ss << "; Entrada A: r25:r24 (float16)\n";
    ss << "; Entrada B: r23:r22 (float16)\n";
    ss << "; Saida:     r25:r24 = float16(trunc(A/B))\n";
    ss << "; Callee-Saved: r18, r20, r21, r26, r27\n";
    ss << "; Volatile: r16, r17\n";
    ss << "; Chama: div_f16\n";
    ss << ";-----------------------------------------------------\n";
    ss << "div_int_f16:\n";
    ss << "    push r18\n";
    ss << "    push r20\n";
    ss << "    push r21\n";
    ss << "    push r26\n";
    ss << "    push r27\n";

    ss << "\n    ; 1. Chamar divisão float normal A/B\n";
    ss << "    call div_f16         ; Resultado R em r25:r24\n\n";

    ss << "    ; 2. Salvar sinal do resultado R -> r20\n";
    ss << "    mov r20, r25\n";
    ss << "    andi r20, 0x80       ; r20 = Sinal de R\n\n";

    ss << "    ; 3. Checar se resultado R é Zero\n";
    ss << "    mov r16, r24         ; Byte baixo de R\n";
    ss << "    mov r17, r25\n";
    ss << "    andi r17, 0x7F       ; Byte alto de R sem sinal\n";
    ss << "    or r16, r17          ; r16 == 0 se R for +/- 0.0\n";
    ss << "    brne check_inf_nan   ; Pular se R não for zero\n";
    ss << "    mov r25, r20         ; Restaurar sinal em r25\n";
    ss << "    ldi r24, 0x00        ; Garantir r24 zero\n";
    ss << "    rjmp div_int_cleanup ; Resultado é +/- 0.0, fim.\n\n";

    ss << "check_inf_nan:\n";
    ss << "    ; 4. Checar Inf/NaN (Expoente == 31) -> r26\n";
    ss << "    mov r26, r25         ; Pegar byte alto de R\n";
    ss << "    andi r26, 0x7C       ; Isolar bits do expoente (posições 6-2)\n";
    ss << "    cpi r26, 0x7C        ; Comparar com padrão Inf/NaN (11111 << 2)\n";
    ss << "    breq div_int_cleanup ; Se Inf/NaN, retornar R como está. Fim.\n\n";

    ss << "    ; 5. Extrair expoente com bias -> r26 (já isolado, só shiftar)\n";
    ss << "    lsr r26\n";
    ss << "    lsr r26              ; r26 = Expoente com bias (0-30)\n\n";

    ss << "    ; 6. Calcular E = Exp - Bias -> r18\n";
    ss << "    mov r18, r26\n";
    ss << "    subi r18, 15         ; r18 = E = Exp - 15. Flags N,Z,C,V,S atualizados.\n\n";

    ss << "    ; 7. Se E < 0 (Exp < 15), |R| < 1.0. Resultado truncado é +/- Zero\n";
    ss << "    brge E_ge_zero       ; Pular se E >= 0 (com sinal)\n";
    ss << "    ; E < 0 (|R| < 1.0)\n";
    ss << "    mov r25, r20         ; Resultado = +/- Zero\n";
    ss << "    ldi r24, 0x00\n";
    ss << "    rjmp div_int_cleanup ; Fim.\n\n";

    ss << "E_ge_zero:\n";
    ss << "    ; E é >= 0 aqui.\n";
    ss << "    ; 8. Se E >= 10 (Exp >= 25), R já é inteiro. Retornar R.\n";
    ss << "    cpi r18, 10\n";
    ss << "    brlt E_lt_10         ; Pular se E < 10 (com sinal)\n";
    ss << "    ; E >= 10. R já é inteiro.\n";
    ss << "    rjmp div_int_cleanup ; Fim.\n\n";

    ss << "E_lt_10:\n";
    ss << "    ; --- Truncar Parte Fracionária (E está entre 0 e 9) --- \n";
    ss << "    ; 9. Calcular k = 10 - E -> r27\n";
    ss << "    ldi r27, 10\n";
    ss << "    sub r27, r18         ; r27 = k = 10 - E (k estará entre 1 e 10)\n\n";

    ss << "    ; 10. Mascarar r24 (M7-M0) - Zerar os k bits inferiores\n";
    ss << "    ldi r16, 0xFF        ; Máscara inicial\n";
    ss << "    mov r17, r27         ; Contador k\n";
    ss << "mask_r24_loop:\n";
    ss << "    tst r17              ; Contador k == 0?\n";
    ss << "    breq mask_r24_done   ; Sim, loop terminou.\n";
    ss << "    lsl r16              ; Deslocar máscara para esquerda\n";
    ss << "    dec r17              ; Decrementar contador k\n";
    ss << "    rjmp mask_r24_loop   ; Repetir o loop\n";
    ss << "mask_r24_done:\n";
    ss << "    and r24, r16         ; Aplicar máscara em r24\n\n";

    ss << "    ; 11. Mascarar r25 (M9, M8 - bits 1,0)\n";
    ss << "    cpi r27, 9           ; Comparar k com 9\n";
    ss << "    brlo mask_r25_done   ; Se k < 9 (E >= 2), M9 e M8 são mantidos.\n";

    ss << "    ldi r17, 0x02        ; Máscara para MANTER M9 (bit 1) - default para k=9\n";
    ss << "    cpi r27, 10          ; Comparar k com 10\n";
    ss << "    brne apply_mask_r25_final ; Pular se k=9\n";
    ss << "    ; k = 10 (E=0), zerar M9 e M8\n";
    ss << "    ldi r17, 0x00        ; Máscara para MANTER NADA (M9=0, M8=0)\n";

    ss << "apply_mask_r25_final:\n";
    ss << "    ; r17 contém a máscara de BITS A MANTER (02 ou 00) para M9, M8\n";
    ss << "    mov r16, r25         ; Copiar r25 original para r16 (Volatile)\n";
    ss << "    andi r16, 0x03       ; Isolar M9, M8 originais em r16\n";
    ss << "    and r16, r17         ; Aplicar máscara de MANTER -> r16 = M9', M8'\n";
    ss << "    andi r25, 0xFC       ; Zerar M9, M8 em r25 original (preserva S|Exp)\n";
    ss << "    or r25, r16          ; Combinar S|Exp com M9', M8' corretos\n";

    ss << "mask_r25_done:\n";
    ss << "    ; O resultado truncado está em r25:r24\n";

    ss << "div_int_cleanup:\n";
    ss << "    pop r27\n";
    ss << "    pop r26\n";
    ss << "    pop r21\n";
    ss << "    pop r20\n";
    ss << "    pop r18\n";
    ss << "    ret\n";

    return ss.str();
}