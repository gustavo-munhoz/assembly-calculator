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

	rjmp comandos
; Rotina para imprimir um número de 16 bits em decimal.
print16:
    push    R16
    push    R17
    push    R18
    push    R19
    push    R20
    push    R21
    push    R22         ; Usado para somar o offset ASCII
    clr     R17         ; Flag: nenhum dígito impresso ainda

    ; Processa divisor 10000 (0x2710)
    ldi     R21, 0x27   ; Parte alta de 10000
    ldi     R20, 0x10   ; Parte baixa de 10000
    rcall   print16_divisor

    ; Processa divisor 1000 (0x03E8)
    ldi     R21, 0x03
    ldi     R20, 0xE8
    rcall   print16_divisor

    ; Processa divisor 100 (0x0064)
    ldi     R21, 0x00
    ldi     R20, 0x64
    rcall   print16_divisor

    ; Processa divisor 10 (0x000A)
    ldi     R21, 0x00
    ldi     R20, 0x0A
    rcall   print16_divisor

    ; Processa divisor 1 (0x0001)
    ldi     R21, 0x00
    ldi     R20, 0x01
    rcall   print16_divisor

    pop     R22
    pop     R21
    pop     R20
    pop     R19
    pop     R18
    pop     R17
    pop     R16
    ldi     R16, 0x0D
    call    tx_R16
    ldi     R16, 0x0A
    call    tx_R16
    ret

; Sub-rotina auxiliar para processar cada divisor.
print16_divisor:
    clr     R16           ; Zera o contador do dígito
print16_divisor_loop:
    cp      R25, R21
    brlo    print16_divisor_done  ; Se R25 < R21, o divisor não cabe
    breq    check_low             ; Se R25 == R21, verifica o byte inferior
    rjmp    print16_subtract      ; Se R25 > R21, o divisor cabe
check_low:
    cp      R24, R20
    brlo    print16_divisor_done
print16_subtract:
    sub     R24, R20
    sbc     R25, R21
    inc     R16           ; Incrementa o contador do dígito
    rjmp    print16_divisor_loop

print16_divisor_done:
    tst     R16
    breq    print16_maybe_skip
    rjmp    print16_send_digit

print16_maybe_skip:
    tst     R17         ; Se já houve impressão, envia o dígito (mesmo que seja 0)
    brne    print16_send_digit
    ; Se nenhum dígito foi impresso, envia '0' somente se estivermos na última casa (divisor 1)
    cpi     R21, 0
    brne    print16_skip
    cpi     R20, 1
    brne    print16_skip

print16_send_digit:
    ldi     R22, 0x30   ; Offset ASCII
    add     R16, R22
    call    tx_R16      ; Envia o dígito via Serial
    ldi     R17, 1      ; Marca que já houve impressão
print16_skip:
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

; int_add: Soma inteira simples de 16 bits.
; Entrada:
;   r25:r24 = operando esquerdo (16 bits)
;   r23:r22 = operando direito (16 bits)
; Saída:
;   r25:r24 = resultado da soma
int_add:
    add   r24, r22    ; soma o byte inferior
    adc   r25, r23    ; soma o byte superior com carry
    ret

; int_sub: Subtrai dois números inteiros de 16 bits.
int_sub:
    sub r24, r22
    sub r25, r23
    ret

; int_mul: Multiplicação por adição repetida (unsigned, 16-bit)
; Entrada:
;   r25:r24 = multiplicando (16 bits)
;   r23:r22 = multiplicador (16 bits, assume que o byte alto (r23) é 0)
; Saída:
;   r25:r24 = produto (16 bits)
; A rotina calcula o produto somando repetidamente o multiplicando, o número de vezes dado pelo multiplicador.
int_mul:
    push    r30            ; Salva r30
    push    r31            ; Salva r31
    mov     r30, r24       ; r30 <- multiplicando (low)
    mov     r31, r25       ; r31 <- multiplicando (high)
    clr     r24            ; Zera produto (low) em r24
    clr     r25            ; Zera produto (high) em r25
    mov     r16, r22       ; Usa o multiplicador (byte baixo) como contador
mul_loop:
    tst     r16            ; Verifica se o contador chegou a 0
    breq    end_mul_loop   ; Se sim, sai do loop
    mov     r22, r30       ; Carrega multiplicando em r22 (para int_add)
    mov     r23, r31       ; Carrega multiplicando em r23 (para int_add)
    RCALL   int_add        ; Produto = produto (em r25:r24) + multiplicando (em r23:r22)
    dec     r16            ; Decrementa o contador
    rjmp    mul_loop       ; Repete o loop
end_mul_loop:
    pop     r31            ; Restaura r31
    pop     r30            ; Restaura r30
    ret

; int_div: Divisão inteira por restauração (unsigned, 16-bit)
; Entrada:
;   r25:r24 = dividendo (16 bits)
;   r23:r22 = divisor   (16 bits)
; Saída:
;   r25:r24 = quociente  (16 bits)
;   r21:r20 = resto      (16 bits)

