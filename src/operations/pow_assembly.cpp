//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <string>
#include <sstream>

std::string generateF16ToUint16Assembly();

/// Cria o código em assembly para fazer a operação de potenciação
/// de números float 16 bits no formato IEEE754.
std::string generatePowAssembly() {
    std::stringstream ss;
    ss << ";-----------------------------------------------------\n";
    ss << "; pow_f16: Potência A^B, onde B é float16 representando int >= 1\n";
    ss << "; Entrada Base A: r25:r24\n";
    ss << "; Entrada Expo B: r23:r22\n";
    ss << "; Saida: r25:r24 = A^B\n";
    ss << "; Usa: r0,r1,r2,r16-r23,r26-r31\n";
    ss << "; Chama: f16_to_uint16, mul_f16\n";
    ss << ";-----------------------------------------------------\n";
    ss << "pow_f16:\n";
    ss << "    push r0\n"; ss << "    push r1\n";
    ss << "    push r2\n"; ss << "    push r3\n";
    ss << "    push r16\n"; ss << "    push r17\n"; ss << "    push r18\n";
    ss << "    push r19\n"; ss << "    push r20\n"; ss << "    push r21\n";
    ss << "    push r22\n"; ss << "    push r23\n"; ss << "    push r26\n";
    ss << "    push r27\n"; ss << "    push r28\n"; ss << "    push r29\n";
    ss << "    push r30\n"; ss << "    push r31\n";

    ss << "    ; --- Converter Expoente B (float) para uint16 --- \n";
    ss << "    call f16_to_uint16   ; Entrada r23:r22, Saída b_int em r27:r26\n\n";

    ss << "    ; --- Salvar Base A (CurrentPower) e Inicializar Result --- \n";
    ss << "    mov r21, r24         ; Copiar A low para CurrentPower low (r21)\n";
    ss << "    mov r20, r25         ; Copiar A high para CurrentPower high (r20)\n";

    ss << "    ; --- Checar se b_int == 1 --- \n";
    ss << "    ldi r16, 1           ; Comparar b_int com 1\n";
    ss << "    ldi r17, 0\n";
    ss << "    cp r26, r16          ; Compara low byte\n";
    ss << "    cpc r27, r17         ; Compara high byte\n";
    ss << "    breq pow_cleanup     ; Se b_int == 1, A (resultado) já está em r25:r24. Fim.\n\n";

    ss << "    ; --- Inicializar Resultado = 1.0 --- \n";
    ss << "    ldi r25, 0x3C        ; Result high = 0x3C\n";
    ss << "    ldi r24, 0x00        ; Result low = 0x00\n\n";

    ss << "    ; --- Loop de Exponenciação por Quadrado --- \n";
    ss << "pow_loop:\n";
    ss << "    ; Checar se b_int == 0\n";
    ss << "    mov r16, r26         ; Usar r16 como temp\n";
    ss << "    or r16, r27          ; Testar se r27:r26 é zero\n";
    ss << "    breq pow_loop_end    ; Se b_int == 0, fim do loop\n";

    ss << "    ; Checar se b_int é ímpar (testar bit 0 de r26)\n";
    ss << "    sbrc r26, 0          ; Pular instrução seguinte se bit 0 de r26 for 0 (par)\n";
    ss << "    rcall pow_mult_result; Se for ímpar, multiplicar Result por CurrentPower\n";

    ss << "pow_square_base_prep:\n";
    ss << "    ; --- Calcular CurrentPower = CurrentPower * CurrentPower --- \n";
    ss << "    ; Guardar Result (r25:r24) na pilha antes de usar regs para CP*CP\n";
    ss << "    push r25\n";
    ss << "    push r24\n";

    ss << "    ; CP está em r20:r21\n";
    ss << "    mov r25, r20         ; Mover CP para entrada A (r25:r24)\n";
    ss << "    mov r24, r21\n";
    ss << "    mov r23, r20         ; Mover CP para entrada B (r23:r22)\n";
    ss << "    mov r22, r21\n";
    ss << "    call mul_f16         ; r25:r24 = CP * CP\n";
    ss << "    ; Guardar novo CurrentPower em r20:r21\n";
    ss << "    mov r20, r25\n";
    ss << "    mov r21, r24\n";

    ss << "    ; Restaurar Result da pilha para r25:r24\n";
    ss << "    pop r24\n";
    ss << "    pop r25\n";

    ss << "    ; --- Shift b_int >> 1 --- \n";
    ss << "    lsr r26              ; Desloca low byte para direita\n";
    ss << "    ror r27              ; Rotaciona high byte com carry do lsr\n";

    ss << "    rjmp pow_loop        ; Volta ao início do loop\n\n";


    ss << "; --- Sub-rotina para Result = Result * CurrentPower --- \n";
    ss << "; Chamada com rcall, precisa retornar com ret\n";
    ss << "; Entradas: Result(r25:r24), CP(r20:r21)\n";
    ss << "; Saida: Novo Result(r25:r24)\n";
    ss << "; Clobbers: r22, r23, e regs usados por mul_f16\n";
    ss << "pow_mult_result:\n";
    ss << "    mov r23, r20\n";
    ss << "    mov r22, r21\n";
    ss << "    ; Result (r25:r24) já está na entrada A\n";
    ss << "    call mul_f16         ; r25:r24 = Result * CP\n";
    ss << "    ret\n\n";


    ss << "pow_loop_end:\n";
    ss << "    ; O resultado final está em r25:r24\n";

    ss << "pow_cleanup:\n";
    ss << "    pop r31\n"; ss << "    pop r30\n"; ss << "    pop r29\n";
    ss << "    pop r28\n"; ss << "    pop r27\n"; ss << "    pop r26\n";
    ss << "    pop r23\n"; ss << "    pop r22\n"; ss << "    pop r21\n";
    ss << "    pop r20\n"; ss << "    pop r19\n"; ss << "    pop r18\n";
    ss << "    pop r17\n"; ss << "    pop r16\n"; ss << "    pop r3\n";
    ss << "    pop r2\n"; ss << "    pop r1\n"; ss << "    pop r0\n";
    ss << "    ret\n";

    ss << generateF16ToUint16Assembly();

    return ss.str();
}

