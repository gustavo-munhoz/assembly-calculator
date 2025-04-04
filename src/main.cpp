#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "../include/generator/assembly_generator.hpp"
#include "../include/generator/temp_var_manager.hpp"
#include "../include/generator/expression_parser.hpp"

using namespace std;

string generateDataSegAssembly(size_t results_size);
void writeCodeToFile(string code, string inputFile);

int main(int argc, char** argv) {
	if (argc != 2) {
		cerr << "[ERROR] Invalid call. Usage: ./proj1 <EXPRESSIONS_FILE>" << endl;
        return 1;
	}

    vector<string> rawCommands = extractRawCommandsFromFile(argv[1]);
    string completeCode;

    completeCode += generateFullHeaderAssembly();

    int lineIndex = 0;
    for (const auto& cmd: rawCommands) {
        int pos = 0;
        string result;

        try {
            string assembly = ExpressionParser::parseExpression(cmd, pos, result, lineIndex);
            int offset = lineIndex * 2;

            completeCode += assembly;
            completeCode += "    LDS r24, " + result + "_L\n";
            completeCode += "    LDS r25, " + result + "_H\n";

            completeCode += "    STS results+" + to_string(offset) + ", r24\n";
            completeCode += "    STS results+" + to_string(offset + 1) + ", r25\n";

            completeCode += "    RCALL print_f16\n";

        } catch (const exception &ex) {
            cerr << "Error: " << ex.what() << endl;
            return 1;
        }
        lineIndex++;
    }

    completeCode += "end:\n\trjmp end\n";

    auto byteCount = rawCommands.size() * 2;
    completeCode += generateDataSegAssembly(byteCount);

    writeCodeToFile(completeCode, argv[1]);

    return 0;
}

string generateDataSegAssembly(size_t results_size) {
    stringstream ss;

    ss << "\n.dseg\n" << TempVarManager::getTempVariables() << "\n";
    ss << "results: .byte " + to_string(results_size) << "\n";

    ss << "    .equ lo8_results = ((results) & 0xFF)\n";
    ss << "    .equ hi8_results = (((results) >> 8) & 0xFF)\n";
    ss << "storeVal: .byte 2\n";
    ss << "    .equ BUFFER_ADDR = 0x100\n";
    ss << "    .equ BUFFER_SIZE = 11";

    return ss.str();
}

void writeCodeToFile(string code, string inputFile) {
    string outputFile;
    size_t dotPos = inputFile.find_last_of('.');

    if (dotPos != string::npos) {
        outputFile = inputFile.substr(0, dotPos) + ".asm";
    } else {
        outputFile = inputFile + ".asm";
    }

    ofstream outFile(outputFile);
    if (!outFile) {
        cerr << "[ERROR] Could not open output file: " << outputFile << endl;
        abort();
    }

    outFile << code;
    outFile.close();

    cout << "Assembly successfully written in " << outputFile << endl;
}