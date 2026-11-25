#include "semantic.h"
#include <fstream>
#include <stdexcept>

using namespace std;

SemanticAnalyzer::SemanticAnalyzer() {
    // Initialize global scope (Scope 0)
    scopes.emplace_back();
}

void SemanticAnalyzer::enterScope() {
    currentScopeLevel++;
    scopes.emplace_back();
}

void SemanticAnalyzer::exitScope() {
    if (scopes.size() > 1) {
        scopes.pop_back();
        currentScopeLevel--;
    }
}

int SemanticAnalyzer::getTypeSize(const string& type) const {
    if (type == "char") return 1;
    if (type == "int16" || type == "uint16") return 2;
    if (type == "int32" || type == "uint32" || type == "float") return 4;
    return 4;
}

bool SemanticAnalyzer::isIntegerType(const string& type) const {
    return type == "int16" || type == "int32" ||
           type == "uint16" || type == "uint32" || type == "char";
}

bool SemanticAnalyzer::isNumericType(const string& type) const {
    return isIntegerType(type) || type == "float";
}

bool SemanticAnalyzer::areTypesCompatible(const string& targetType, const string& exprType) const {
    if (targetType == exprType) return true;
    // Allow integer promotions / assignments between integer types
    if (isIntegerType(targetType) && isIntegerType(exprType)) return true;
    // Allow assigning integer literals/values to float
    if (targetType == "float" && isNumericType(exprType)) return true;
    return false;
}

void SemanticAnalyzer::declareSymbol(const string& name, const string& type, bool isArray, int arraySize) {
    auto& currentMap = scopes.back();
    if (currentMap.find(name) != currentMap.end()) {
        throw runtime_error("Semantic Error: Variable '" + name + "' is already declared in scope " + to_string(currentScopeLevel));
    }

    if (isArray && arraySize <= 0) {
        throw runtime_error("Semantic Error: Array '" + name + "' must have a positive size (got " + to_string(arraySize) + ")");
    }

    int elementSize = getTypeSize(type);
    int totalBytes = isArray ? (elementSize * arraySize) : elementSize;

    SymbolEntry entry{name, type, isArray, isArray ? arraySize : 1, currentScopeLevel, currentOffset};
    currentMap[name] = entry;
    allSymbolsHistory.push_back(entry);
    currentOffset += totalBytes;
}

SymbolEntry SemanticAnalyzer::lookupSymbol(const string& name) const {
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            return it->second;
        }
    }
    throw runtime_error("Semantic Error: Undeclared identifier '" + name + "'");
}

void SemanticAnalyzer::analyze(const shared_ptr<ProgramNode>& program) {
    for (const auto& stmt : program->statements) {
        analyzeStmt(stmt);
    }
}

void SemanticAnalyzer::analyzeBlock(const shared_ptr<BlockNode>& block, bool createNewScope) {
    if (createNewScope) enterScope();
    for (const auto& stmt : block->statements) {
        analyzeStmt(stmt);
    }
    if (createNewScope) exitScope();
}

void SemanticAnalyzer::analyzeStmt(const shared_ptr<StmtNode>& stmt) {
    if (auto varDecl = dynamic_pointer_cast<VarDeclNode>(stmt)) {
        bool isArr = !varDecl->arraySize.empty();
        int sz = isArr ? stoi(varDecl->arraySize) : 1;
        declareSymbol(varDecl->name, varDecl->dataType, isArr, sz);
        return;
    }

    if (auto assign = dynamic_pointer_cast<AssignNode>(stmt)) {
        string targetType = analyzeLocation(assign->target);
        string valType = analyzeExpr(assign->expr);

        if (!areTypesCompatible(targetType, valType)) {
            throw runtime_error("Semantic Error: Cannot assign expression of type '" + valType +
                                "' to '" + assign->target->name + "' of type '" + targetType + "'");
        }
        return;
    }

    if (auto printStmt = dynamic_pointer_cast<PrintNode>(stmt)) {
        analyzeExpr(printStmt->expr);
        return;
    }

    if (dynamic_pointer_cast<BreakNode>(stmt)) {
        if (loopDepth <= 0) {
            throw runtime_error("Semantic Error: 'nikal' (break) statement used outside of a 'jabtak' loop");
        }
        return;
    }

    if (auto block = dynamic_pointer_cast<BlockNode>(stmt)) {
        analyzeBlock(block, true);
        return;
    }

    if (auto whileStmt = dynamic_pointer_cast<WhileNode>(stmt)) {
        string condType = analyzeExpr(whileStmt->condition);
        if (condType != "bool") {
            throw runtime_error("Semantic Error: 'jabtak' condition must evaluate to a boolean comparison");
        }
        loopDepth++;
        analyzeBlock(whileStmt->body, true);
        loopDepth--;
        return;
    }

    if (auto ifStmt = dynamic_pointer_cast<IfNode>(stmt)) {
        string condType = analyzeExpr(ifStmt->condition);
        if (condType != "bool") {
            throw runtime_error("Semantic Error: 'agar' condition must evaluate to a boolean comparison");
        }
        analyzeBlock(ifStmt->thenBlock, true);
        if (ifStmt->elseBlock) {
            analyzeBlock(ifStmt->elseBlock, true);
        }
        return;
    }
}

