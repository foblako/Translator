#include "DialogueInterpreter.h"
#include <iostream>
#include <stdexcept>

DialogueInterpreter::DialogueInterpreter(WorldState& mir, BranchRegistry& reestr_vetok)
    : mir(mir), reestr_vetok(reestr_vetok) {}

std::string DialogueInterpreter::udalitKavychki(const std::string& stroka) {
    if (stroka.length() >= 2 && stroka.front() == '"' && stroka.back() == '"') {
        return stroka.substr(1, stroka.length() - 2);
    }
    return stroka;
}

antlrcpp::Any DialogueInterpreter::visitProgram(DialogueScriptParser::ProgramContext* ctx) {
    // Проход 1: Статический анализ (регистрация веток и стартовой точки)
    etap_stat_analiza = true;

    if (!ctx->startDecl()) {
        throw std::runtime_error("Семантическая ошибка: отсутствует объявление 'start'");
    }

    visit(ctx->startDecl());

    for (auto* vetka : ctx->branchDecl()) {
        visit(vetka);
    }

    // Проверка существования стартовой ветки
    if (!reestr_vetok.sushchestvuetVetka(reestr_vetok.poluchitStart())) {
        throw std::runtime_error("Семантическая ошибка: неопределенная стартовая ветка '" + reestr_vetok.poluchitStart() + "' на строке " + std::to_string(ctx->startDecl()->getStart()->getLine()));
    }

    // Проход 2: Исполнение диалога
    etap_stat_analiza = false;
    vypolnitVetku(reestr_vetok.poluchitStart());
    std::cout << "\nДиалог завершен." << std::endl;
    return {};
}

antlrcpp::Any DialogueInterpreter::visitStartDecl(DialogueScriptParser::StartDeclContext* ctx) {
    if (etap_stat_analiza) {
        reestr_vetok.ustanovitStart(ctx->IDENTIFIER()->getText());
    }
    return {};
}

antlrcpp::Any DialogueInterpreter::visitBranchDecl(DialogueScriptParser::BranchDeclContext* ctx) {
    if (etap_stat_analiza) {
        std::string imya_vetki = ctx->IDENTIFIER()->getText();
        if (reestr_vetok.sushchestvuetVetka(imya_vetki)) {
            throw std::runtime_error("Семантическая ошибка: дублирующееся имя ветки '" + imya_vetki + "' на строке " + std::to_string(ctx->getStart()->getLine()));
        }
        reestr_vetok.zaregistrirovatVetku(imya_vetki, ctx);
    }
    return {};
}

void DialogueInterpreter::vypolnitVetku(const std::string& imya_vetki) {
    auto* kontekst_vetki = reestr_vetok.poluchitVetku(imya_vetki);
    if (!kontekst_vetki) {
        throw std::runtime_error("Ошибка выполнения: попытка запустить неопределенную ветку '" + imya_vetki + "'");
    }

    for (auto* kontekst_operatora : kontekst_vetki->statement()) {
        // Выполняем оператор. goto/choice могут вернуть булево значение, указывающее на необходимость остановки
        antlrcpp::Any rezultat = visit(kontekst_operatora);
        if (rezultat.type() == typeid(bool) && std::any_cast<bool>(rezultat)) {
            return; // Произошел переход в другую ветку
        }
    }
}

antlrcpp::Any DialogueInterpreter::visitNpcStmt(DialogueScriptParser::NpcStmtContext* ctx) {
    if (etap_stat_analiza) return {};

    std::string imya = udalitKavychki(ctx->STRING(0)->getText());
    std::string tekst = udalitKavychki(ctx->STRING(1)->getText());

    std::cout << "[" << imya << "]: " << tekst << std::endl;
    return {};
}

antlrcpp::Any DialogueInterpreter::visitChoiceStmt(DialogueScriptParser::ChoiceStmtContext* ctx) {
    if (etap_stat_analiza) {
        // Во время статического анализа проверяем корректность целевых веток
        for (auto* kontekst_opcii : ctx->option()) {
            std::string celevaya_vetka = kontekst_opcii->IDENTIFIER()->getText();
            if (!reestr_vetok.sushchestvuetVetka(celevaya_vetka)) {
                throw std::runtime_error("Семантическая ошибка: неизвестная целевая ветка '" + celevaya_vetka + "' на строке " + std::to_string(kontekst_opcii->getStart()->getLine()));
            }
        }
        return {};
    }

    std::cout << std::endl;

    std::vector<std::string> teks_opcij;
    std::vector<std::string> tseli_opcij;

    int indeks = 1;
    for (auto* kontekst_opcii : ctx->option()) {
        std::string tekst = udalitKavychki(kontekst_opcii->STRING()->getText());
        std::string tsel = kontekst_opcii->IDENTIFIER()->getText();
        teks_opcij.push_back(tekst);
        tseli_opcij.push_back(tsel);
        std::cout << indeks << ") " << tekst << std::endl;
        indeks++;
    }

    std::cout << "\nВаш выбор (1-" << teks_opcij.size() << "): ";
    int vybor = -1;
    if (!(std::cin >> vybor)) {
        // Ошибка потока ввода, прерываем выполнение
        std::cin.clear();
        return true;
    }

    if (vybor >= 1 && vybor <= (int)tseli_opcij.size()) {
        std::cout << std::endl;
        vypolnitVetku(tseli_opcij[vybor - 1]);
    } else {
        std::cerr << "Недопустимый выбор. Завершение диалога." << std::endl;
    }

    return true; // Прерываем выполнение текущей ветки, так как мы сделали переход
}

