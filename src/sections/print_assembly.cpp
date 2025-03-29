//
// Created by Gustavo Munhoz Correa on 28/03/25.
//

#include <sstream>
#include <string>

std::string generatePrint16Assembly() {
    std::stringstream ss;
    ss << "; Rotina para imprimir um número de 16 bits em decimal.\n";
    ss << "print16:\n";
    ss << "    push    R16\n";
    ss << "    push    R17\n";
    ss << "    push    R18\n";
    ss << "    push    R19\n";
    ss << "    push    R20\n";
    ss << "    push    R21\n";
    ss << "    push    R22         ; Usado para somar o offset ASCII\n";
    ss << "    clr     R17         ; Flag: nenhum dígito impresso ainda\n";
    ss << "\n    ; Processa divisor 10000 (0x2710)\n";
    ss << "    ldi     R21, 0x27   ; Parte alta de 10000\n";
    ss << "    ldi     R20, 0x10   ; Parte baixa de 10000\n";
    ss << "    rcall   print16_divisor\n";
    ss << "\n    ; Processa divisor 1000 (0x03E8)\n";
    ss << "    ldi     R21, 0x03\n";
    ss << "    ldi     R20, 0xE8\n";
    ss << "    rcall   print16_divisor\n";
    ss << "\n    ; Processa divisor 100 (0x0064)\n";
    ss << "    ldi     R21, 0x00\n";
    ss << "    ldi     R20, 0x64\n";
    ss << "    rcall   print16_divisor\n";
    ss << "\n    ; Processa divisor 10 (0x000A)\n";
    ss << "    ldi     R21, 0x00\n";
    ss << "    ldi     R20, 0x0A\n";
    ss << "    rcall   print16_divisor\n";
    ss << "\n    ; Processa divisor 1 (0x0001)\n";
    ss << "    ldi     R21, 0x00\n";
    ss << "    ldi     R20, 0x01\n";
    ss << "    rcall   print16_divisor\n";
    ss << "\n    pop     R22\n";
    ss << "    pop     R21\n";
    ss << "    pop     R20\n";
    ss << "    pop     R19\n";
    ss << "    pop     R18\n";
    ss << "    pop     R17\n";
    ss << "    pop     R16\n";
    ss << "    ldi     R16, 0x0D\n";
    ss << "    call    tx_R16\n";
    ss << "    ldi     R16, 0x0A\n";
    ss << "    call    tx_R16\n";
    ss << "    ret\n";
    return ss.str();
}

std::string generatePrint16DivisorAssembly() {
    std::stringstream ss;
    ss << "; Sub-rotina auxiliar para processar cada divisor.\n";
    ss << "print16_divisor:\n";
    ss << "    clr     R16           ; Zera o contador do dígito\n";
    ss << "print16_divisor_loop:\n";
    ss << "    cp      R25, R21\n";
    ss << "    brlo    print16_divisor_done  ; Se R25 < R21, o divisor não cabe\n";
    ss << "    breq    check_low             ; Se R25 == R21, verifica o byte inferior\n";
    ss << "    rjmp    print16_subtract      ; Se R25 > R21, o divisor cabe\n";
    ss << "check_low:\n";
    ss << "    cp      R24, R20\n";
    ss << "    brlo    print16_divisor_done\n";
    ss << "print16_subtract:\n";
    ss << "    sub     R24, R20\n";
    ss << "    sbc     R25, R21\n";
    ss << "    inc     R16           ; Incrementa o contador do dígito\n";
    ss << "    rjmp    print16_divisor_loop\n";
    ss << "\nprint16_divisor_done:\n";
    ss << "    tst     R16\n";
    ss << "    breq    print16_maybe_skip\n";
    ss << "    rjmp    print16_send_digit\n";
    ss << "\nprint16_maybe_skip:\n";
    ss << "    tst     R17         ; Se já houve impressão, envia o dígito (mesmo que seja 0)\n";
    ss << "    brne    print16_send_digit\n";
    ss << "    ; Se nenhum dígito foi impresso, envia '0' somente se estivermos na última casa (divisor 1)\n";
    ss << "    cpi     R21, 0\n";
    ss << "    brne    print16_skip\n";
    ss << "    cpi     R20, 1\n";
    ss << "    brne    print16_skip\n";
    ss << "\nprint16_send_digit:\n";
    ss << "    ldi     R22, 0x30   ; Offset ASCII\n";
    ss << "    add     R16, R22\n";
    ss << "    call    tx_R16      ; Envia o dígito via Serial\n";
    ss << "    ldi     R17, 1      ; Marca que já houve impressão\n";
    ss << "print16_skip:\n";
    ss << "    ret\n";
    return ss.str();
}