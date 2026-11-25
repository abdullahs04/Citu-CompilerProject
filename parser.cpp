#include "token.h"
#include "ast.h"
#include <fstream>
#include <stdexcept>

using namespace std;

class Parser {
private:
    vector<Token> tokens;
    size_t current = 0;

    const Token& peek() const { return tokens[current]; }
    const Token& previous() const { return tokens[current - 1]; }
    bool isAtEnd() const { return peek().type == TokenType::END_OF_FILE; }

    const Token& advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    bool check(TokenType type) const {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    bool match(const vector<TokenType>& types) {
        for (TokenType t : types) {
            if (check(t)) {
                advance();
                return true;
            }
        }
        return false;
    }

    const Token& consume(TokenType type, const string& msg) {
        if (check(type)) return advance();
        throw runtime_error("Syntax Error at " + to_string(peek().line) + ":" + to_string(peek().col) +
                            " - " + msg + " (found '" + peek().lexeme + "')");
    }

    bool isTypeToken(TokenType t) const {
        return t == TokenType::KW_INT16 || t == TokenType::KW_INT32 ||
               t == TokenType::KW_UINT16 || t == TokenType::KW_UINT32 ||
               t == TokenType::KW_FLOAT || t == TokenType::KW_CHAR;
    }

    shared_ptr<StmtNode> parseStatement() {
        if (isTypeToken(peek().type)) return parseVarDecl();
        if (check(TokenType::KW_JABTAK)) return parseWhileStmt();
        if (check(TokenType::KW_AGAR)) return parseIfStmt();
        if (check(TokenType::KW_PRINT)) return parsePrintStmt();
        if (check(TokenType::KW_NIKAL)) return parseBreakStmt();
        if (check(TokenType::KW_SHRO)) return parseBlock();
        if (check(TokenType::ID)) return parseAssignStmt();

        throw runtime_error("Syntax Error at " + to_string(peek().line) + ":" + to_string(peek().col) +
                            " - Unexpected token '" + peek().lexeme + "' at start of statement");
    }

    shared_ptr<VarDeclNode> parseVarDecl() {
        Token typeTok = advance();
        Token idTok = consume(TokenType::ID, "Expected variable identifier after type");
        string arrSize = "";
        if (match({TokenType::LBRACK})) {
            Token sizeTok = consume(TokenType::INT_LIT, "Expected integer array size inside '[ ]'");
            arrSize = sizeTok.lexeme;
            consume(TokenType::RBRACK, "Expected ']' after array size");
        }
        consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
        return make_shared<VarDeclNode>(typeTok.lexeme, idTok.lexeme, arrSize);
    }

    shared_ptr<LocationNode> parseLocation() {
        Token idTok = consume(TokenType::ID, "Expected identifier");
        shared_ptr<ExprNode> idx = nullptr;
        if (match({TokenType::LBRACK})) {
            idx = parseExpr();
            consume(TokenType::RBRACK, "Expected ']' after array index");
        }
        return make_shared<LocationNode>(idTok.lexeme, idx);
    }

    shared_ptr<AssignNode> parseAssignStmt() {
        auto loc = parseLocation();
        consume(TokenType::OP_ASSIGN, "Expected 'medalo' assignment operator");
        auto val = parseExpr();
        consume(TokenType::SEMICOLON, "Expected ';' after assignment statement");
        return make_shared<AssignNode>(loc, val);
    }

    shared_ptr<PrintNode> parsePrintStmt() {
        consume(TokenType::KW_PRINT, "Expected 'print'");
        consume(TokenType::LPAREN, "Expected '(' after 'print'");
        auto expr = parseExpr();
        consume(TokenType::RPAREN, "Expected ')' after print expression");
        consume(TokenType::SEMICOLON, "Expected ';' after print statement");
        return make_shared<PrintNode>(expr);
    }

    shared_ptr<BreakNode> parseBreakStmt() {
        consume(TokenType::KW_NIKAL, "Expected 'nikal'");
        consume(TokenType::SEMICOLON, "Expected ';' after 'nikal'");
        return make_shared<BreakNode>();
    }

    shared_ptr<BlockNode> parseBlock() {
        consume(TokenType::KW_SHRO, "Expected 'shro' to start block");
        auto block = make_shared<BlockNode>();
        while (!check(TokenType::KW_KHATAM) && !isAtEnd()) {
            block->statements.push_back(parseStatement());
        }
        consume(TokenType::KW_KHATAM, "Expected 'khatam' to close block");
        return block;
    }

    shared_ptr<BinaryExprNode> parseCondition() {
        auto left = parseExpr();
        if (!match({TokenType::OP_EQ, TokenType::OP_NEQ, TokenType::OP_LT,
                    TokenType::OP_GT, TokenType::OP_LTE, TokenType::OP_GTE})) {
            throw runtime_error("Syntax Error at " + to_string(peek().line) + ":" + to_string(peek().col) +
                                " - Expected relational operator (=, !=, <, >, <=, >=)");
        }
        string op = previous().lexeme;
        auto right = parseExpr();
        return make_shared<BinaryExprNode>(op, left, right);
    }

    shared_ptr<WhileNode> parseWhileStmt() {
        consume(TokenType::KW_JABTAK, "Expected 'jabtak'");
        consume(TokenType::LPAREN, "Expected '(' after 'jabtak'");
        auto cond = parseCondition();
        consume(TokenType::RPAREN, "Expected ')' after condition");
        auto body = parseBlock();
        return make_shared<WhileNode>(cond, body);
    }

    shared_ptr<IfNode> parseIfStmt() {
        consume(TokenType::KW_AGAR, "Expected 'agar'");
        consume(TokenType::LPAREN, "Expected '(' after 'agar'");
        auto cond = parseCondition();
        consume(TokenType::RPAREN, "Expected ')' after condition");
        auto thenBlk = parseBlock();
        shared_ptr<BlockNode> elseBlk = nullptr;
        if (match({TokenType::KW_WARNA})) {
            elseBlk = parseBlock();
        }
        return make_shared<IfNode>(cond, thenBlk, elseBlk);
    }

    shared_ptr<ExprNode> parseExpr() {
        auto expr = parseTerm();
        while (match({TokenType::OP_PLUS, TokenType::OP_MINUS})) {
            string op = previous().lexeme;
            auto right = parseTerm();
            expr = make_shared<BinaryExprNode>(op, expr, right);
        }
        return expr;
    }

    shared_ptr<ExprNode> parseTerm() {
        auto expr = parseFactor();
        while (match({TokenType::OP_MUL, TokenType::OP_DIV})) {
            string op = previous().lexeme;
            auto right = parseFactor();
            expr = make_shared<BinaryExprNode>(op, expr, right);
        }
        return expr;
    }

    shared_ptr<ExprNode> parseFactor() {
        if (match({TokenType::LPAREN})) {
            auto expr = parseExpr();
            consume(TokenType::RPAREN, "Expected ')' after grouped expression");
            return expr;
        }
        if (match({TokenType::INT_LIT})) return make_shared<LiteralNode>(previous().lexeme, "INT");
        if (match({TokenType::FLOAT_LIT})) return make_shared<LiteralNode>(previous().lexeme, "FLOAT");
        if (match({TokenType::CHAR_LIT})) return make_shared<LiteralNode>(previous().lexeme, "CHAR");
        if (check(TokenType::ID)) return parseLocation();

        throw runtime_error("Syntax Error at " + to_string(peek().line) + ":" + to_string(peek().col) +
                            " - Expected expression factor, found '" + peek().lexeme + "'");
    }

public:
    Parser(const vector<Token>& toks) : tokens(toks) {}

    shared_ptr<ProgramNode> parseProgram() {
        auto prog = make_shared<ProgramNode>();
        while (!isAtEnd()) {
            prog->statements.push_back(parseStatement());
        }
        return prog;
    }
};

shared_ptr<ProgramNode> runParser(const vector<Token>& tokens) {
    Parser parser(tokens);
    return parser.parseProgram();
}

void writeASTToFile(const shared_ptr<ProgramNode>& ast, const string& filename) {
    ofstream out(filename);
    if (!out) throw runtime_error("Failed to open output file: " + filename);
    out << ast->toJSON() << "\n";
}