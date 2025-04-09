.include "m328Pdef.inc"

;-----------------------------------------------------------
; Macros e Configurações
;-----------------------------------------------------------
.macro store
    ldi     R16, @1
    sts     @0, R16
.endm

; Configuração da USART
; Configuração da USART
    store   UBRR0H, 0x01
    store   UBRR0L, 0xA0
    store   UCSR0A, 0b00100000
    store   UCSR0C, 0x06
    store   UCSR0B, 0x18

	rjmp main
; ---------------------------------------------------------------
; Rotina para imprimir o valor em R25:R24 como HEX (f16 IEEE 754)
; ---------------------------------------------------------------

print_f16:
 push R16
 push R17
 push R18
 push R19
 push R24
 push R25

 ldi R16, '0'
 rcall tx_R16
 ldi R16, 'x'
 rcall tx_R16

 mov R16, R25
 rcall print_hex8

 mov R16, R24
 rcall print_hex8

 ldi R16, 0x0D ; CR
 rcall tx_R16
 ldi R16, 0x0A ; LF
 rcall tx_R16

 pop R25
 pop R24
 pop R19
 pop R18
 pop R17
 pop R16
 ret

print_hex8:
 push R17

 mov R17, R16

 swap R16
 andi R16, 0x0F
 rcall nibble2hex

 mov R16, R17
 andi R16, 0x0F
 rcall nibble2hex

 pop R17
 ret

nibble2hex:
 cpi R16, 10
 brlt nibble_is_dec
 subi R16, 10
 subi R16, -'A'
 rjmp nibble_print

nibble_is_dec:
 subi R16, -'0'
nibble_print:
 rcall tx_R16
 ret


; Rotina para transmitir um caractere via Serial.
tx_R16:
    push    R16
tx_R16_aguarda:
    lds     R16, UCSR0A
    sbrs    R16, 5
    rjmp    tx_R16_aguarda
    pop     R16
    sts     UDR0, R16
    ret

;---------------------------------------------------------
; add_f16: Adição de dois half-precision IEEE-754
; Entradas: Operando A em R25:R24, Operando B em R23:R22
; Saída: Resultado em R25:R24
; Callee-Saved: r18, r19, r20, r21, r26, r27, r30
; Volatile: r2, r16, r17, r28, r29
;---------------------------------------------------------
add_f16:
    push R18
    push R19
    push R20
    push R21
    push R26
    push R27
    push R30
    ; Extrair Sinais -> r28, r29 (Volatile)
    mov R28, R25
    andi R28, 0x80
    mov R29, R23
    andi R29, 0x80

    ; Extrair Expoentes -> r26, r27 (Callee-Saved)
    mov R26, R25
    andi R26, 0x7C
    lsr R26
    lsr R26
    mov R27, R23
    andi R27, 0x7C
    lsr R27
    lsr R27

    ; Extrair Mantissas -> r19:r18(A), r17:r16(B)
    mov R18, R24
    mov R19, R25
    andi R19, 0x03
    tst R26
    brne add_norm_a
    ldi r16, 1
    mov R26, r16
    rjmp add_extract_b
add_norm_a:
    ori R19, 0x04
add_extract_b:
    mov R16, R22
    mov R17, R23
    andi R17, 0x03
    tst R27
    brne add_norm_b
    ldi r16, 1
    mov R27, r16
    rjmp align_mantissas_start
add_norm_b:
    ori R17, 0x04
align_mantissas_start:
    cp R26, R27
    brsh exp_a_ge_b
    ; Expoente B > A, deslocar A à direita
    mov R30, R27
    mov r2, R27
    sub r2, R26
shift_a_loop:
    tst r2
    breq align_done
    lsr R19
    ror R18
    dec r2
    rjmp shift_a_loop
    rjmp align_done

exp_a_ge_b:
    ; Expoente A >= B, deslocar B à direita
    mov R30, R26
    mov r2, R26
    sub r2, R27
shift_b_loop:
    tst r2
    breq align_done
    lsr R17
    ror R16
    dec r2
    rjmp shift_b_loop

align_done:

    ; Adicionar / Subtrair Mantissas
    ; Guarda Sinal em r21 (Callee-Saved)
    cp R28, R29
    breq same_signs

    ; --- Sinais Diferentes ---
    mov R20, R19        ; Guardar cópia de r19:r18 para comparação
    cp R19, R17
    brne mantissa_cmp_decided
    cp R18, R16
    brne mantissa_cmp_decided
    ; Mantissas iguais, resultado é zero
    clr R18
    clr R19
    clr R30
    clr r21
    rjmp pack_result

mantissa_cmp_decided:
    brlo b_greater_sub
a_ge_b_sub:
    ; A >= B, subtrair B de A
    sub R18, R16
    sbc R19, R17
    mov r21, R28        ; Sinal de A
    rjmp normalize_after_sub

b_greater_sub:
    ; B > A, subtrair A de B
    sub R16, R18
    sbc R17, R19
    mov r21, R29        ; Sinal de B
    mov R18, R16        ; Mover resultado para r19:r18
    mov R19, R17
    rjmp normalize_after_sub

same_signs:
    ; --- Sinais Iguais (Adição) ---
    add R18, R16
    adc R19, R17
    mov r21, R28
    rjmp normalize_check_add

normalize_check_add:
    ; Verificar se resultado é zero
    mov R16, R19
    or R16, R18
    brne check_add_overflow
    clr R30
    rjmp pack_result

