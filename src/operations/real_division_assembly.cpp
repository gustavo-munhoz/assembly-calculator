//
// Created by Gustavo Munhoz Correa on 30/03/25.
//

#include <string>
#include <sstream>

/// Cria o código em assembly para fazer a operação de divisão real
/// de números float 16 bits no formato IEEE754.
std::string generateRealDivisionAssembly() {
    std::stringstream ss;
    ss << ";-----------------------------------------------------\n";
    ss << "; div_f16: Divisão em ponto flutuante IEEE 754 de 16 bits\n";
    ss << "; Entrada:\n";
    ss << ";   r25:r24 = dividendo\n";
    ss << ";   r23:r22 = divisor\n";
    ss << "; Saída:\n";
    ss << ";   r25:r24 = resultado\n";
    ss << "; Registradores usados: r0, r1, r2, r3, r16-r21\n";
    ss << ";-----------------------------------------------------\n";
    ss << "div_f16:\n";
    ss << "    push r0\n";
    ss << "    push r1\n";
    ss << "    push r2\n";
    ss << "    push r3\n";
    ss << "    push r16\n";
    ss << "    push r17\n";
    ss << "    push r18\n";
    ss << "    push r19\n";
    ss << "    push r20\n";
    ss << "    push r21\n";

    ss << "    ; Verificar se divisor é zero\n";
    ss << "    mov  r16, r22\n";
    ss << "    or   r16, r23\n";
    ss << "    brne divisor_not_zero\n";

    ss << "    ; Divisão por zero - retornar infinito com sinal apropriado\n";
    ss << "    mov  r16, r25\n";
    ss << "    andi r16, 0x80\n";
    ss << "    ori  r16, 0x7C\n";
    ss << "    mov  r25, r16\n";
    ss << "    ldi  r24, 0x00\n";
    ss << "    rjmp div_f16_end\n";

    ss << "divisor_not_zero:\n";
    ss << "    ; Extrair sinais\n";
    ss << "    mov  r16, r25\n";
    ss << "    andi r16, 0x80\n";
    ss << "    mov  r17, r23\n";
    ss << "    andi r17, 0x80\n";
    ss << "    mov  r18, r16\n";
    ss << "    eor  r18, r17\n";
    ss << "    mov  r20, r18\n";

    ss << "    ; Extrair magnitude (remover bit de sinal)\n";
    ss << "    mov  r16, r25\n";
    ss << "    andi r16, 0x7F\n";
    ss << "    mov  r17, r23\n";
    ss << "    andi r17, 0x7F\n";

    ss << "    ; Verificar se dividendo é zero (após remover sinal)\n";
    ss << "    mov  r18, r16\n";
    ss << "    or   r18, r24\n";
    ss << "    brne dividend_not_zero\n";

    ss << "    ; Dividendo é zero - retornar zero com sinal apropriado\n";
    ss << "    mov  r25, r20\n";
    ss << "    ldi  r24, 0x00\n";
    ss << "    rjmp div_f16_end\n";

    ss << "dividend_not_zero:\n";
    ss << "    ; Extrair expoentes\n";
    ss << "    mov  r18, r16\n";
    ss << "    andi r18, 0x7C\n";
    ss << "    lsr  r18\n";
    ss << "    lsr  r18\n";
    ss << "    mov  r19, r17\n";
    ss << "    andi r19, 0x7C\n";
    ss << "    lsr  r19\n";
    ss << "    lsr  r19\n";

    ss << "    ; Calcular expoente do resultado: exp_res = exp_divd - exp_divs + 15\n";
    ss << "    sub  r18, r19\n";
    ss << "    subi r18, -15\n";

    ss << "    ; Extrair mantissas (11 bits com implícito)\n";
    ss << "    ; Dividendo -> r1:r0\n";
    ss << "    mov  r19, r16\n";
    ss << "    andi r19, 0x03\n";
    ss << "    ori  r19, 0x04\n";
    ss << "    mov  r1, r19\n";
    ss << "    mov  r0, r24\n";
    ss << "    ; Divisor -> r3:r2\n";
    ss << "    mov  r19, r17\n";
    ss << "    andi r19, 0x03\n";
    ss << "    ori  r19, 0x04\n";
    ss << "    mov  r3, r19\n";
    ss << "    mov  r2, r22\n";

    ss << "    clr  r16            ; Limpar registrador baixo do quociente\n";
    ss << "    clr  r17            ; Limpar registrador alto do quociente\n";
    ss << "    ldi  r19, 16        ; Contador de iterações (16 para quociente de 16 bits)\n";

    ss << "division_loop_new:\n";
    ss << "    ; Comparar Resto (r1:r0) com Divisor (r3:r2)\n";
    ss << "    cp   r0, r2\n";
    ss << "    cpc  r1, r3\n";
    ss << "    brcc sub_ok_new     ; Pular se R >= D (Carry Limpo)\n";

    ss << "    ; R < D : Bit do quociente é 0\n";
    ss << "    clc                 ; Garantir Carry = 0 para shift no quociente\n";
    ss << "    rjmp shift_q_new    ; Pular subtração\n";

    ss << "sub_ok_new:\n";
    ss << "    ; R >= D : Bit do quociente é 1\n";
    ss << "    sub  r0, r2         ; R = R - D\n";
    ss << "    sbc  r1, r3\n";
    ss << "    sec                 ; Setar Carry = 1 para shift no quociente\n";

    ss << "shift_q_new:\n";
    ss << "    ; Deslocar Quociente (r17:r16) à esquerda e inserir bit (Carry)\n";
    ss << "    rol  r16\n";
    ss << "    rol  r17\n";

    ss << "shift_r_new:\n";
    ss << "    lsl  r0\n";
    ss << "    rol  r1\n";

    ss << "    ; Decrementar contador e loop\n";
    ss << "    dec  r19\n";
    ss << "    brne division_loop_new\n";

    ss << "    ; Normalizar o quociente (r17:r16) e ajustar o expoente (r18)\n";
    ss << "    mov r19, r16\n";
    ss << "    or r19, r17\n";
    ss << "    breq result_is_zero\n";

    ss << "div_normalize_loop:\n";
    ss << "    tst r17\n";
    ss << "    brne msb_in_r17\n";
    ss << "    mov r17, r16\n";
    ss << "    clr r16\n";
    ss << "    subi r18, 8\n";
    ss << "    tst r17\n";
    ss << "    breq result_is_zero\n";
    ss << "    rjmp div_normalize_loop\n";

    ss << "msb_in_r17:\n";
    ss << "    mov r19, r17\n";
    ss << "check_msb_pos:\n";
    ss << "    sbrc r19, 7        ; Bit 7 de r17 é a posição do bit implícito (1.xxxx)\n";
    ss << "    rjmp msb_normalized\n";
    ss << "    tst r19\n";
    ss << "    breq msb_normalized ; Parar se ficou zero\n";
    ss << "    lsl  r16\n";
    ss << "    rol  r17\n";
    ss << "    lsl  r19\n";
    ss << "    dec  r18           ; Ajustar (diminuir) expoente para cada LSL\n";
    ss << "    rjmp check_msb_pos\n";

    ss << "msb_normalized:\n";
    ss << "    ; Verificar limites do expoente (Overflow / Underflow)\n";
    ss << "check_exponent:\n";
    ss << "    cpi  r18, 31\n";
    ss << "    brsh exponent_overflow\n";
    ss << "    cpi  r18, 1\n";
    ss << "    brlo exponent_underflow\n";

    ss << "    ; Expoente válido (1 a 30)\n";
    ss << "    rjmp construct_result\n";

    ss << "exponent_overflow:\n";
    ss << "    mov  r25, r20\n";
    ss << "    ori  r25, 0x7C\n";
    ss << "    ldi  r24, 0x00\n";
    ss << "    rjmp div_f16_end\n";

    ss << "result_is_zero:\n";
    ss << "exponent_underflow:\n";
    ss << "    mov  r25, r20\n";
    ss << "    ldi  r24, 0x00\n";
    ss << "    rjmp div_f16_end\n";

    ss << "construct_result:\n";
    ss << "    ; Construir número normalizado\n";
    ss << "    ; Expoente ajustado em r18, mantissa normalizada em r17:r16, sinal em r20.\n";
    ss << "    ; r17 = 1 M9 M8 M7 M6 M5 M4 M3\n";
    ss << "    ; r16 = M2 M1 M0 X X X X X\n";

    ss << "    ; Preparar byte alto (r25): Sinal | Expoente | M9 M8\n";
    ss << "    mov  r25, r18       ; Expoente (1-30)\n";
    ss << "    lsl  r25\n";
    ss << "    lsl  r25            ; Posicionar em bits 6-2\n";
    ss << "    andi r25, 0x7C      ; Isolar bits do expoente\n";
    ss << "    mov  r19, r17       ; r17 = 1 M9 M8 ...\n";
    ss << "    andi r19, 0x60      ; Isolar M9 M8 em posições 6,5\n";
    ss << "    lsr  r19\n";
    ss << "    lsr  r19\n";
    ss << "    lsr  r19\n";
    ss << "    lsr  r19\n";
    ss << "    lsr  r19            ; Mover M9 M8 para posições 1,0\n";
    ss << "    or   r25, r19       ; Combinar expoente e M9 M8\n";
    ss << "    or   r25, r20       ; Adicionar bit de sinal\n";

    ss << "    ; Preparar byte baixo (r24): M7 M6 M5 M4 M3 M2 M1 M0\n";
    ss << "    mov  r19, r17       ; r17 = 1 M9 M8 M7 M6 M5 M4 M3\n";
    ss << "    andi r19, 0x1F     ; Isolar M7..M3\n";
    ss << "    lsl  r19\n";
    ss << "    lsl  r19\n";
    ss << "    lsl  r19           ; r19 = M7M6M5M4M3000\n";
    ss << "    mov  r24, r19       ; Mover para resultado parcial\n";
    ss << "    mov  r19, r16       ; r16 = M2 M1 M0 X ...\n";
    ss << "    andi r19, 0xE0     ; Isolar M2 M1 M0\n";
    ss << "    lsr  r19\n";
    ss << "    lsr  r19\n";
    ss << "    lsr  r19           ; r19 = 000M2M1M000\n";
    ss << "    or   r24, r19       ; Combinar -> r24 = M7M6M5M4M3M2M1M0\n";

    ss << "div_f16_end:\n";
    ss << "    ; Restaurar registradores\n";
    ss << "    pop r21\n";
    ss << "    pop r20\n";
    ss << "    pop r19\n";
    ss << "    pop r18\n";
    ss << "    pop r17\n";
    ss << "    pop r16\n";
    ss << "    pop r3\n";
    ss << "    pop r2\n";
    ss << "    pop r1\n";
    ss << "    pop r0\n";
    ss << "    ret\n";

    return ss.str();
}