std::string generateF16ToUint16Assembly() {
    std::stringstream ss;
    ss << ";-----------------------------------------------------\n";
    ss << "; f16_to_uint16: Converte float16 para uint16_t\n";
    ss << "; Entrada: r23:r22 (float16)\n";
    ss << "; Saída:   r27:r26 (uint16_t)\n";
    ss << "; Modifica: r16, r17, r18, r19\n";
    ss << ";-----------------------------------------------------\n";
    ss << "f16_to_uint16:\n";
    ss << "    mov r18, r23        ; Byte alto de B\n";
    ss << "    andi r18, 0x7C      ; Isolar bits do expoente\n";
    ss << "    lsr r18\n";
    ss << "    lsr r18             ; r18 = Exp (15 a 30)\n\n";

    ss << "    ; Extrair mantissa M (10 bits)\n";
    ss << "    mov r16, r22        ; M7-M0\n";
    ss << "    mov r17, r23\n";
    ss << "    andi r17, 0x03      ; M9-M8\n\n";

    ss << "    ; Construir mantissa inteira de 11 bits: IntM = (1 << 10) | M\n";
    ss << "    ori r17, 0x04       ; Adicionar '1' implícito (bit 10 = bit 2 de r17)\n";
    ss << "    ; Mantissa IntM em r17:r16\n\n";

    ss << "    ; Calcular quantidade de shift: shift = E - 10 = (Exp - 15) - 10 = Exp - 25\n";
    ss << "    mov r19, r18        ; r19 = Exp\n";
    ss << "    subi r19, 25        ; r19 = shift = E - 10\n";
    ss << "    ; Se shift >= 0 (E >= 10), shift left. Se shift < 0 (E < 10), shift right.\n\n";

    ss << "    cpi r19, 0\n";
    ss << "    brge f16tu16_lsl    ; Se shift >= 0, pular para left shift\n\n";

    ss << "    ; --- Shift Direita (shift < 0, E < 10) ---\n";
    ss << "f16tu16_rsr:\n";
    ss << "    neg r19             ; shift_count = -shift = 10 - E\n";
    ss << "f16tu16_rsr_loop:\n";
    ss << "    tst r19\n";
    ss << "    breq f16tu16_done   ; Se contador == 0, terminou\n";
    ss << "    lsr r17             ; Shift IntM (r17:r16) >> 1\n";
    ss << "    ror r16\n";
    ss << "    dec r19\n";
    ss << "    rjmp f16tu16_rsr_loop\n\n";

    ss << "    ; --- Shift Esquerda (shift >= 0, E >= 10) ---\n";
    ss << "f16tu16_lsl:\n";
    ss << "f16tu16_lsl_loop:\n";
    ss << "    tst r19\n";
    ss << "    breq f16tu16_done   ; Se contador == 0, terminou\n";
    ss << "    lsl r16             ; Shift IntM (r17:r16) << 1\n";
    ss << "    rol r17\n";
    ss << "    dec r19\n";
    ss << "    rjmp f16tu16_lsl_loop\n\n";

    ss << "f16tu16_done:\n";
    ss << "    ; Resultado uint16 está em r17:r16\n";
    ss << "    mov r26, r16        ; Mover resultado low byte para r26\n";
    ss << "    mov r27, r17        ; Mover resultado high byte para r27\n";
    ss << "    ret                 ; Retorna uint16_t em r27:r26\n";

    return ss.str();
}