int_div:
    clr   r21         ; remainder high = 0
    clr   r20         ; remainder low  = 0
    ldi   r16, 16     ; 16 iteracoes
div_loop:
    rol   r21
    rol   r20
    rol   r25
    rol   r24
    cp    r21, r23
    cpc   r20, r22
    brlo  no_sub
    sub   r20, r22
    sbc   r21, r23
    ori   r24, 0x01
no_sub:
    dec   r16
    brne  div_loop
    ret

;---------------------------------------------------
; int_pow: Exponenciação rápida (unsigned 16 bits)
;   Entrada:
;     r25:r24 = base  (r25 alto, r24 baixo)
;     r23:r22 = exponent (16 bits)
;   Saída:
;     r25:r24 = base^exponent (16 bits)
;   Utiliza:
;     int_mul  (rotina de multiplicacao 16 bits)
;     r19:r18  -> copia de base
;     r21:r20  -> acumulador de result
;   Destrói:
;     r19:r18, r21:r20, r24, r25
;---------------------------------------------------
int_pow:
    ; Copia base (r25:r24) para r19:r18
    mov   r19, r25
    mov   r18, r24

    ; result = 1 (r21:r20)
    ldi   r21, 0
    ldi   r20, 1

pow_loop:
    ; Se exponent == 0 => sai
    tst   r22         ; testa parte baixa
    brne  exponent_not_zero
    tst   r23         ; parte alta
    breq  pow_done
exponent_not_zero:

    ; Se (exponent & 1) != 0, ou seja, se for ímpar => result *= base
    sbrs  r22, 0      ; se bit 0 de r22 = 0, pula a multiplicacao
    rjmp  do_mult

skip_mult:
    ; exponent >>= 1
    lsr   r23
    ror   r22

    ; base = base * base
    mov   r24, r18   ; carrega base em r24:r25
    mov   r25, r19
    mov   r22, r18   ; multiplicador (mesmo valor)
    mov   r23, r19
    rcall int_mul    ; produto sai em r25:r24
    ; atualiza base (r19:r18)
    mov   r18, r24
    mov   r19, r25
    rjmp  pow_loop

do_mult:
    ; result = result * base
    mov   r24, r20   ; r21:r20 => result
    mov   r25, r21
    mov   r22, r18   ; r19:r18 => base
    mov   r23, r19
    rcall int_mul    ; produto sai em r25:r24
    ; salva em result
    mov   r20, r24
    mov   r21, r25
    rjmp  skip_mult

pow_done:
    ; resultado final esta em r21:r20
    mov   r24, r20
    mov   r25, r21
    ret

;-----------------------------------------------------
; res_op: Recupera o resultado da linha N anterior.
; Entrada:
;   r24 = índice N (número de linhas anteriores)
; Saída:
;   r24:r25 = resultado (16 bits) armazenado em 'results' na posição (N*2)
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

comandos:
; Operação de adição: 1 + 1
    LDI r24, 1
    LDI r25, 0
    LDI r22, 1
    LDI r23, 0
    RCALL int_add
    STS T1_L, r24
    STS T1_H, r25
    LDS r24, T1_L
    LDS r25, T1_H
    STS results+0, r24
    STS results+1, r25
    RCALL print16
; Operação de adição: 2 + 3
    LDI r24, 2
    LDI r25, 0
    LDI r22, 3
    LDI r23, 0
    RCALL int_add
    STS T2_L, r24
    STS T2_H, r25
    LDS r24, T2_L
    LDS r25, T2_H
    STS results+2, r24
    STS results+3, r25
    RCALL print16
; Operação de resultado N linhas atrás: 1 RES
    LDI r24, 1
    RCALL res_op
    STS T3_L, r24
    STS T3_H, r25
; Operação de escrever valor na memória: T3 MEM
    LDS r24, T3_L
    LDS r25, T3_H
    RCALL set_mem
    STS T4_L, r24
    STS T4_H, r25
    LDS r24, T4_L
    LDS r25, T4_H
    STS results+4, r24
    STS results+5, r25
    RCALL print16
; Operação de potenciação: 2 ^ 2
    LDI r24, 2
    LDI r25, 0
    LDI r22, 2
    LDI r23, 0
    RCALL int_pow
    STS T5_L, r24
    STS T5_H, r25
    LDS r24, T5_L
    LDS r25, T5_H
    STS results+6, r24
    STS results+7, r25
    RCALL print16
; Operação de ler valor da memória
    RCALL get_mem
    STS T6_L, r24
    STS T6_H, r25
    LDS r24, T6_L
    LDS r25, T6_H
    STS results+8, r24
    STS results+9, r25
    RCALL print16
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

results: .byte 10
.equ lo8_results = ((results) & 0xFF)
.equ hi8_results = (((results) >> 8) & 0xFF)
storeVal: .byte 2
