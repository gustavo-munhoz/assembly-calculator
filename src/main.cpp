#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "../include/assembly_generator.hpp"
#include "../include/dsegManager.hpp"

using namespace std;

int main(int argc, char** argv) {
	if (argc != 2) {
		cerr << "[ERROR] Invalid call. Usage: ./asmGenerator <EXPRESSIONS_FILE>" << endl;
        return 1;
	}

    vector<string> rawCommands = extractRawCommandsFromFile(argv[1]);

    string completeCode;
    string header = generateFullHeaderAssembly();
    completeCode += header;

    int lineIndex = 0;
    for (const auto& cmd: rawCommands) {
        int pos = 0;
        string result;

        try {
            string assembly = parseExpression(cmd, pos, result, lineIndex);
            int offset = lineIndex * 2;

            completeCode += assembly;
            completeCode += "    LDS r24, " + result + "_L\n";
            completeCode += "    LDS r25, " + result + "_H\n";

            completeCode += "    STS results+" + to_string(offset) + ", r24\n";
            completeCode += "    STS results+" + to_string(offset + 1) + ", r25\n";

            completeCode += "    RCALL print16\n";

        } catch (const exception &ex) {
            cerr << "Error: " << ex.what() << endl;
            return 1;
        }
        lineIndex++;
    }

    completeCode += "end:\n\trjmp end\n";
    completeCode += "\n.dseg\n" + dsegDeclarations.str() + "\n";

    auto byteCount = rawCommands.size() * 2;
    completeCode += "results: .byte " + to_string(byteCount) + "\n";

    completeCode += ".equ lo8_results = ((results) & 0xFF)\n";
    completeCode += ".equ hi8_results = (((results) >> 8) & 0xFF)\n";
    completeCode += "storeVal: .byte 2\n";

    string inputFile = argv[1];
    string outputFile;
    size_t dotPos = inputFile.find_last_of('.');
    if (dotPos != string::npos)
        outputFile = inputFile.substr(0, dotPos) + ".asm";
    else
        outputFile = inputFile + ".asm";

    ofstream outFile(outputFile);
    if (!outFile) {
        cerr << "[ERROR] Could not open output file: " << outputFile << endl;
        return 1;
    }

    outFile << completeCode;
    outFile.close();

    cout << "Assembly successfully generated in " << outputFile << endl;
    return 0;
}