check_add_overflow:
    sbrc R19, 3         ; Verificar overflow (bit 11)
    rjmp adjust_add_overflow
    rjmp add_normalize_loop_entry

normalize_after_sub:
    ; Verificar se resultado é zero
    mov R16, R19
    or R16, R18
    brne add_normalize_loop_entry
    clr R30
    clr r21             ; Zero positivo
    rjmp pack_result

add_normalize_loop_entry:
    ; Normalizar resultado (bit implícito em R19 bit 2)
add_normalize_loop:
    sbrc R19, 2         ; Verificar se bit implícito está em posição
    rjmp normalize_done
    ; Deslocar mantissa à esquerda até posicionar bit implícito
    lsl R18
    rol R19
    ; Ajustar expoente
    dec R30
    ; Verificar denormalização
    cpi R30, 1
    brsh add_normalize_loop  ; Se R30 >= 1, continuar normalizando
handle_denormalized:
    ; Resultado denormalizado, retornar zero
    clr R19
    clr R18
    clr R30
    rjmp pack_result

adjust_add_overflow:
    lsr R19
    ror R18
    inc R30
    cpi R30, 31
    brsh handle_overflow
normalize_done:
    rjmp pack_result

handle_overflow:
    ldi R19, 0x00
    ldi R18, 0x00
    ldi R30, 31
    rjmp pack_result

pack_result:
    mov R16, R30
    lsl R16
    lsl R16
    andi R16, 0x7C      ; Expoente em bits 6-2
    ; Montar byte alto do resultado
    mov R25, R16        ; Bits de expoente
    mov R16, R19
    andi R16, 0x03      ; Bits 1-0 da mantissa alta
    or R25, R16         ; Combinar expoente com mantissa alta
    or R25, r21         ; Aplicar bit de sinal
    mov R24, R18
add_cleanup:
    pop R30
    pop R27
    pop R26
    pop R21
    pop R20
    pop R19
    pop R18
    ret

;-----------------------------------------------------
; sub_f16: Subtração de dois half-precision IEEE-754
; Entradas: Operando A em R25:R24, Operando B em R23:R22
; Saída: Resultado em R25:R24
;-----------------------------------------------------

sub_f16:
    ldi R16, 0x80
    eor R23, R16
    
    rcall add_f16
    ret

;---------------------------------------------------------
; mul_f16: Multiplicação de dois half-precision IEEE-754
; Entradas: Operando A em R25:R24, Operando B em R23:R22
; Saída: Resultado em R25:R24
; Callee-Saved: r18-r21, r26-r27, r30-r31
; Volatile: r0, r1, r16, r17, r28, r29
;---------------------------------------------------------
mul_f16:
    push r18
    push r19
    push r20
    push r21
    push r26
    push r27
    push r30
    push r31

    ; --- Calcular Sinal do Resultado -> r20 (Callee-Saved) ---
    mov R20, R25
    andi R20, 0x80    ; Sinal A
    mov R21, R23
    andi R21, 0x80    ; Sinal B
    eor R20, R21      ; r20 = Sinal Resultado (0x00 ou 0x80)

    ; --- Extrair Expoentes -> r26, r27 (Callee-Saved) ---
    mov R26, R25
    andi R26, 0x7C    ; Isolar bits de expoente A
    lsr R26
    lsr R26           ; r26 = Expoente A (0-31, com bias)
    mov R27, R23
    andi R27, 0x7C    ; Isolar bits de expoente B
    lsr R27
    lsr R27           ; r27 = Expoente B (0-31, com bias)

    ; --- Checar Operandos Zero --- 
    tst R26
    brne check_b_zero
    mov R28, R24
    mov R29, R25
    andi R29, 0x03
    or R28, R29
    brne check_b_zero
    mov R25, R20
    ldi R24, 0x00
    rjmp mul_cleanup

check_b_zero:
    tst R27
    brne extract_mantissas
    mov R28, R22
    mov R29, R23
    andi R29, 0x03
    or R28, R29
    brne extract_mantissas
    mov R25, R20
    ldi R24, 0x00
    rjmp mul_cleanup

extract_mantissas:
    ; --- Extrair Mantissas (11 bits com implícito) ---
    mov R18, R24
    mov R19, R25
    andi R19, 0x03
    ldi R21, 1
    tst R26
    brne norm_a
    mov R26, R21
    rjmp extract_b
norm_a:
    ori R19, 0x04

extract_b:
    mov R16, R22
    mov R17, R23
    andi R17, 0x03
    ldi R21, 1
    tst R27
    brne norm_b
    mov R27, R21
    rjmp calc_exp
norm_b:
    ori R17, 0x04

calc_exp:
    add R26, R27
    subi R26, 15

    ; --- Multiplicar Mantissas (32 bits) ---
    clr R28
    clr R29
    clr R30
    clr R31
    mul R18, R16      ; AL * BL
    mov R28, r0
    mov R29, r1
    mul R18, R17      ; AL * BH
    add R29, r0
    adc R30, r1
    clr r0
    adc r31, r0
    mul R19, R16      ; AH * BL
    add R29, r0
    adc R30, r1
    clr r0
    adc r31, r0
    mul R19, R17      ; AH * BH
    add R30, r0
    adc R31, r1

    sbrc r30, 5
    rjmp mul_did_overflow
