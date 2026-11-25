#include "token.h"
#include <cctype>

using namespace std;

class Lexer {
private:
    string src;
    size_t pos = 0;
    int line = 1;
    int col = 1;

    char peek(int offset = 0) const {
        if (pos + offset >= src.size()) return '\0';
        return src[pos + offset];
    }

    char advance() {
        char c = src[pos++];
        if (c == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        return c;
    }

    TokenType checkKeyword(const string& word) {
        if (word == "int16") return TokenType::KW_INT16;
        if (word == "int32") return TokenType::KW_INT32;
        if (word == "uint16") return TokenType::KW_UINT16;
        if (word == "uint32") return TokenType::KW_UINT32;
        if (word == "float") return TokenType::KW_FLOAT;
        if (word == "char") return TokenType::KW_CHAR;
        if (word == "shro") return TokenType::KW_SHRO;
        if (word == "khatam") return TokenType::KW_KHATAM;
        if (word == "jabtak") return TokenType::KW_JABTAK;
        if (word == "agar") return TokenType::KW_AGAR;
        if (word == "warna") return TokenType::KW_WARNA;
        if (word == "nikal") return TokenType::KW_NIKAL;
        if (word == "print") return TokenType::KW_PRINT;
        if (word == "medalo") return TokenType::OP_ASSIGN;
        return TokenType::ID;
    }

public:
    Lexer(const string& source) : src(source) {}

    vector<Token> tokenize() {
        vector<Token> tokens;

        while (pos < src.size()) {
            char c = peek();

            if (isspace(c)) {
                advance();
                continue;
            }

            if (c == '/' && peek(1) == '/') {
                while (peek() != '\n' && peek() != '\0') advance();
                continue;
            }

            int startLine = line;
            int startCol = col;

            // Keywords & Identifiers
            if (isalpha(c) || c == '_') {
                string lex;
                while (isalnum(peek()) || peek() == '_') {
                    lex += advance();
                }
                tokens.push_back({checkKeyword(lex), lex, startLine, startCol});
                continue;
            }

            // Integer & Float Literals
            if (isdigit(c)) {
                string num;
                bool isFloat = false;
                while (isdigit(peek())) {
                    num += advance();
                }
                if (peek() == '.' && isdigit(peek(1))) {
                    isFloat = true;
                    num += advance();
                    while (isdigit(peek())) {
                        num += advance();
                    }
                }
                tokens.push_back({isFloat ? TokenType::FLOAT_LIT : TokenType::INT_LIT, num, startLine, startCol});
                continue;
            }

            // Character Literals
            if (c == '\'') {
                string ch;
                ch += advance();
                if (peek() == '\0' || peek() == '\n') {
                    throw runtime_error("Lexical Error at " + to_string(startLine) + ":" + to_string(startCol) + " - Unterminated char literal");
                }
                ch += advance();
                if (peek() != '\'') {
                    throw runtime_error("Lexical Error at " + to_string(startLine) + ":" + to_string(startCol) + " - Expected closing '");
                }
                ch += advance();
                tokens.push_back({TokenType::CHAR_LIT, ch, startLine, startCol});
                continue;
            }

            // Multi-character Relational Operators
            if (c == '!' && peek(1) == '=') { advance(); advance(); tokens.push_back({TokenType::OP_NEQ, "!=", startLine, startCol}); continue; }
            if (c == '<' && peek(1) == '=') { advance(); advance(); tokens.push_back({TokenType::OP_LTE, "<=", startLine, startCol}); continue; }
            if (c == '>' && peek(1) == '=') { advance(); advance(); tokens.push_back({TokenType::OP_GTE, ">=", startLine, startCol}); continue; }

            // Single-character Operators & Delimiters
            advance();
            switch (c) {
                case '+': tokens.push_back({TokenType::OP_PLUS, "+", startLine, startCol}); break;
                case '-': tokens.push_back({TokenType::OP_MINUS, "-", startLine, startCol}); break;
                case '*': tokens.push_back({TokenType::OP_MUL, "*", startLine, startCol}); break;
                case '/': tokens.push_back({TokenType::OP_DIV, "/", startLine, startCol}); break;
                case '=': tokens.push_back({TokenType::OP_EQ, "=", startLine, startCol}); break;
                case '<': tokens.push_back({TokenType::OP_LT, "<", startLine, startCol}); break;
                case '>': tokens.push_back({TokenType::OP_GT, ">", startLine, startCol}); break;
                case '(': tokens.push_back({TokenType::LPAREN, "(", startLine, startCol}); break;
                case ')': tokens.push_back({TokenType::RPAREN, ")", startLine, startCol}); break;
                case '[': tokens.push_back({TokenType::LBRACK, "[", startLine, startCol}); break;
                case ']': tokens.push_back({TokenType::RBRACK, "]", startLine, startCol}); break;
                case ';': tokens.push_back({TokenType::SEMICOLON, ";", startLine, startCol}); break;
                default:
                    throw runtime_error("Lexical Error at " + to_string(startLine) + ":" + to_string(startCol) +
                                        " - Unexpected character '" + string(1, c) + "'");
            }
        }

        tokens.push_back({TokenType::END_OF_FILE, "EOF", line, col});
        return tokens;
    }
};

vector<Token> runLexer(const string& sourceCode) {
    Lexer lexer(sourceCode);
    return lexer.tokenize();
}

void writeTokensToFile(const vector<Token>& tokens, const string& filename) {
    ofstream out(filename);
    if (!out) throw runtime_error("Failed to open output file: " + filename);
    for (const auto& t : tokens) {
        out << t.line << ":" << t.col << " | " << tokenTypeToString(t.type) << " | " << t.lexeme << "\n";
    }
}

vector<Token> readTokensFromFile(const string& filename) {
    ifstream in(filename);
    if (!in) throw runtime_error("Failed to open token file: " + filename);

    vector<Token> tokens;
    string lineStr;
    while (getline(in, lineStr)) {
        if (lineStr.empty()) continue;
        size_t firstBar = lineStr.find(" | ");
        size_t secondBar = lineStr.find(" | ", firstBar + 3);
        if (firstBar == string::npos || secondBar == string::npos) continue;

        string locPart = lineStr.substr(0, firstBar);
        string typePart = lineStr.substr(firstBar + 3, secondBar - (firstBar + 3));
        string lexemePart = lineStr.substr(secondBar + 3);

        size_t colon = locPart.find(':');
        int ln = stoi(locPart.substr(0, colon));
        int cl = stoi(locPart.substr(colon + 1));

        tokens.push_back({stringToTokenType(typePart), lexemePart, ln, cl});
    }
    return tokens;
}