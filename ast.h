#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual std::string toJSON(int indent = 0) const = 0;

protected:
    std::string pad(int indent) const { return std::string(indent * 2, ' '); }
};

struct ExprNode : ASTNode {};
struct StmtNode : ASTNode {};

struct LiteralNode : ExprNode {
    std::string value;
    std::string litType;
    LiteralNode(std::string v, std::string t) : value(v), litType(t) {}
    std::string toJSON(int indent) const override {
        return "{ \"node\": \"Literal\", \"type\": \"" + litType + "\", \"value\": \"" + value + "\" }";
    }
};

struct LocationNode : ExprNode {
    std::string name;
    std::shared_ptr<ExprNode> index;
    LocationNode(std::string n, std::shared_ptr<ExprNode> idx = nullptr) : name(n), index(idx) {}
    std::string toJSON(int indent) const override {
        if (!index) {
            return "{ \"node\": \"Variable\", \"name\": \"" + name + "\" }";
        }
        return "{ \"node\": \"ArrayAccess\", \"name\": \"" + name + "\", \"index\": " + index->toJSON(indent) + " }";
    }
};

struct BinaryExprNode : ExprNode {
    std::string op;
    std::shared_ptr<ExprNode> left;
    std::shared_ptr<ExprNode> right;
    BinaryExprNode(std::string o, std::shared_ptr<ExprNode> l, std::shared_ptr<ExprNode> r) : op(o), left(l), right(r) {}
    std::string toJSON(int indent) const override {
        return "{ \"node\": \"BinaryExpr\", \"op\": \"" + op + "\", \"left\": " +
               left->toJSON(indent) + ", \"right\": " + right->toJSON(indent) + " }";
    }
};

struct VarDeclNode : StmtNode {
    std::string dataType;
    std::string name;
    std::string arraySize;
    VarDeclNode(std::string dt, std::string n, std::string sz = "") : dataType(dt), name(n), arraySize(sz) {}
    std::string toJSON(int indent) const override {
        std::string res = pad(indent) + "{ \"node\": \"VarDecl\", \"type\": \"" + dataType + "\", \"name\": \"" + name + "\"";
        if (!arraySize.empty()) res += ", \"arraySize\": " + arraySize;
        res += " }";
        return res;
    }
};

struct AssignNode : StmtNode {
    std::shared_ptr<LocationNode> target;
    std::shared_ptr<ExprNode> expr;
    AssignNode(std::shared_ptr<LocationNode> t, std::shared_ptr<ExprNode> e) : target(t), expr(e) {}
    std::string toJSON(int indent) const override {
        return pad(indent) + "{ \"node\": \"Assign\", \"target\": " + target->toJSON(indent) +
               ", \"value\": " + expr->toJSON(indent) + " }";
    }
};

struct PrintNode : StmtNode {
    std::shared_ptr<ExprNode> expr;
    PrintNode(std::shared_ptr<ExprNode> e) : expr(e) {}
    std::string toJSON(int indent) const override {
        return pad(indent) + "{ \"node\": \"Print\", \"expr\": " + expr->toJSON(indent) + " }";
    }
};

struct BreakNode : StmtNode {
    std::string toJSON(int indent) const override {
        return pad(indent) + "{ \"node\": \"Break\" }";
    }
};

struct BlockNode : StmtNode {
    std::vector<std::shared_ptr<StmtNode>> statements;
    std::string toJSON(int indent) const override {
        std::string res = pad(indent) + "{\n" + pad(indent + 1) + "\"node\": \"Block\",\n" + pad(indent + 1) + "\"statements\": [\n";
        for (size_t i = 0; i < statements.size(); ++i) {
            res += statements[i]->toJSON(indent + 2);
            if (i + 1 < statements.size()) res += ",";
            res += "\n";
        }
        res += pad(indent + 1) + "]\n" + pad(indent) + "}";
        return res;
    }
};

struct IfNode : StmtNode {
    std::shared_ptr<BinaryExprNode> condition;
    std::shared_ptr<BlockNode> thenBlock;
    std::shared_ptr<BlockNode> elseBlock;
    IfNode(std::shared_ptr<BinaryExprNode> c, std::shared_ptr<BlockNode> t, std::shared_ptr<BlockNode> e = nullptr)
        : condition(c), thenBlock(t), elseBlock(e) {}
    std::string toJSON(int indent) const override {
        std::string res = pad(indent) + "{\n" + pad(indent + 1) + "\"node\": \"If\",\n" +
                     pad(indent + 1) + "\"condition\": " + condition->toJSON(indent + 1) + ",\n" +
                     pad(indent + 1) + "\"then\":\n" + thenBlock->toJSON(indent + 1);
        if (elseBlock) {
            res += ",\n" + pad(indent + 1) + "\"else\":\n" + elseBlock->toJSON(indent + 1);
        }
        res += "\n" + pad(indent) + "}";
        return res;
    }
};

struct WhileNode : StmtNode {
    std::shared_ptr<BinaryExprNode> condition;
    std::shared_ptr<BlockNode> body;
    WhileNode(std::shared_ptr<BinaryExprNode> c, std::shared_ptr<BlockNode> b) : condition(c), body(b) {}
    std::string toJSON(int indent) const override {
        return pad(indent) + "{\n" + pad(indent + 1) + "\"node\": \"While\",\n" +
               pad(indent + 1) + "\"condition\": " + condition->toJSON(indent + 1) + ",\n" +
               pad(indent + 1) + "\"body\":\n" + body->toJSON(indent + 1) + "\n" + pad(indent) + "}";
    }
};

struct ProgramNode : ASTNode {
    std::vector<std::shared_ptr<StmtNode>> statements;
    std::string toJSON(int indent = 0) const override {
        std::string res = "{\n  \"node\": \"Program\",\n  \"statements\": [\n";
        for (size_t i = 0; i < statements.size(); ++i) {
            res += statements[i]->toJSON(2);
            if (i + 1 < statements.size()) res += ",";
            res += "\n";
        }
        res += "  ]\n}";
        return res;
    }
};

std::shared_ptr<ProgramNode> runParser(const std::vector<Token>& tokens);
void writeASTToFile(const std::shared_ptr<ProgramNode>& ast, const std::string& filename);

#endif // AST_H