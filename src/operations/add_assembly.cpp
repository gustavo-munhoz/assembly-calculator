//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <string>
#include <sstream>

/// Cria o código em assembly para fazer a operação de soma
/// de números float 16 bits no formato IEEE754.
std::string generateAddAssembly() {
    std::stringstream ss;
    ss << ";---------------------------------------------------------\n";
    ss << "; add_f16: Adição de dois half-precision IEEE-754\n";
    ss << "; Entradas: Operando A em R25:R24, Operando B em R23:R22\n";
    ss << "; Saída: Resultado em R25:R24\n";
    ss << "; Callee-Saved: r18, r19, r20, r21, r26, r27, r30\n";
    ss << "; Volatile: r2, r16, r17, r28, r29\n";
    ss << ";---------------------------------------------------------\n";
    ss << "add_f16:\n";
    ss << "    push R18\n";
    ss << "    push R19\n";
    ss << "    push R20\n";
    ss << "    push R21\n";
    ss << "    push R26\n";
    ss << "    push R27\n";
    ss << "    push R30\n";

    ss << "    ; Extrair Sinais -> r28, r29 (Volatile)\n";
    ss << "    mov R28, R25\n";
    ss << "    andi R28, 0x80\n";
    ss << "    mov R29, R23\n";
    ss << "    andi R29, 0x80\n\n";

    ss << "    ; Extrair Expoentes -> r26, r27 (Callee-Saved)\n";
    ss << "    mov R26, R25\n";
    ss << "    andi R26, 0x7C\n";
    ss << "    lsr R26\n";
    ss << "    lsr R26\n";
    ss << "    mov R27, R23\n";
    ss << "    andi R27, 0x7C\n";
    ss << "    lsr R27\n";
    ss << "    lsr R27\n\n";

    ss << "    ; Extrair Mantissas -> r19:r18(A), r17:r16(B)\n";
    ss << "    mov R18, R24\n";
    ss << "    mov R19, R25\n";
    ss << "    andi R19, 0x03\n";
    ss << "    tst R26\n";
    ss << "    brne add_norm_a\n";
    ss << "    ldi r16, 1\n";
    ss << "    mov R26, r16\n";
    ss << "    rjmp add_extract_b\n";
    ss << "add_norm_a:\n";
    ss << "    ori R19, 0x04\n";

    ss << "add_extract_b:\n";
    ss << "    mov R16, R22\n";
    ss << "    mov R17, R23\n";
    ss << "    andi R17, 0x03\n";
    ss << "    tst R27\n";
    ss << "    brne add_norm_b\n";
    ss << "    ldi r16, 1\n";
    ss << "    mov R27, r16\n";
    ss << "    rjmp align_mantissas_start\n";
    ss << "add_norm_b:\n";
    ss << "    ori R17, 0x04\n";

    ss << "align_mantissas_start:\n";
    ss << "    cp R26, R27\n";
    ss << "    brsh exp_a_ge_b\n";
    ss << "    ; Expoente B > A, deslocar A à direita\n";
    ss << "    mov R30, R27\n";
    ss << "    mov r2, R27\n";
    ss << "    sub r2, R26\n";
    ss << "shift_a_loop:\n";
    ss << "    tst r2\n";
    ss << "    breq align_done\n";
    ss << "    lsr R19\n";
    ss << "    ror R18\n";
    ss << "    dec r2\n";
    ss << "    rjmp shift_a_loop\n";
    ss << "    rjmp align_done\n\n";
    ss << "exp_a_ge_b:\n";
    ss << "    ; Expoente A >= B, deslocar B à direita\n";
    ss << "    mov R30, R26\n";
    ss << "    mov r2, R26\n";
    ss << "    sub r2, R27\n";
    ss << "shift_b_loop:\n";
    ss << "    tst r2\n";
    ss << "    breq align_done\n";
    ss << "    lsr R17\n";
    ss << "    ror R16\n";
    ss << "    dec r2\n";
    ss << "    rjmp shift_b_loop\n\n";
    ss << "align_done:\n\n";

    ss << "    ; Adicionar / Subtrair Mantissas\n";
    ss << "    ; Guarda Sinal em r21 (Callee-Saved)\n";
    ss << "    cp R28, R29\n";
    ss << "    breq same_signs\n\n";
    ss << "    ; --- Sinais Diferentes ---\n";
    ss << "    mov R20, R19        ; Guardar cópia de r19:r18 para comparação\n";
    ss << "    cp R19, R17\n";
    ss << "    brne mantissa_cmp_decided\n";
    ss << "    cp R18, R16\n";
    ss << "    brne mantissa_cmp_decided\n";
    ss << "    ; Mantissas iguais, resultado é zero\n";
    ss << "    clr R18\n";
    ss << "    clr R19\n";
    ss << "    clr R30\n";
    ss << "    clr r21\n";
    ss << "    rjmp pack_result\n\n";
    ss << "mantissa_cmp_decided:\n";
    ss << "    brlo b_greater_sub\n";
    ss << "a_ge_b_sub:\n";
    ss << "    ; A >= B, subtrair B de A\n";
    ss << "    sub R18, R16\n";
    ss << "    sbc R19, R17\n";
    ss << "    mov r21, R28        ; Sinal de A\n";
    ss << "    rjmp normalize_after_sub\n\n";
    ss << "b_greater_sub:\n";
    ss << "    ; B > A, subtrair A de B\n";
    ss << "    sub R16, R18\n";
    ss << "    sbc R17, R19\n";
    ss << "    mov r21, R29        ; Sinal de B\n";
    ss << "    mov R18, R16        ; Mover resultado para r19:r18\n";
    ss << "    mov R19, R17\n";
    ss << "    rjmp normalize_after_sub\n\n";
    ss << "same_signs:\n";
    ss << "    ; --- Sinais Iguais (Adição) ---\n";
    ss << "    add R18, R16\n";
    ss << "    adc R19, R17\n";
    ss << "    mov r21, R28\n";
    ss << "    rjmp normalize_check_add\n\n";

    ss << "normalize_check_add:\n";
    ss << "    ; Verificar se resultado é zero\n";
    ss << "    mov R16, R19\n";
    ss << "    or R16, R18\n";
    ss << "    brne check_add_overflow\n";
    ss << "    clr R30\n";
    ss << "    rjmp pack_result\n\n";
    ss << "check_add_overflow:\n";
    ss << "    sbrc R19, 3         ; Verificar overflow (bit 11)\n";
    ss << "    rjmp adjust_add_overflow\n";
    ss << "    rjmp add_normalize_loop_entry\n\n";
    ss << "normalize_after_sub:\n";
    ss << "    ; Verificar se resultado é zero\n";
    ss << "    mov R16, R19\n";
    ss << "    or R16, R18\n";
    ss << "    brne add_normalize_loop_entry\n";
    ss << "    clr R30\n";
    ss << "    clr r21             ; Zero positivo\n";
    ss << "    rjmp pack_result\n\n";
    ss << "add_normalize_loop_entry:\n";
    ss << "    ; Normalizar resultado (bit implícito em R19 bit 2)\n";
    ss << "add_normalize_loop:\n";
    ss << "    sbrc R19, 2         ; Verificar se bit implícito está em posição\n";
    ss << "    rjmp normalize_done\n";
    ss << "    ; Deslocar mantissa à esquerda até posicionar bit implícito\n";
    ss << "    lsl R18\n";
    ss << "    rol R19\n";
    ss << "    ; Ajustar expoente\n";
    ss << "    dec R30\n";
    ss << "    ; Verificar denormalização\n";
    ss << "    cpi R30, 1\n";
    ss << "    brsh add_normalize_loop  ; Se R30 >= 1, continuar normalizando\n";
    ss << "handle_denormalized:\n";
    ss << "    ; Resultado denormalizado, retornar zero\n";
    ss << "    clr R19\n";
    ss << "    clr R18\n";
    ss << "    clr R30\n";
    ss << "    rjmp pack_result\n\n";
    ss << "adjust_add_overflow:\n";
    ss << "    lsr R19\n";
    ss << "    ror R18\n";
    ss << "    inc R30\n";
    ss << "    cpi R30, 31\n";
    ss << "    brsh handle_overflow\n";
    ss << "normalize_done:\n";
    ss << "    rjmp pack_result\n\n";
    ss << "handle_overflow:\n";
    ss << "    ldi R19, 0x00\n";
    ss << "    ldi R18, 0x00\n";
    ss << "    ldi R30, 31\n";
    ss << "    rjmp pack_result\n\n";

    ss << "pack_result:\n";
    ss << "    mov R16, R30\n";
    ss << "    lsl R16\n";
    ss << "    lsl R16\n";
    ss << "    andi R16, 0x7C      ; Expoente em bits 6-2\n";
    ss << "    ; Montar byte alto do resultado\n";
    ss << "    mov R25, R16        ; Bits de expoente\n";
    ss << "    mov R16, R19\n";
    ss << "    andi R16, 0x03      ; Bits 1-0 da mantissa alta\n";
    ss << "    or R25, R16         ; Combinar expoente com mantissa alta\n";
    ss << "    or R25, r21         ; Aplicar bit de sinal\n";
    ss << "    mov R24, R18\n";

    ss << "add_cleanup:\n";
    ss << "    pop R30\n";
    ss << "    pop R27\n";
    ss << "    pop R26\n";
    ss << "    pop R21\n";
    ss << "    pop R20\n";
    ss << "    pop R19\n";
    ss << "    pop R18\n";
    ss << "    ret\n";

    return ss.str();
}