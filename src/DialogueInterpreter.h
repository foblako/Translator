#pragma once

#include "DialogueScriptBaseVisitor.h"
#include "WorldState.h"
#include "BranchRegistry.h"
#include <string>

// Главный класс интерпретатора, обходящий дерево разбора
class DialogueInterpreter : public DialogueScriptBaseVisitor {
    WorldState& mir;               // Ссылка на состояние мира
    BranchRegistry& reestr_vetok;  // Ссылка на реестр веток
    bool etap_stat_analiza = true; // Флаг текущего прохода (анализ или исполнение)

public:
    DialogueInterpreter(WorldState& mir, BranchRegistry& reestr_vetok);

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
    void vypolnitVetku(const std::string& imya_vetki);

    // Вычисление условия (наличие предмета или флага)
    bool vychislitUslovie(DialogueScriptParser::ConditionContext* ctx);

    // Вспомогательная функция для удаления кавычек у строк
    std::string udalitKavychki(const std::string& stroka);
};
