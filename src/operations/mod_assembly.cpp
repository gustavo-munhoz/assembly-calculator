//
// Created by Gustavo Munhoz Correa on 31/03/25.
//

#include <string>
#include <sstream>

/// Cria o código em assembly para fazer a operação de resto
/// de números float 16 bits no formato IEEE754.
std::string generateModAssembly() {
    std::stringstream ss;
    ss << ";-----------------------------------------------------\n";
    ss << "; mod_f16: Resto da divisão de float16\n";
    ss << "; Entrada A: r25:r24\n";
    ss << "; Entrada B: r23:r22\n";
    ss << "; Saida:     r25:r24 = A % B = A - trunc(A/B) * B\n";
    ss << "; Usa:       Registradores conforme funções chamadas.\n";
    ss << "; Chama:     div_int_f16, mul_f16, sub_f16\n";
    ss << ";-----------------------------------------------------\n";
    ss << "mod_f16:\n";
    ss << "    ; --- Salvar Registradores ---\n";
    ss << "    push r0\n"; ss << "    push r1\n";
    ss << "    push r2\n"; ss << "    push r3\n";
    ss << "    push r16\n"; ss << "    push r17\n";
    ss << "    push r18\n"; ss << "    push r19\n";
    ss << "    push r20\n"; ss << "    push r21\n";
    ss << "    push r26\n"; ss << "    push r27\n";
    ss << "    push r28\n"; ss << "    push r29\n";
    ss << "    push r30\n"; ss << "    push r31\n";

    ss << "    ; --- Checar Divisor B Zero --- \n";
    ss << "    mov r16, r22         ; B low\n";
    ss << "    mov r17, r23         ; B high\n";
    ss << "    andi r17, 0x7F       ; Limpar bit de sinal de B high\n";
    ss << "    or r16, r17          ; Checar se magnitude é zero\n";
    ss << "    brne mod_b_not_zero  ; Pular se B != 0\n";
    ss << "    ldi r25, 0x7E\n";
    ss << "    ldi r24, 0x00\n";
    ss << "    rjmp mod_cleanup     ; Pular para restauração e ret\n";
    ss << "mod_b_not_zero:\n\n";

    ss << "    ; Guardar A e B originais na pilha\n";
    ss << "    push r25 ; Salvar A high\n";
    ss << "    push r24 ; Salvar A low\n";
    ss << "    push r23 ; Salvar B high\n";
    ss << "    push r22 ; Salvar B low\n";

    ss << "    ; --- Passo 1: Calcular i = trunc(A / B) --- \n";
    ss << "    call div_int_f16\n\n";

    ss << "    ; Guardar 'i' na pilha\n";
    ss << "    push r25\n";
    ss << "    push r24\n";

    ss << "    ; --- Passo 2: Calcular P = i * B --- \n";
    ss << "    pop r22              ; Pop B low original para r22\n";
    ss << "    pop r23              ; Pop B high original para r23\n";
    ss << "    pop r24              ; Pop i low para r24\n";
    ss << "    pop r25              ; Pop i high para r25\n";
    ss << "    call mul_f16         ; P = i * B fica em r25:r24\n\n";

    ss << "    ; Guardar P na pilha\n";
    ss << "    push r25\n";
    ss << "    push r24\n";

    ss << "    ; --- Passo 3: Calcular R = A - P --- \n";
    ss << "    pop r24              ; Pop A low original para r24\n";
    ss << "    pop r25              ; Pop A high original para r25\n";
    ss << "    pop r22              ; Pop P low para r22\n";
    ss << "    pop r23              ; Pop P high para r23\n";
    ss << "    call sub_f16\n\n";

    ss << "    ; Checar se R' (r25:r24) tem magnitude zero\n";
    ss << "    mov r16, r24         ; R' low\n";
    ss << "    mov r17, r25         ; R' high\n";
    ss << "    andi r17, 0x7F       ; Ignorar sinal de R'\n";
    ss << "    or r16, r17          ; r16 == 0 se magnitude zero\n";
    ss << "    brne mod_flip_sign   ; Pular para inverter se não for zero\n";
    ss << "    ; Magnitude é zero, não fazer nada\n";
    ss << "    rjmp mod_sign_done\n";
    ss << "mod_flip_sign:\n";
    ss << "    ; Resultado R' não é zero, inverter seu bit de sinal\n";
    ss << "    ldi r16, 0x80\n";
    ss << "    eor r25, r16         ; Inverte bit 7 de r25\n";
    ss << "mod_sign_done:\n";
    ss << "    ; O resultado final R (com sinal correto) está em r25:r24\n";

    ss << "mod_cleanup:\n";
    ss << "    pop r31\n"; ss << "    pop r30\n";
    ss << "    pop r29\n"; ss << "    pop r28\n";
    ss << "    pop r27\n"; ss << "    pop r26\n";
    ss << "    pop r21\n"; ss << "    pop r20\n";
    ss << "    pop r19\n"; ss << "    pop r18\n";
    ss << "    pop r17\n"; ss << "    pop r16\n";
    ss << "    pop r3\n"; ss << "    pop r2\n";
    ss << "    pop r1\n"; ss << "    pop r0\n";
    ss << "    ret\n";

    return ss.str();
}