mul_no_overflow:
    rjmp mantissa_ready
mul_did_overflow:
    lsr r31
    ror r30
    ror r29
    ror r28
    inc r26
mantissa_ready:

mul_check_exponent:
    mov r19, r28
    or  r19, r29
    or  r19, r30
    or  r19, r31
    brne check_exp_non_zero
    mov R25, R20
    ldi R24, 0x00
    rjmp mul_cleanup
check_exp_non_zero:
    cpi R26, 31
    brsh exponent_overflow_final
    cpi R26, 1
    brsh pack_result_new
exponent_underflow_final:
    ldi R24, 0x00
    mov R25, R20
    rjmp mul_cleanup
exponent_overflow_final:
    ldi R24, 0x00
    ldi R25, 0x7C
    or R25, R20
    rjmp mul_cleanup

pack_result_new:
    mov R25, R20
    mov R21, R26
    lsl R21
    lsl R21
    andi R21, 0x7C
    or R25, R21
    mov R21, R30
    andi R21, 0x0C
    lsr R21
    lsr R21
    or R25, R21
    mov R21, R30
    andi R21, 0x03
    lsl R21
    lsl R21
    lsl R21
    lsl R21
    lsl R21
    lsl R21
    mov R24, R21
    mov R21, R29
    andi R21, 0xFC
    lsr R21
    lsr R21
    or R24, R21

mul_cleanup:
    pop r31
    pop r30
    pop r27
    pop r26
    pop r21
    pop r20
    pop r19
    pop r18
    ret

;-----------------------------------------------------
; div_f16: Divisão em ponto flutuante IEEE 754 de 16 bits
; Entrada:
;   r25:r24 = dividendo
;   r23:r22 = divisor
; Saída:
;   r25:r24 = resultado
; Registradores usados: r0, r1, r2, r3, r16-r21
;-----------------------------------------------------
div_f16:
    push r0
    push r1
    push r2
    push r3
    push r16
    push r17
    push r18
    push r19
    push r20
    push r21
    ; Verificar se divisor é zero
    mov  r16, r22
    or   r16, r23
    brne divisor_not_zero
    ; Divisão por zero - retornar infinito com sinal apropriado
    mov  r16, r25
    andi r16, 0x80
    ori  r16, 0x7C
    mov  r25, r16
    ldi  r24, 0x00
    rjmp div_f16_end
divisor_not_zero:
    ; Extrair sinais
    mov  r16, r25
    andi r16, 0x80
    mov  r17, r23
    andi r17, 0x80
    mov  r18, r16
    eor  r18, r17
    mov  r20, r18
    ; Extrair magnitude (remover bit de sinal)
    mov  r16, r25
    andi r16, 0x7F
    mov  r17, r23
    andi r17, 0x7F
    ; Verificar se dividendo é zero (após remover sinal)
    mov  r18, r16
    or   r18, r24
    brne dividend_not_zero
    ; Dividendo é zero - retornar zero com sinal apropriado
    mov  r25, r20
    ldi  r24, 0x00
    rjmp div_f16_end
dividend_not_zero:
    ; Extrair expoentes
    mov  r18, r16
    andi r18, 0x7C
    lsr  r18
    lsr  r18
    mov  r19, r17
    andi r19, 0x7C
    lsr  r19
    lsr  r19
    ; Calcular expoente do resultado: exp_res = exp_divd - exp_divs + 15
    sub  r18, r19
    subi r18, -15
    ; Extrair mantissas (11 bits com implícito)
    ; Dividendo -> r1:r0
    mov  r19, r16
    andi r19, 0x03
    ori  r19, 0x04
    mov  r1, r19
    mov  r0, r24
    ; Divisor -> r3:r2
    mov  r19, r17
    andi r19, 0x03
    ori  r19, 0x04
    mov  r3, r19
    mov  r2, r22
    clr  r16            ; Limpar registrador baixo do quociente
    clr  r17            ; Limpar registrador alto do quociente
    ldi  r19, 16        ; Contador de iterações (16 para quociente de 16 bits)
division_loop_new:
    ; Comparar Resto (r1:r0) com Divisor (r3:r2)
    cp   r0, r2
    cpc  r1, r3
    brcc sub_ok_new     ; Pular se R >= D (Carry Limpo)
    ; R < D : Bit do quociente é 0
    clc                 ; Garantir Carry = 0 para shift no quociente
    rjmp shift_q_new    ; Pular subtração
sub_ok_new:
    ; R >= D : Bit do quociente é 1
    sub  r0, r2         ; R = R - D
    sbc  r1, r3
    sec                 ; Setar Carry = 1 para shift no quociente
shift_q_new:
    ; Deslocar Quociente (r17:r16) à esquerda e inserir bit (Carry)
    rol  r16
    rol  r17
shift_r_new:
    lsl  r0
    rol  r1
    ; Decrementar contador e loop
    dec  r19
    brne division_loop_new
    ; Normalizar o quociente (r17:r16) e ajustar o expoente (r18)
    mov r19, r16
    or r19, r17
    breq result_is_zero
div_normalize_loop:
    tst r17
    brne msb_in_r17
    mov r17, r16
    clr r16
    subi r18, 8
    tst r17
    breq result_is_zero
    rjmp div_normalize_loop
msb_in_r17:
    mov r19, r17
