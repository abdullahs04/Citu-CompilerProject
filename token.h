#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>

enum class TokenType {
    // Data Types
    KW_INT16, KW_INT32, KW_UINT16, KW_UINT32, KW_FLOAT, KW_CHAR,
    // Control Flow & Built-ins
    KW_SHRO, KW_KHATAM, KW_JABTAK, KW_AGAR, KW_WARNA, KW_NIKAL, KW_PRINT,
    // Assignment & Operators
    OP_ASSIGN, // medalo
    OP_PLUS, OP_MINUS, OP_MUL, OP_DIV,
    OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LTE, OP_GTE,
    // Delimiters
    LPAREN, RPAREN, LBRACK, RBRACK, SEMICOLON,
    // Literals & Identifiers
    INT_LIT, FLOAT_LIT, CHAR_LIT, ID,
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int col;
};

inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KW_INT16: return "KW_INT16";
        case TokenType::KW_INT32: return "KW_INT32";
        case TokenType::KW_UINT16: return "KW_UINT16";
        case TokenType::KW_UINT32: return "KW_UINT32";
        case TokenType::KW_FLOAT: return "KW_FLOAT";
        case TokenType::KW_CHAR: return "KW_CHAR";
        case TokenType::KW_SHRO: return "KW_SHRO";
        case TokenType::KW_KHATAM: return "KW_KHATAM";
        case TokenType::KW_JABTAK: return "KW_JABTAK";
        case TokenType::KW_AGAR: return "KW_AGAR";
        case TokenType::KW_WARNA: return "KW_WARNA";
        case TokenType::KW_NIKAL: return "KW_NIKAL";
        case TokenType::KW_PRINT: return "KW_PRINT";
        case TokenType::OP_ASSIGN: return "OP_ASSIGN";
        case TokenType::OP_PLUS: return "OP_PLUS";
        case TokenType::OP_MINUS: return "OP_MINUS";
        case TokenType::OP_MUL: return "OP_MUL";
        case TokenType::OP_DIV: return "OP_DIV";
        case TokenType::OP_EQ: return "OP_EQ";
        case TokenType::OP_NEQ: return "OP_NEQ";
        case TokenType::OP_LT: return "OP_LT";
        case TokenType::OP_GT: return "OP_GT";
        case TokenType::OP_LTE: return "OP_LTE";
        case TokenType::OP_GTE: return "OP_GTE";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACK: return "LBRACK";
        case TokenType::RBRACK: return "RBRACK";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::INT_LIT: return "INT_LIT";
        case TokenType::FLOAT_LIT: return "FLOAT_LIT";
        case TokenType::CHAR_LIT: return "CHAR_LIT";
        case TokenType::ID: return "ID";
        case TokenType::END_OF_FILE: return "EOF";
    }
    return "UNKNOWN";
}

inline TokenType stringToTokenType(const std::string& str) {
    if (str == "KW_INT16") return TokenType::KW_INT16;
    if (str == "KW_INT32") return TokenType::KW_INT32;
    if (str == "KW_UINT16") return TokenType::KW_UINT16;
    if (str == "KW_UINT32") return TokenType::KW_UINT32;
    if (str == "KW_FLOAT") return TokenType::KW_FLOAT;
    if (str == "KW_CHAR") return TokenType::KW_CHAR;
    if (str == "KW_SHRO") return TokenType::KW_SHRO;
    if (str == "KW_KHATAM") return TokenType::KW_KHATAM;
    if (str == "KW_JABTAK") return TokenType::KW_JABTAK;
    if (str == "KW_AGAR") return TokenType::KW_AGAR;
    if (str == "KW_WARNA") return TokenType::KW_WARNA;
    if (str == "KW_NIKAL") return TokenType::KW_NIKAL;
    if (str == "KW_PRINT") return TokenType::KW_PRINT;
    if (str == "OP_ASSIGN") return TokenType::OP_ASSIGN;
    if (str == "OP_PLUS") return TokenType::OP_PLUS;
    if (str == "OP_MINUS") return TokenType::OP_MINUS;
    if (str == "OP_MUL") return TokenType::OP_MUL;
    if (str == "OP_DIV") return TokenType::OP_DIV;
    if (str == "OP_EQ") return TokenType::OP_EQ;
    if (str == "OP_NEQ") return TokenType::OP_NEQ;
    if (str == "OP_LT") return TokenType::OP_LT;
    if (str == "OP_GT") return TokenType::OP_GT;
    if (str == "OP_LTE") return TokenType::OP_LTE;
    if (str == "OP_GTE") return TokenType::OP_GTE;
    if (str == "LPAREN") return TokenType::LPAREN;
    if (str == "RPAREN") return TokenType::RPAREN;
    if (str == "LBRACK") return TokenType::LBRACK;
    if (str == "RBRACK") return TokenType::RBRACK;
    if (str == "SEMICOLON") return TokenType::SEMICOLON;
    if (str == "INT_LIT") return TokenType::INT_LIT;
    if (str == "FLOAT_LIT") return TokenType::FLOAT_LIT;
    if (str == "CHAR_LIT") return TokenType::CHAR_LIT;
    if (str == "ID") return TokenType::ID;
    if (str == "EOF") return TokenType::END_OF_FILE;
    throw std::runtime_error("Unknown token type in intermediate file: " + str);
}

// Forward declarations for Lexer and Parser modules
std::vector<Token> runLexer(const std::string& sourceCode);
void writeTokensToFile(const std::vector<Token>& tokens, const std::string& filename);
std::vector<Token> readTokensFromFile(const std::string& filename);

#endif // TOKEN_H