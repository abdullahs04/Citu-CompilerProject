#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

struct SymbolEntry {
    std::string name;
    std::string dataType; // int16, int32, uint16, uint32, float, char
    bool isArray;
    int arraySize;
    int scopeLevel;
    int memoryOffset;
};

class SemanticAnalyzer {
private:
    std::vector<std::unordered_map<std::string, SymbolEntry>> scopes;
    std::vector<SymbolEntry> allSymbolsHistory; // Preserves all symbols for output.symtab
    int currentScopeLevel = 0;
    int currentOffset = 0;
    int loopDepth = 0;

    void enterScope();
    void exitScope();
    int getTypeSize(const std::string& type) const;
    bool isIntegerType(const std::string& type) const;
    bool isNumericType(const std::string& type) const;
    bool areTypesCompatible(const std::string& targetType, const std::string& exprType) const;

    void declareSymbol(const std::string& name, const std::string& type, bool isArray, int arraySize);
    SymbolEntry lookupSymbol(const std::string& name) const;

    void analyzeStmt(const std::shared_ptr<StmtNode>& stmt);
    void analyzeBlock(const std::shared_ptr<BlockNode>& block, bool createNewScope = true);
    std::string analyzeExpr(const std::shared_ptr<ExprNode>& expr);
    std::string analyzeLocation(const std::shared_ptr<LocationNode>& loc);

public:
    SemanticAnalyzer();
    void analyze(const std::shared_ptr<ProgramNode>& program);
    const std::vector<SymbolEntry>& getSymbolTable() const { return allSymbolsHistory; }
};

std::vector<SymbolEntry> runSemanticAnalysis(const std::shared_ptr<ProgramNode>& program);
void writeSymbolTableToFile(const std::vector<SymbolEntry>& symbols, const std::string& filename);

#endif // SEMANTIC_H