bool DialogueInterpreter::vychislitUslovie(DialogueScriptParser::ConditionContext* ctx) {
    std::string imya_usloviya = udalitKavychki(ctx->STRING()->getText());
    if (ctx->getText().rfind("has_item", 0) == 0) {
        return mir.estPredmet(imya_usloviya);
    } else if (ctx->getText().rfind("has_flag", 0) == 0) {
        return mir.proveritFlag(imya_usloviya);
    }
    return false;
}

antlrcpp::Any DialogueInterpreter::visitIfStmt(DialogueScriptParser::IfStmtContext* ctx) {
    if (etap_stat_analiza) {
        for (auto* kontekst_operatora : ctx->statement()) {
            visit(kontekst_operatora);
        }
        return {};
    }

    bool uslovie = vychislitUslovie(ctx->condition());

    // Ищем индекс разделителя else
    size_t elseStartIdx = (size_t)-1;
    for (size_t i = 0; i < ctx->children.size(); i++) {
        if (ctx->children[i]->getText() == "else") {
            elseStartIdx = i;
            break;
        }
    }

    // Проходим по каждому оператору и решаем нужно ли его выполнять
    for (auto* kontekst_operatora : ctx->statement()) {
        bool do_else = true;

        for (size_t i = 0; i < ctx->children.size(); i++) {
            if (ctx->children[i] == kontekst_operatora) {
                if (elseStartIdx != (size_t)-1 && i > elseStartIdx) {
                    do_else = false;
                }
                break;
            }
        }

        if (uslovie && do_else) {
            antlrcpp::Any rezultat = visit(kontekst_operatora);
            if (rezultat.type() == typeid(bool) && std::any_cast<bool>(rezultat)) return true;
        } else if (!uslovie && !do_else) {
            antlrcpp::Any rezultat = visit(kontekst_operatora);
            if (rezultat.type() == typeid(bool) && std::any_cast<bool>(rezultat)) return true;
        }
    }

    return false;
}

antlrcpp::Any DialogueInterpreter::visitGotoStmt(DialogueScriptParser::GotoStmtContext* ctx) {
    std::string imya_vetki = ctx->IDENTIFIER()->getText();
    if (etap_stat_analiza) {
        if (!reestr_vetok.sushchestvuetVetka(imya_vetki)) {
            throw std::runtime_error("Семантическая ошибка: неизвестная целевая ветка '" + imya_vetki + "' на строке " + std::to_string(ctx->getStart()->getLine()));
        }
        return {};
    }

    vypolnitVetku(imya_vetki);
    return true; // Прерываем выполнение, чтобы перейти
}

antlrcpp::Any DialogueInterpreter::visitActionStmt(DialogueScriptParser::ActionStmtContext* ctx) {
    if (etap_stat_analiza) return {};

    std::string ves_tekst = ctx->getText();
    std::string argument = udalitKavychki(ctx->STRING()->getText());

    if (ves_tekst.rfind("give_item", 0) == 0) {
        mir.datPredmet(argument);
        std::cout << "(Предмет добавлен в инвентарь: " << argument << ")" << std::endl;
    } else if (ves_tekst.rfind("take_item", 0) == 0) {
        int kolichestvo = std::stoi(ctx->INT()->getText());
        if (!mir.zabratPredmet(argument, kolichestvo)) {
            throw std::runtime_error("Ошибка выполнения: недостаточно предметов '" + argument + "' чтобы забрать");
        }
        std::cout << "(Предмет изъят из инвентаря: " << argument << " x" << kolichestvo << ")" << std::endl;
    } else if (ves_tekst.rfind("set_flag", 0) == 0) {
        bool znachenie = ctx->BOOL()->getText() == "true";
        mir.ustanovitFlag(argument, znachenie);
        std::cout << "(Флаг установлен: " << argument << " = " << (znachenie ? "true" : "false") << ")" << std::endl;
    }

    return false;
}
