#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "token.h"
#include "ast.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage:\n"
             << "  " << argv[0] << " <source.txt>          (Run both Lexer and Parser)\n"
             << "  " << argv[0] << " --lex <source.txt>    (Run Lexical Analysis only -> output.tokens)\n"
             << "  " << argv[0] << " --parse <output.tokens> (Run Syntax Analysis only -> output.ast.json)\n";
        return 1;
    }

    string mode = "all";
    string inputFile;

    if (string(argv[1]) == "--lex" && argc >= 3) {
        mode = "lex";
        inputFile = argv[2];
    } else if (string(argv[1]) == "--parse" && argc >= 3) {
        mode = "parse";
        inputFile = argv[2];
    } else {
        inputFile = argv[1];
    }

    try {
        if (mode == "lex" || mode == "all") {
            ifstream inFile(inputFile);
            if (!inFile) throw runtime_error("Cannot open source file: " + inputFile);
            stringstream buffer;
            buffer << inFile.rdbuf();

            vector<Token> tokens = runLexer(buffer.str());
            writeTokensToFile(tokens, "output.tokens");
            cout << "[LEXER OK] Generated 'output.tokens' (" << tokens.size() << " tokens)\n";

            if (mode == "all") {
                auto ast = runParser(tokens);
                writeASTToFile(ast, "output.ast.json");
                cout << "[PARSER OK] Generated 'output.ast.json'\n";
            }
        } else if (mode == "parse") {
            vector<Token> tokens = readTokensFromFile(inputFile);
            auto ast = runParser(tokens);
            writeASTToFile(ast, "output.ast.json");
            cout << "[PARSER OK] Parsed '" << inputFile << "' -> Generated 'output.ast.json'\n";
        }
    } catch (const exception& ex) {
        cerr << "[ERROR] " << ex.what() << endl;
        return 1;
    }

    return 0;
}