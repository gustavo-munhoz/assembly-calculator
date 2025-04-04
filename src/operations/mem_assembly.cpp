//
// Created by Gustavo Munhoz Correa on 29/03/25.
//

#include <string>
#include <sstream>

/// Cria o código em assembly para fazer a operação de salvar um valor na memória.
std::string generateValueMemAssembly() {
    std::stringstream ss;
    ss << ";-----------------------------------------------------\n";
    ss << "; set_mem: Armazena o valor de r24:r25 em storeVal\n";
    ss << ";-----------------------------------------------------\n";
    ss << "set_mem:\n";
    ss << "    STS storeVal, r24\n";
    ss << "    STS storeVal+1, r25\n";
    ss << "    ret\n";
    return ss.str();
}

/// Cria o código em assembly para fazer a operação de retornar um valor salvo na memória.
std::string generateMemAssembly() {
    std::stringstream ss;
    ss << ";-----------------------------------------------------\n";
    ss << "; get_mem: Carrega o valor de storeVal em r24:r25\n";
    ss << ";-----------------------------------------------------\n";
    ss << "get_mem:\n";
    ss << "    LDS r24, storeVal\n";
    ss << "    LDS r25, storeVal+1\n";
    ss << "    ret\n";
    return ss.str();
}
