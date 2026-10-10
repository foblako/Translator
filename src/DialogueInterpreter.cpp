#include "DialogueInterpreter.h"
#include <iostream>
#include <stdexcept>

DialogueInterpreter::DialogueInterpreter(WorldState& world, BranchRegistry& registry)
    : world(world), registry(registry) {}

std::string DialogueInterpreter::unquote(const std::string& str) {
    if (str.length() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.length() - 2);
    }
    return str;
}

antlrcpp::Any DialogueInterpreter::visitProgram(DialogueScriptParser::ProgramContext* ctx) {
    // Pass 1: Static analysis (Register branches and start point)
    isStaticAnalysisPass = true;

    if (!ctx->startDecl()) {
        throw std::runtime_error("Semantic error: missing 'start' declaration");
    }

    visit(ctx->startDecl());

    for (auto* branch : ctx->branchDecl()) {
        visit(branch);
    }

    // Validate that the start branch exists
    if (!registry.hasBranch(registry.getStartBranch())) {
        throw std::runtime_error("Semantic error: undefined branch '" + registry.getStartBranch() + "' at line " + std::to_string(ctx->startDecl()->getStart()->getLine()));
    }

    // Pass 2: Execution
    isStaticAnalysisPass = false;
    executeBranch(registry.getStartBranch());
    std::cout << "\nДиалог завершен." << std::endl;
    return {};
}

antlrcpp::Any DialogueInterpreter::visitStartDecl(DialogueScriptParser::StartDeclContext* ctx) {
    if (isStaticAnalysisPass) {
        registry.setStartBranch(ctx->IDENTIFIER()->getText());
    }
    return {};
}

antlrcpp::Any DialogueInterpreter::visitBranchDecl(DialogueScriptParser::BranchDeclContext* ctx) {
    if (isStaticAnalysisPass) {
        std::string branchName = ctx->IDENTIFIER()->getText();
        if (registry.hasBranch(branchName)) {
            throw std::runtime_error("Semantic error: duplicate branch name '" + branchName + "' at line " + std::to_string(ctx->getStart()->getLine()));
        }
        registry.registerBranch(branchName, ctx);
    }
    return {};
}

void DialogueInterpreter::executeBranch(const std::string& branchName) {
    auto* branchCtx = registry.getBranch(branchName);
    if (!branchCtx) {
        throw std::runtime_error("Runtime error: attempting to execute undefined branch '" + branchName + "'");
    }

    for (auto* stmtCtx : branchCtx->statement()) {
        // Evaluate the statement. A goto/choice might return a boolean indicating to stop executing the current branch.
        antlrcpp::Any result = visit(stmtCtx);
        if (result.type() == typeid(bool) && std::any_cast<bool>(result)) {
            return; // We transitioned away from this branch
        }
    }
}

antlrcpp::Any DialogueInterpreter::visitNpcStmt(DialogueScriptParser::NpcStmtContext* ctx) {
    if (isStaticAnalysisPass) return {};

    std::string name = unquote(ctx->STRING(0)->getText());
    std::string text = unquote(ctx->STRING(1)->getText());

    std::cout << "[" << name << "]: " << text << std::endl;
    return {};
}

antlrcpp::Any DialogueInterpreter::visitChoiceStmt(DialogueScriptParser::ChoiceStmtContext* ctx) {
    if (isStaticAnalysisPass) {
        for (auto* optCtx : ctx->option()) {
            std::string branchName = optCtx->IDENTIFIER()->getText();
            if (!registry.hasBranch(branchName)) {
                throw std::runtime_error("Semantic error: undefined branch '" + branchName + "' at line " + std::to_string(optCtx->getStart()->getLine()));
            }
        }
        return {};
    }

    std::cout << std::endl;

    std::vector<std::string> optionsText;
    std::vector<std::string> optionsTargets;

    int index = 1;
    for (auto* optCtx : ctx->option()) {
        std::string text = unquote(optCtx->STRING()->getText());
        std::string target = optCtx->IDENTIFIER()->getText();
        optionsText.push_back(text);
        optionsTargets.push_back(target);
        std::cout << index << ") " << text << std::endl;
        index++;
    }

    std::cout << "\nВаш выбор (1-" << optionsText.size() << "): ";
    int choice = -1;
    if (!(std::cin >> choice)) {
        // End of input or failure, just abort gracefully (useful for testing)
        std::cin.clear();
        return true;
    }

    if (choice >= 1 && choice <= (int)optionsTargets.size()) {
        std::cout << std::endl;
        executeBranch(optionsTargets[choice - 1]);
    } else {
        std::cerr << "Invalid choice. Aborting dialog." << std::endl;
    }

    return true; // Stop execution of the current branch since we picked a choice
}

