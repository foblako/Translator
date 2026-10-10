#pragma once

#include "DialogueScriptBaseVisitor.h"
#include "WorldState.h"
#include "BranchRegistry.h"
#include <string>

class DialogueInterpreter : public DialogueScriptBaseVisitor {
    WorldState& world;
    BranchRegistry& registry;
    bool isStaticAnalysisPass = true;

public:
    DialogueInterpreter(WorldState& world, BranchRegistry& registry);

    antlrcpp::Any visitProgram(DialogueScriptParser::ProgramContext* ctx) override;
    antlrcpp::Any visitStartDecl(DialogueScriptParser::StartDeclContext* ctx) override;
    antlrcpp::Any visitBranchDecl(DialogueScriptParser::BranchDeclContext* ctx) override;

    antlrcpp::Any visitNpcStmt(DialogueScriptParser::NpcStmtContext* ctx) override;
    antlrcpp::Any visitChoiceStmt(DialogueScriptParser::ChoiceStmtContext* ctx) override;
    antlrcpp::Any visitIfStmt(DialogueScriptParser::IfStmtContext* ctx) override;
    antlrcpp::Any visitGotoStmt(DialogueScriptParser::GotoStmtContext* ctx) override;
    antlrcpp::Any visitActionStmt(DialogueScriptParser::ActionStmtContext* ctx) override;

private:
    void executeBranch(const std::string& branchName);
    bool evaluateCondition(DialogueScriptParser::ConditionContext* ctx);

    // Helper string util
    std::string unquote(const std::string& str);
};