check_msb_pos:
    sbrc r19, 7        ; Bit 7 de r17 é a posição do bit implícito (1.xxxx)
    rjmp msb_normalized
    tst r19
    breq msb_normalized ; Parar se ficou zero
    lsl  r16
    rol  r17
    lsl  r19
    dec  r18           ; Ajustar (diminuir) expoente para cada LSL
    rjmp check_msb_pos
msb_normalized:
    ; Verificar limites do expoente (Overflow / Underflow)
check_exponent:
    cpi  r18, 31
    brsh exponent_overflow
    cpi  r18, 1
    brlo exponent_underflow
    ; Expoente válido (1 a 30)
    rjmp construct_result
exponent_overflow:
    mov  r25, r20
    ori  r25, 0x7C
    ldi  r24, 0x00
    rjmp div_f16_end
result_is_zero:
exponent_underflow:
    mov  r25, r20
    ldi  r24, 0x00
    rjmp div_f16_end
construct_result:
    ; Construir número normalizado
    ; Expoente ajustado em r18, mantissa normalizada em r17:r16, sinal em r20.
    ; r17 = 1 M9 M8 M7 M6 M5 M4 M3
    ; r16 = M2 M1 M0 X X X X X
    ; Preparar byte alto (r25): Sinal | Expoente | M9 M8
    mov  r25, r18       ; Expoente (1-30)
    lsl  r25
    lsl  r25            ; Posicionar em bits 6-2
    andi r25, 0x7C      ; Isolar bits do expoente
    mov  r19, r17       ; r17 = 1 M9 M8 ...
    andi r19, 0x60      ; Isolar M9 M8 em posições 6,5
    lsr  r19
    lsr  r19
    lsr  r19
    lsr  r19
    lsr  r19            ; Mover M9 M8 para posições 1,0
    or   r25, r19       ; Combinar expoente e M9 M8
    or   r25, r20       ; Adicionar bit de sinal
    ; Preparar byte baixo (r24): M7 M6 M5 M4 M3 M2 M1 M0
    mov  r19, r17       ; r17 = 1 M9 M8 M7 M6 M5 M4 M3
    andi r19, 0x1F     ; Isolar M7..M3
    lsl  r19
    lsl  r19
    lsl  r19           ; r19 = M7M6M5M4M3000
    mov  r24, r19       ; Mover para resultado parcial
    mov  r19, r16       ; r16 = M2 M1 M0 X ...
    andi r19, 0xE0     ; Isolar M2 M1 M0
    lsr  r19
    lsr  r19
    lsr  r19           ; r19 = 000M2M1M000
    or   r24, r19       ; Combinar -> r24 = M7M6M5M4M3M2M1M0
div_f16_end:
    ; Restaurar registradores
    pop r21
    pop r20
    pop r19
    pop r18
    pop r17
    pop r16
    pop r3
    pop r2
    pop r1
    pop r0
    ret

;-----------------------------------------------------
; div_int_f16: Divisão inteira de float16
; Entrada A: r25:r24 (float16)
; Entrada B: r23:r22 (float16)
; Saida:     r25:r24 = float16(trunc(A/B))
; Callee-Saved: r18, r20, r21, r26, r27
; Volatile: r16, r17
; Chama: div_f16
;-----------------------------------------------------
div_int_f16:
    push r18
    push r20
    push r21
    push r26
    push r27

    ; 1. Chamar divisão float normal A/B
    call div_f16         ; Resultado R em r25:r24

    ; 2. Salvar sinal do resultado R -> r20
    mov r20, r25
    andi r20, 0x80       ; r20 = Sinal de R

    ; 3. Checar se resultado R é Zero
    mov r16, r24         ; Byte baixo de R
    mov r17, r25
    andi r17, 0x7F       ; Byte alto de R sem sinal
    or r16, r17          ; r16 == 0 se R for +/- 0.0
    brne check_inf_nan   ; Pular se R não for zero
    mov r25, r20         ; Restaurar sinal em r25
    ldi r24, 0x00        ; Garantir r24 zero
    rjmp div_int_cleanup ; Resultado é +/- 0.0, fim.

check_inf_nan:
    ; 4. Checar Inf/NaN (Expoente == 31) -> r26
    mov r26, r25         ; Pegar byte alto de R
    andi r26, 0x7C       ; Isolar bits do expoente (posições 6-2)
    cpi r26, 0x7C        ; Comparar com padrão Inf/NaN (11111 << 2)
    breq div_int_cleanup ; Se Inf/NaN, retornar R como está. Fim.

    ; 5. Extrair expoente com bias -> r26 (já isolado, só shiftar)
    lsr r26
    lsr r26              ; r26 = Expoente com bias (0-30)

    ; 6. Calcular E = Exp - Bias -> r18
    mov r18, r26
    subi r18, 15         ; r18 = E = Exp - 15. Flags N,Z,C,V,S atualizados.

    ; 7. Se E < 0 (Exp < 15), |R| < 1.0. Resultado truncado é +/- Zero
    brge E_ge_zero       ; Pular se E >= 0 (com sinal)
    ; E < 0 (|R| < 1.0)
    mov r25, r20         ; Resultado = +/- Zero
    ldi r24, 0x00
    rjmp div_int_cleanup ; Fim.

E_ge_zero:
    ; E é >= 0 aqui.
    ; 8. Se E >= 10 (Exp >= 25), R já é inteiro. Retornar R.
    cpi r18, 10
    brlt E_lt_10         ; Pular se E < 10 (com sinal)
    ; E >= 10. R já é inteiro.
    rjmp div_int_cleanup ; Fim.

