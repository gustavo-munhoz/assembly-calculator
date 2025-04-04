cmake_minimum_required(VERSION 3.27)
project(proj1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    proj1
    src/main.cpp
        src/generator/assembly_generator.cpp
        src/sections/config_assembly.cpp
        src/sections/serial_comm_assembly.cpp
        src/sections/print_assembly.cpp
        src/operations/add_assembly.cpp
        include/operations/add_assembly.hpp
        include/sections/config_assembly.hpp
        include/sections/print_assembly.hpp
        include/sections/serial_comm_assembly.hpp
        include/operations/subtract_assembly.hpp
        src/operations/subtract_assembly.cpp
        include/operations/mult_assembly.hpp
        src/operations/mult_assembly.cpp
        include/operations/int_division_assembly.hpp
        src/operations/int_division_assembly.cpp
        include/operations/pow_assembly.hpp
        src/operations/pow_assembly.cpp
        include/operations/res_assembly.hpp
        src/operations/res_assembly.cpp
        include/operations/mem_assembly.hpp
        src/operations/mem_assembly.cpp
        include/operations/real_division_assembly.hpp
        src/operations/real_division_assembly.cpp
        include/operations/mod_assembly.hpp
        src/operations/mod_assembly.cpp
        src/generator/parse_helpers.cpp
        src/generator/temp_var_manager.cpp
        include/generator/parse_helpers.hpp
        include/generator/expression_parser.hpp
        src/generator/expression_parser.cpp
        include/generator/temp_var_manager.hpp
)