bool DialogueInterpreter::evaluateCondition(DialogueScriptParser::ConditionContext* ctx) {
    std::string name = unquote(ctx->STRING()->getText());
    if (ctx->getText().rfind("has_item", 0) == 0) {
        return world.hasItem(name);
    } else if (ctx->getText().rfind("has_flag", 0) == 0) {
        return world.hasFlag(name);
    }
    return false;
}

antlrcpp::Any DialogueInterpreter::visitIfStmt(DialogueScriptParser::IfStmtContext* ctx) {
    if (isStaticAnalysisPass) {
        for (auto* stmtCtx : ctx->statement()) {
            visit(stmtCtx);
        }
        return {};
    }

    bool cond = evaluateCondition(ctx->condition());

    // The antlr4 generated context groups all statements.
    // We need to count where the 'else' splits them.
    // Fortunately, we can check how many '{' there are, but since statement* is flat,
    // it's easier to manually traverse based on the AST structure.
    // A better approach for the grammar is often to use blocks `if (cond) block (else block)?`
    // Since we used statement* flatly inside the braces, let's find the split point.
    // But ANTLR actually provides ctx->statement() as a single list.
    // Wait, in `if condition '{' statement* '}' ('else' '{' statement* '}')?`
    // ANTLR merges both `statement*` into one list!
    // We can't distinguish them easily without checking start/stop tokens.
    // Let's modify the grammar locally or use token indexes.
    // Since we can't modify grammar here easily, let's use token indexes to split the statements.

    size_t elseStartIdx = (size_t)-1;
    for (size_t i = 0; i < ctx->children.size(); i++) {
        if (ctx->children[i]->getText() == "else") {
            elseStartIdx = i;
            break;
        }
    }

    for (auto* stmtCtx : ctx->statement()) {
        bool isBeforeElse = true;

        // Check if the statement is before or after the 'else' token
        for (size_t i = 0; i < ctx->children.size(); i++) {
            if (ctx->children[i] == stmtCtx) {
                if (elseStartIdx != (size_t)-1 && i > elseStartIdx) {
                    isBeforeElse = false;
                }
                break;
            }
        }

        if (cond && isBeforeElse) {
            antlrcpp::Any result = visit(stmtCtx);
            if (result.type() == typeid(bool) && std::any_cast<bool>(result)) return true;
        } else if (!cond && !isBeforeElse) {
            antlrcpp::Any result = visit(stmtCtx);
            if (result.type() == typeid(bool) && std::any_cast<bool>(result)) return true;
        }
    }

    return false;
}

antlrcpp::Any DialogueInterpreter::visitGotoStmt(DialogueScriptParser::GotoStmtContext* ctx) {
    std::string branchName = ctx->IDENTIFIER()->getText();
    if (isStaticAnalysisPass) {
        if (!registry.hasBranch(branchName)) {
            throw std::runtime_error("Semantic error: undefined branch '" + branchName + "' at line " + std::to_string(ctx->getStart()->getLine()));
        }
        return {};
    }

    executeBranch(branchName);
    return true; // Stop execution of the current branch since we jumped
}

antlrcpp::Any DialogueInterpreter::visitActionStmt(DialogueScriptParser::ActionStmtContext* ctx) {
    if (isStaticAnalysisPass) return {};

    std::string text = ctx->getText();
    std::string name = unquote(ctx->STRING()->getText());

    if (text.rfind("give_item", 0) == 0) {
        world.giveItem(name);
        std::cout << "(Предмет добавлен в инвентарь: " << name << ")" << std::endl;
    } else if (text.rfind("take_item", 0) == 0) {
        int count = std::stoi(ctx->INT()->getText());
        if (!world.takeItem(name, count)) {
            throw std::runtime_error("Runtime error: not enough items '" + name + "' to take");
        }
        std::cout << "(Предмет изъят из инвентаря: " << name << " x" << count << ")" << std::endl;
    } else if (text.rfind("set_flag", 0) == 0) {
        bool val = ctx->BOOL()->getText() == "true";
        world.setFlag(name, val);
        std::cout << "(Флаг установлен: " << name << " = " << (val ? "true" : "false") << ")" << std::endl;
    }

    return false;
}