E_lt_10:
    ; --- Truncar Parte Fracionária (E está entre 0 e 9) --- 
    ; 9. Calcular k = 10 - E -> r27
    ldi r27, 10
    sub r27, r18         ; r27 = k = 10 - E (k estará entre 1 e 10)

    ; 10. Mascarar r24 (M7-M0) - Zerar os k bits inferiores
    ldi r16, 0xFF        ; Máscara inicial
    mov r17, r27         ; Contador k
mask_r24_loop:
    tst r17              ; Contador k == 0?
    breq mask_r24_done   ; Sim, loop terminou.
    lsl r16              ; Deslocar máscara para esquerda
    dec r17              ; Decrementar contador k
    rjmp mask_r24_loop   ; Repetir o loop
mask_r24_done:
    and r24, r16         ; Aplicar máscara em r24

    ; 11. Mascarar r25 (M9, M8 - bits 1,0)
    cpi r27, 9           ; Comparar k com 9
    brlo mask_r25_done   ; Se k < 9 (E >= 2), M9 e M8 são mantidos.
    ldi r17, 0x02        ; Máscara para MANTER M9 (bit 1) - default para k=9
    cpi r27, 10          ; Comparar k com 10
    brne apply_mask_r25_final ; Pular se k=9
    ; k = 10 (E=0), zerar M9 e M8
    ldi r17, 0x00        ; Máscara para MANTER NADA (M9=0, M8=0)
apply_mask_r25_final:
    ; r17 contém a máscara de BITS A MANTER (02 ou 00) para M9, M8
    mov r16, r25         ; Copiar r25 original para r16 (Volatile)
    andi r16, 0x03       ; Isolar M9, M8 originais em r16
    and r16, r17         ; Aplicar máscara de MANTER -> r16 = M9', M8'
    andi r25, 0xFC       ; Zerar M9, M8 em r25 original (preserva S|Exp)
    or r25, r16          ; Combinar S|Exp com M9', M8' corretos
mask_r25_done:
    ; O resultado truncado está em r25:r24
div_int_cleanup:
    pop r27
    pop r26
    pop r21
    pop r20
    pop r18
    ret

;-----------------------------------------------------
; mod_f16: Resto da divisão de float16
; Entrada A: r25:r24
; Entrada B: r23:r22
; Saida:     r25:r24 = A % B = A - trunc(A/B) * B
; Usa:       Registradores conforme funções chamadas.
; Chama:     div_int_f16, mul_f16, sub_f16
;-----------------------------------------------------
mod_f16:
    ; --- Salvar Registradores ---
    push r0
    push r1
    push r2
    push r3
    push r16
    push r17
    push r18
    push r19
    push r20
    push r21
    push r26
    push r27
    push r28
    push r29
    push r30
    push r31
    ; --- Checar Divisor B Zero --- 
    mov r16, r22         ; B low
    mov r17, r23         ; B high
    andi r17, 0x7F       ; Limpar bit de sinal de B high
    or r16, r17          ; Checar se magnitude é zero
    brne mod_b_not_zero  ; Pular se B != 0
    ldi r25, 0x7E
    ldi r24, 0x00
    rjmp mod_cleanup     ; Pular para restauração e ret
mod_b_not_zero:

    ; Guardar A e B originais na pilha
    push r25 ; Salvar A high
    push r24 ; Salvar A low
    push r23 ; Salvar B high
    push r22 ; Salvar B low
    ; --- Passo 1: Calcular i = trunc(A / B) --- 
    call div_int_f16

    ; Guardar 'i' na pilha
    push r25
    push r24
    ; --- Passo 2: Calcular P = i * B --- 
    pop r22              ; Pop B low original para r22
    pop r23              ; Pop B high original para r23
    pop r24              ; Pop i low para r24
    pop r25              ; Pop i high para r25
    call mul_f16         ; P = i * B fica em r25:r24

    ; Guardar P na pilha
    push r25
    push r24
    ; --- Passo 3: Calcular R = A - P --- 
    pop r24              ; Pop A low original para r24
    pop r25              ; Pop A high original para r25
    pop r22              ; Pop P low para r22
    pop r23              ; Pop P high para r23
    call sub_f16

    ; Checar se R' (r25:r24) tem magnitude zero
    mov r16, r24         ; R' low
    mov r17, r25         ; R' high
    andi r17, 0x7F       ; Ignorar sinal de R'
    or r16, r17          ; r16 == 0 se magnitude zero
    brne mod_flip_sign   ; Pular para inverter se não for zero
    ; Magnitude é zero, não fazer nada
    rjmp mod_sign_done
mod_flip_sign:
    ; Resultado R' não é zero, inverter seu bit de sinal
    ldi r16, 0x80
    eor r25, r16         ; Inverte bit 7 de r25
mod_sign_done:
    ; O resultado final R (com sinal correto) está em r25:r24
mod_cleanup:
    pop r31
    pop r30
    pop r29
    pop r28
    pop r27
    pop r26
    pop r21
    pop r20
    pop r19
    pop r18
    pop r17
    pop r16
    pop r3
    pop r2
    pop r1
    pop r0
    ret

