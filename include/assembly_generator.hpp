//
// Created by Gustavo Munhoz Correa on 20/03/25.
//

#ifndef ASSEMBLY_GENERATOR_HPP
#define ASSEMBLY_GENERATOR_HPP

#include <string>

std::string parseExpression(const std::string &s, int &pos, std::string &result, int lineIndex);
std::vector<std::string> extractRawCommandsFromFile(const std::string &filename);
std::string generateFullHeaderAssembly();

#endif // ASSEMBLY_GENERATOR_HPP
