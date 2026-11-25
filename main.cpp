#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "token.h"
#include "ast.h"
#include "semantic.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage:\n"
             << "  " << argv[0] << " <source.txt>            (Run Lexer + Parser + Semantic Analyzer)\n"
             << "  " << argv[0] << " --lex <source.txt>      (Phase 1: Lexical Analysis -> output.tokens)\n"
             << "  " << argv[0] << " --parse <output.tokens> (Phase 2: Syntax Analysis -> output.ast.json)\n"
             << "  " << argv[0] << " --sem <source.txt>      (Phase 3: Semantic Analysis -> output.symtab)\n";
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
    } else if (string(argv[1]) == "--sem" && argc >= 3) {
        mode = "sem";
        inputFile = argv[2];
    } else {
        inputFile = argv[1];
    }

    try {
        if (mode == "parse") {
            vector<Token> tokens = readTokensFromFile(inputFile);
            auto ast = runParser(tokens);
            writeASTToFile(ast, "output.ast.json");
            cout << "[PARSER OK] Parsed '" << inputFile << "' -> Generated 'output.ast.json'\n";
            return 0;
        }

        ifstream inFile(inputFile);
        if (!inFile) throw runtime_error("Cannot open source file: " + inputFile);
        stringstream buffer;
        buffer << inFile.rdbuf();

        // Phase 1: Lexer
        vector<Token> tokens = runLexer(buffer.str());
        if (mode == "lex" || mode == "all") {
            writeTokensToFile(tokens, "output.tokens");
            cout << "[LEXER OK] Generated 'output.tokens' (" << tokens.size() << " tokens)\n";
        }
        if (mode == "lex") return 0;

        // Phase 2: Parser
        auto ast = runParser(tokens);
        if (mode == "all") {
            writeASTToFile(ast, "output.ast.json");
            cout << "[PARSER OK] Generated 'output.ast.json'\n";
        }

        // Phase 3: Semantic Analysis
        if (mode == "sem" || mode == "all") {
            vector<SymbolEntry> symTable = runSemanticAnalysis(ast);
            writeSymbolTableToFile(symTable, "output.symtab");
            cout << "[SEMANTIC OK] Type & scope checks passed -> Generated 'output.symtab' ("
                 << symTable.size() << " symbols)\n";
        }

    } catch (const exception& ex) {
        cerr << "[ERROR] " << ex.what() << endl;
        return 1;
    }

    return 0;
}