;-----------------------------------------------------
; pow_f16: Potência A^B, onde B é float16 representando int >= 1
; Entrada Base A: r25:r24
; Entrada Expo B: r23:r22
; Saida: r25:r24 = A^B
; Usa: r0,r1,r2,r16-r23,r26-r31
; Chama: f16_to_uint16, mul_f16
;-----------------------------------------------------
pow_f16:
    push r0
    push r1
    push r2
    push r3
    push r16
    push r17
    push r18
    push r19
    push r20
    push r21
    push r22
    push r23
    push r26
    push r27
    push r28
    push r29
    push r30
    push r31
    ; --- Converter Expoente B (float) para uint16 --- 
    call f16_to_uint16   ; Entrada r23:r22, Saída b_int em r27:r26

    ; --- Salvar Base A (CurrentPower) e Inicializar Result --- 
    mov r21, r24         ; Copiar A low para CurrentPower low (r21)
    mov r20, r25         ; Copiar A high para CurrentPower high (r20)
    ; --- Checar se b_int == 1 --- 
    ldi r16, 1           ; Comparar b_int com 1
    ldi r17, 0
    cp r26, r16          ; Compara low byte
    cpc r27, r17         ; Compara high byte
    breq pow_cleanup     ; Se b_int == 1, A (resultado) já está em r25:r24. Fim.

    ; --- Inicializar Resultado = 1.0 --- 
    ldi r25, 0x3C        ; Result high = 0x3C
    ldi r24, 0x00        ; Result low = 0x00

    ; --- Loop de Exponenciação por Quadrado --- 
pow_loop:
    ; Checar se b_int == 0
    mov r16, r26         ; Usar r16 como temp
    or r16, r27          ; Testar se r27:r26 é zero
    breq pow_loop_end    ; Se b_int == 0, fim do loop
    ; Checar se b_int é ímpar (testar bit 0 de r26)
    sbrc r26, 0          ; Pular instrução seguinte se bit 0 de r26 for 0 (par)
    rcall pow_mult_result; Se for ímpar, multiplicar Result por CurrentPower
pow_square_base_prep:
    ; --- Calcular CurrentPower = CurrentPower * CurrentPower --- 
    ; Guardar Result (r25:r24) na pilha antes de usar regs para CP*CP
    push r25
    push r24
    ; CP está em r20:r21
    mov r25, r20         ; Mover CP para entrada A (r25:r24)
    mov r24, r21
    mov r23, r20         ; Mover CP para entrada B (r23:r22)
    mov r22, r21
    call mul_f16         ; r25:r24 = CP * CP
    ; Guardar novo CurrentPower em r20:r21
    mov r20, r25
    mov r21, r24
    ; Restaurar Result da pilha para r25:r24
    pop r24
    pop r25
    ; --- Shift b_int >> 1 --- 
    lsr r26              ; Desloca low byte para direita
    ror r27              ; Rotaciona high byte com carry do lsr
    rjmp pow_loop        ; Volta ao início do loop

; --- Sub-rotina para Result = Result * CurrentPower --- 
; Chamada com rcall, precisa retornar com ret
; Entradas: Result(r25:r24), CP(r20:r21)
; Saida: Novo Result(r25:r24)
; Clobbers: r22, r23, e regs usados por mul_f16
pow_mult_result:
    mov r23, r20
    mov r22, r21
    ; Result (r25:r24) já está na entrada A
    call mul_f16         ; r25:r24 = Result * CP
    ret

pow_loop_end:
    ; O resultado final está em r25:r24
pow_cleanup:
    pop r31
    pop r30
    pop r29
    pop r28
    pop r27
    pop r26
    pop r23
    pop r22
    pop r21
    pop r20
    pop r19
    pop r18
    pop r17
    pop r16
    pop r3
    pop r2
    pop r1
    pop r0
    ret
;-----------------------------------------------------
; f16_to_uint16: Converte float16 para uint16_t
; Entrada: r23:r22 (float16)
; Saída:   r27:r26 (uint16_t)
; Modifica: r16, r17, r18, r19
;-----------------------------------------------------
f16_to_uint16:
    mov r18, r23        ; Byte alto de B
    andi r18, 0x7C      ; Isolar bits do expoente
    lsr r18
    lsr r18             ; r18 = Exp (15 a 30)

    ; Extrair mantissa M (10 bits)
    mov r16, r22        ; M7-M0
    mov r17, r23
    andi r17, 0x03      ; M9-M8

    ; Construir mantissa inteira de 11 bits: IntM = (1 << 10) | M
    ori r17, 0x04       ; Adicionar '1' implícito (bit 10 = bit 2 de r17)
    ; Mantissa IntM em r17:r16

    ; Calcular quantidade de shift: shift = E - 10 = (Exp - 15) - 10 = Exp - 25
    mov r19, r18        ; r19 = Exp
    subi r19, 25        ; r19 = shift = E - 10
    ; Se shift >= 0 (E >= 10), shift left. Se shift < 0 (E < 10), shift right.

    cpi r19, 0
    brge f16tu16_lsl    ; Se shift >= 0, pular para left shift

    ; --- Shift Direita (shift < 0, E < 10) ---
f16tu16_rsr:
    neg r19             ; shift_count = -shift = 10 - E
f16tu16_rsr_loop:
    tst r19
    breq f16tu16_done   ; Se contador == 0, terminou
    lsr r17             ; Shift IntM (r17:r16) >> 1
    ror r16
    dec r19
    rjmp f16tu16_rsr_loop

    ; --- Shift Esquerda (shift >= 0, E >= 10) ---