string SemanticAnalyzer::analyzeLocation(const shared_ptr<LocationNode>& loc) {
    SymbolEntry sym = lookupSymbol(loc->name);

    if (loc->index) {
        if (!sym.isArray) {
            throw runtime_error("Semantic Error: Variable '" + loc->name + "' is not an array and cannot be indexed");
        }
        string idxType = analyzeExpr(loc->index);
        if (!isIntegerType(idxType)) {
            throw runtime_error("Semantic Error: Array index for '" + loc->name + "' must be an integer type (got '" + idxType + "')");
        }
        // Static out-of-bounds check if index is a constant integer literal
        if (auto lit = dynamic_pointer_cast<LiteralNode>(loc->index)) {
            if (lit->litType == "INT") {
                int idxVal = stoi(lit->value);
                if (idxVal < 0 || idxVal >= sym.arraySize) {
                    throw runtime_error("Semantic Error: Array index " + to_string(idxVal) +
                                        " out of bounds for '" + loc->name + "[" + to_string(sym.arraySize) + "]'");
                }
            }
        }
        return sym.dataType;
    } else {
        if (sym.isArray) {
            throw runtime_error("Semantic Error: Array '" + loc->name + "' cannot be used without an index '[ ]'");
        }
        return sym.dataType;
    }
}

string SemanticAnalyzer::analyzeExpr(const shared_ptr<ExprNode>& expr) {
    if (auto lit = dynamic_pointer_cast<LiteralNode>(expr)) {
        if (lit->litType == "INT") return "int32";
        if (lit->litType == "FLOAT") return "float";
        if (lit->litType == "CHAR") return "char";
    }

    if (auto loc = dynamic_pointer_cast<LocationNode>(expr)) {
        return analyzeLocation(loc);
    }

    if (auto bin = dynamic_pointer_cast<BinaryExprNode>(expr)) {
        string leftType = analyzeExpr(bin->left);
        string rightType = analyzeExpr(bin->right);

        if (!isNumericType(leftType) || !isNumericType(rightType)) {
            throw runtime_error("Semantic Error: Invalid operand types '" + leftType + "' and '" +
                                rightType + "' for operator '" + bin->op + "'");
        }

        // Relational operators produce boolean result
        if (bin->op == "=" || bin->op == "!=" || bin->op == "<" ||
            bin->op == ">" || bin->op == "<=" || bin->op == ">=") {
            return "bool";
        }

        // Arithmetic operators: promote to float if either operand is float
        if (leftType == "float" || rightType == "float") {
            return "float";
        }
        return "int32";
    }

    throw runtime_error("Semantic Error: Unknown expression node encountered");
}

vector<SymbolEntry> runSemanticAnalysis(const shared_ptr<ProgramNode>& program) {
    SemanticAnalyzer analyzer;
    analyzer.analyze(program);
    return analyzer.getSymbolTable();
}

void writeSymbolTableToFile(const vector<SymbolEntry>& symbols, const string& filename) {
    ofstream out(filename);
    if (!out) throw runtime_error("Failed to open output file: " + filename);

    out << "SCOPE,NAME,TYPE,IS_ARRAY,SIZE,OFFSET\n";
    for (const auto& s : symbols) {
        out << s.scopeLevel << ","
            << s.name << ","
            << s.dataType << ","
            << (s.isArray ? "true" : "false") << ","
            << s.arraySize << ","
            << s.memoryOffset << "\n";
    }
}