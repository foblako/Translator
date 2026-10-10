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
    // Проход 1: Статический анализ (регистрация веток и стартовой точки)
    isStaticAnalysisPass = true;

    if (!ctx->startDecl()) {
        throw std::runtime_error("Семантическая ошибка: отсутствует объявление 'start'");
    }

    visit(ctx->startDecl());

    for (auto* branch : ctx->branchDecl()) {
        visit(branch);
    }

    // Проход 1b: Проверка переходов (goto/options) во всех ветках
    for (auto* branch : ctx->branchDecl()) {
        for (auto* stmt : branch->statement()) {
            visit(stmt);
        }
    }

    // Проверка существования стартовой ветки
    if (!registry.hasBranch(registry.getStartBranch())) {
        throw std::runtime_error("Семантическая ошибка: неопределенная стартовая ветка '" + registry.getStartBranch() + "' на строке " + std::to_string(ctx->startDecl()->getStart()->getLine()));
    }

    // Проход 2: Исполнение диалога
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
            throw std::runtime_error("Семантическая ошибка: дублирующееся имя ветки '" + branchName + "' на строке " + std::to_string(ctx->getStart()->getLine()));
        }
        registry.registerBranch(branchName, ctx);
    }
    return {};
}

void DialogueInterpreter::executeBranch(const std::string& branchName) {
    auto* branchCtx = registry.getBranch(branchName);
    if (!branchCtx) {
        throw std::runtime_error("Ошибка выполнения: попытка запустить неопределенную ветку '" + branchName + "'");
    }

    for (auto* stmtCtx : branchCtx->statement()) {
        // Выполняем оператор. goto/choice могут вернуть булево значение, указывающее на необходимость остановки
        antlrcpp::Any result = visit(stmtCtx);
        if (result.type() == typeid(bool) && std::any_cast<bool>(result)) {
            return; // Произошел переход в другую ветку
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
        // Во время статического анализа проверяем корректность целевых веток
        for (auto* optCtx : ctx->option()) {
            std::string targetBranch = optCtx->IDENTIFIER()->getText();
            if (!registry.hasBranch(targetBranch)) {
                throw std::runtime_error("Семантическая ошибка: неизвестная целевая ветка '" + targetBranch + "' на строке " + std::to_string(optCtx->getStart()->getLine()));
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
        // Ошибка потока ввода, прерываем выполнение
        std::cin.clear();
        return true;
    }

    if (choice >= 1 && choice <= (int)optionsTargets.size()) {
        std::cout << std::endl;
        executeBranch(optionsTargets[choice - 1]);
    } else {
        std::cerr << "Недопустимый выбор. Завершение диалога." << std::endl;
    }

    return true; // Прерываем выполнение текущей ветки, так как мы сделали переход
}

bool DialogueInterpreter::evaluateCondition(DialogueScriptParser::ConditionContext* ctx) {
    std::string conditionName = unquote(ctx->STRING()->getText());
    if (ctx->getText().rfind("has_item", 0) == 0) {
        return world.hasItem(conditionName);
    } else if (ctx->getText().rfind("has_flag", 0) == 0) {
        return world.hasFlag(conditionName);
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

    bool condition = evaluateCondition(ctx->condition());

    // Ищем индекс разделителя else
    size_t elseStartIdx = (size_t)-1;
    for (size_t i = 0; i < ctx->children.size(); i++) {
        if (ctx->children[i]->getText() == "else") {
            elseStartIdx = i;
            break;
        }
    }

    // Проходим по каждому оператору и решаем нужно ли его выполнять
    for (auto* stmtCtx : ctx->statement()) {
        bool beforeElse = true;

        for (size_t i = 0; i < ctx->children.size(); i++) {
            if (ctx->children[i] == stmtCtx) {
                if (elseStartIdx != (size_t)-1 && i > elseStartIdx) {
                    beforeElse = false;
                }
                break;
            }
        }

        if (condition && beforeElse) {
            antlrcpp::Any result = visit(stmtCtx);
            if (result.type() == typeid(bool) && std::any_cast<bool>(result)) return true;
        } else if (!condition && !beforeElse) {
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
            throw std::runtime_error("Семантическая ошибка: неизвестная целевая ветка '" + branchName + "' на строке " + std::to_string(ctx->getStart()->getLine()));
        }
        return {};
    }

    executeBranch(branchName);
    return true; // Прерываем выполнение, чтобы перейти
}

antlrcpp::Any DialogueInterpreter::visitActionStmt(DialogueScriptParser::ActionStmtContext* ctx) {
    if (isStaticAnalysisPass) return {};

    std::string fullText = ctx->getText();
    std::string argument = unquote(ctx->STRING()->getText());

    if (fullText.rfind("give_item", 0) == 0) {
        world.giveItem(argument);
        std::cout << "(Предмет добавлен в инвентарь: " << argument << ")" << std::endl;
    } else if (fullText.rfind("take_item", 0) == 0) {
        int count = std::stoi(ctx->INT()->getText());
        if (!world.takeItem(argument, count)) {
            throw std::runtime_error("Ошибка выполнения: недостаточно предметов '" + argument + "' чтобы забрать");
        }
        std::cout << "(Предмет изъят из инвентаря: " << argument << " x" << count << ")" << std::endl;
    } else if (fullText.rfind("set_flag", 0) == 0) {
        bool value = ctx->BOOL()->getText() == "true";
        world.setFlag(argument, value);
        std::cout << "(Флаг установлен: " << argument << " = " << (value ? "true" : "false") << ")" << std::endl;
    }

    return false;
}