f16tu16_lsl:
f16tu16_lsl_loop:
    tst r19
    breq f16tu16_done   ; Se contador == 0, terminou
    lsl r16             ; Shift IntM (r17:r16) << 1
    rol r17
    dec r19
    rjmp f16tu16_lsl_loop

f16tu16_done:
    ; Resultado uint16 está em r17:r16
    mov r26, r16        ; Mover resultado low byte para r26
    mov r27, r17        ; Mover resultado high byte para r27
    ret                 ; Retorna uint16_t em r27:r26

;-----------------------------------------------------
; res_op: Recupera o resultado da linha N anterior.
; Entrada:
;   r24 = índice N (número de linhas anteriores)
; Saída:
;   r24:r25 = resultado armazenado em 'results' na posição (N*2)
;-----------------------------------------------------

res_op:
    lsl   r24         ; Multiplica N por 2 (offset em bytes)
    ldi   r30, lo8_results
    ldi   r31, hi8_results
    add   r30, r24    ; Adiciona o offset ao endereço base
    ld    r24, Z+     ; Carrega o byte baixo do resultado
    ld    r25, Z      ; Carrega o byte alto do resultado
    ret

;-----------------------------------------------------
; set_mem: Armazena o valor de r24:r25 em storeVal
;-----------------------------------------------------
set_mem:
    STS storeVal, r24
    STS storeVal+1, r25
    ret

;-----------------------------------------------------
; get_mem: Carrega o valor de storeVal em r24:r25
;-----------------------------------------------------
get_mem:
    LDS r24, storeVal
    LDS r25, storeVal+1
    ret

main:
; Operação de adição: 100 + 200
    LDI r24, 64
    LDI r25, 86
    LDI r22, 64
    LDI r23, 90
    RCALL add_f16
    STS T1_L, r24
    STS T1_H, r25
    LDS r24, T1_L
    LDS r25, T1_H
    STS results+0, r24
    STS results+1, r25
    RCALL print_f16
; Operação de adição: 123.5 + 456.25
    LDI r24, 184
    LDI r25, 87
    LDI r22, 33
    LDI r23, 95
    RCALL add_f16
    STS T2_L, r24
    STS T2_H, r25
    LDS r24, T2_L
    LDS r25, T2_H
    STS results+2, r24
    STS results+3, r25
    RCALL print_f16
; Operação de adição: -300 + 150
    LDI r24, 176
    LDI r25, 220
    LDI r22, 176
    LDI r23, 88
    RCALL add_f16
    STS T3_L, r24
    STS T3_H, r25
    LDS r24, T3_L
    LDS r25, T3_H
    STS results+4, r24
    STS results+5, r25
    RCALL print_f16
; Operação de adição: 10 + -9
    LDI r24, 0
    LDI r25, 73
    LDI r22, 128
    LDI r23, 200
    RCALL add_f16
    STS T4_L, r24
    STS T4_H, r25
    LDS r24, T4_L
    LDS r25, T4_H
    STS results+6, r24
    STS results+7, r25
    RCALL print_f16
; Operação de adição: -1000 + 250
    LDI r24, 208
    LDI r25, 227
    LDI r22, 208
    LDI r23, 91
    RCALL add_f16
    STS T5_L, r24
    STS T5_H, r25
    LDS r24, T5_L
    LDS r25, T5_H
    STS results+8, r24
    STS results+9, r25
    RCALL print_f16
; Operação de adição: 1024 + 512
    LDI r24, 0
    LDI r25, 100
    LDI r22, 0
    LDI r23, 96
    RCALL add_f16
    STS T6_L, r24
    STS T6_H, r25
    LDS r24, T6_L
    LDS r25, T6_H
    STS results+10, r24
    STS results+11, r25
    RCALL print_f16
; Operação de adição: -512.5 + 1023.5
    LDI r24, 1
    LDI r25, 224
    LDI r22, 255
    LDI r23, 99
    RCALL add_f16
    STS T7_L, r24
    STS T7_H, r25
    LDS r24, T7_L
    LDS r25, T7_H
    STS results+12, r24
    STS results+13, r25
    RCALL print_f16
; Operação de adição: 327.5 + -100.25
    LDI r24, 30
    LDI r25, 93
    LDI r22, 68
    LDI r23, 214
    RCALL add_f16
    STS T8_L, r24
    STS T8_H, r25
    LDS r24, T8_L
    LDS r25, T8_H
    STS results+14, r24
    STS results+15, r25
    RCALL print_f16
; Operação de adição: -250.75 + -100.125
    LDI r24, 214
    LDI r25, 219
    LDI r22, 66
    LDI r23, 214
    RCALL add_f16
    STS T9_L, r24
    STS T9_H, r25
    LDS r24, T9_L
    LDS r25, T9_H
    STS results+16, r24
    STS results+17, r25
    RCALL print_f16
; Operação de adição: 999.5 + -0.5
    LDI r24, 207
    LDI r25, 99
    LDI r22, 0
    LDI r23, 184
    RCALL add_f16
    STS T10_L, r24
    STS T10_H, r25
    LDS r24, T10_L
    LDS r25, T10_H
    STS results+18, r24
    STS results+19, r25
    RCALL print_f16
