#pragma once

#include "DialogueScriptBaseVisitor.h"
#include "WorldState.h"
#include "BranchRegistry.h"
#include <string>

// Главный класс интерпретатора, обходящий дерево разбора
class DialogueInterpreter : public DialogueScriptBaseVisitor {
    WorldState& world;               // Ссылка на состояние мира
    BranchRegistry& registry;        // Ссылка на реестр веток
    bool isStaticAnalysisPass = true; // Флаг текущего прохода (анализ или исполнение)

public:
    DialogueInterpreter(WorldState& world, BranchRegistry& registry);

    // Переопределение базовых методов обхода ANTLR
    antlrcpp::Any visitProgram(DialogueScriptParser::ProgramContext* ctx) override;
    antlrcpp::Any visitStartDecl(DialogueScriptParser::StartDeclContext* ctx) override;
    antlrcpp::Any visitBranchDecl(DialogueScriptParser::BranchDeclContext* ctx) override;

    antlrcpp::Any visitNpcStmt(DialogueScriptParser::NpcStmtContext* ctx) override;
    antlrcpp::Any visitChoiceStmt(DialogueScriptParser::ChoiceStmtContext* ctx) override;
    antlrcpp::Any visitIfStmt(DialogueScriptParser::IfStmtContext* ctx) override;
    antlrcpp::Any visitGotoStmt(DialogueScriptParser::GotoStmtContext* ctx) override;
    antlrcpp::Any visitActionStmt(DialogueScriptParser::ActionStmtContext* ctx) override;

private:
    // Исполнение конкретной ветки
    void executeBranch(const std::string& branchName);

    // Вычисление условия (наличие предмета или флага)
    bool evaluateCondition(DialogueScriptParser::ConditionContext* ctx);

    // Вспомогательная функция для удаления кавычек у строк
    std::string unquote(const std::string& str);
};