; Operação de adição: 12.5 + 0
    LDI r24, 64
    LDI r25, 74
    LDI r22, 0
    LDI r23, 0
    RCALL add_f16
    STS T11_L, r24
    STS T11_H, r25
    LDS r24, T11_L
    LDS r25, T11_H
    STS results+20, r24
    STS results+21, r25
    RCALL print_f16
; Operação de adição: -100 + 0
    LDI r24, 64
    LDI r25, 214
    LDI r22, 0
    LDI r23, 0
    RCALL add_f16
    STS T12_L, r24
    STS T12_H, r25
    LDS r24, T12_L
    LDS r25, T12_H
    STS results+22, r24
    STS results+23, r25
    RCALL print_f16
; Operação de adição: 0 + 0
    LDI r24, 0
    LDI r25, 0
    LDI r22, 0
    LDI r23, 0
    RCALL add_f16
    STS T13_L, r24
    STS T13_H, r25
    LDS r24, T13_L
    LDS r25, T13_H
    STS results+24, r24
    STS results+25, r25
    RCALL print_f16
; Operação de adição: 500 + -500
    LDI r24, 208
    LDI r25, 95
    LDI r22, 208
    LDI r23, 223
    RCALL add_f16
    STS T14_L, r24
    STS T14_H, r25
    LDS r24, T14_L
    LDS r25, T14_H
    STS results+26, r24
    STS results+27, r25
    RCALL print_f16
; Operação de adição: 1.25 + -1.25
    LDI r24, 0
    LDI r25, 61
    LDI r22, 0
    LDI r23, 189
    RCALL add_f16
    STS T15_L, r24
    STS T15_H, r25
    LDS r24, T15_L
    LDS r25, T15_H
    STS results+28, r24
    STS results+29, r25
    RCALL print_f16
; Operação de adição: 60000 + 5000
    LDI r24, 83
    LDI r25, 123
    LDI r22, 226
    LDI r23, 108
    RCALL add_f16
    STS T16_L, r24
    STS T16_H, r25
    LDS r24, T16_L
    LDS r25, T16_H
    STS results+30, r24
    STS results+31, r25
    RCALL print_f16
; Operação de adição: 65000 + 1000
    LDI r24, 239
    LDI r25, 123
    LDI r22, 208
    LDI r23, 99
    RCALL add_f16
    STS T17_L, r24
    STS T17_H, r25
    LDS r24, T17_L
    LDS r25, T17_H
    STS results+32, r24
    STS results+33, r25
    RCALL print_f16
; Operação de adição: -65504 + 1
    LDI r24, 255
    LDI r25, 251
    LDI r22, 0
    LDI r23, 60
    RCALL add_f16
    STS T18_L, r24
    STS T18_H, r25
    LDS r24, T18_L
    LDS r25, T18_H
    STS results+34, r24
    STS results+35, r25
    RCALL print_f16
; Operação de adição: -60000 + -6000
    LDI r24, 83
    LDI r25, 251
    LDI r22, 220
    LDI r23, 237
    RCALL add_f16
    STS T19_L, r24
    STS T19_H, r25
    LDS r24, T19_L
    LDS r25, T19_H
    STS results+36, r24
    STS results+37, r25
    RCALL print_f16
; Operação de adição: 0.00006103515625 + -0.000000059604644775390625
    LDI r24, 0
    LDI r25, 4
    LDI r22, 0
    LDI r23, 0
    RCALL add_f16
    STS T20_L, r24
    STS T20_H, r25
    LDS r24, T20_L
    LDS r25, T20_H
    STS results+38, r24
    STS results+39, r25
    RCALL print_f16
; Operação de adição: 0.00000011920928955078125 + -0.000000059604644775390625
    LDI r24, 0
    LDI r25, 0
    LDI r22, 0
    LDI r23, 0
    RCALL add_f16
    STS T21_L, r24
    STS T21_H, r25
    LDS r24, T21_L
    LDS r25, T21_H
    STS results+40, r24
    STS results+41, r25
    RCALL print_f16
end:
	rjmp end

.dseg
T1_L: .byte 1
T1_H: .byte 1
T2_L: .byte 1
T2_H: .byte 1
T3_L: .byte 1
T3_H: .byte 1
T4_L: .byte 1
T4_H: .byte 1
T5_L: .byte 1
T5_H: .byte 1
T6_L: .byte 1
T6_H: .byte 1
T7_L: .byte 1
T7_H: .byte 1
T8_L: .byte 1
T8_H: .byte 1
T9_L: .byte 1
T9_H: .byte 1
T10_L: .byte 1
T10_H: .byte 1
T11_L: .byte 1
T11_H: .byte 1
T12_L: .byte 1
T12_H: .byte 1
T13_L: .byte 1
T13_H: .byte 1
T14_L: .byte 1
T14_H: .byte 1
T15_L: .byte 1
T15_H: .byte 1
T16_L: .byte 1
T16_H: .byte 1
T17_L: .byte 1
T17_H: .byte 1
T18_L: .byte 1
T18_H: .byte 1
T19_L: .byte 1
T19_H: .byte 1
T20_L: .byte 1
T20_H: .byte 1
T21_L: .byte 1
T21_H: .byte 1

results: .byte 42
    .equ lo8_results = ((results) & 0xFF)
    .equ hi8_results = (((results) >> 8) & 0xFF)
storeVal: .byte 2
    .equ BUFFER_ADDR = 0x100
    .equ BUFFER_SIZE = 11