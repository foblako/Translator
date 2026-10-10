#pragma once

#include <unordered_map>
#include <string>
#include "DialogueScriptParser.h"

// Реестр всех веток диалога для быстрых переходов
class BranchRegistry {
    // Хранилище веток: имя ветки -> указатель на узел в дереве разбора
    std::unordered_map<std::string, DialogueScriptParser::BranchDeclContext*> branches;
    std::string startBranch; // Имя стартовой ветки

public:
    // Зарегистрировать новую ветку
    void registerBranch(const std::string& branchName, DialogueScriptParser::BranchDeclContext* ctx) {
        branches[branchName] = ctx;
    }

    // Получить ветку по имени
    DialogueScriptParser::BranchDeclContext* getBranch(const std::string& branchName) const {
        auto it = branches.find(branchName);
        if (it != branches.end()) {
            return it->second;
        }
        return nullptr;
    }

    // Установить стартовую ветку
    void setStartBranch(const std::string& branchName) {
        startBranch = branchName;
    }

    // Получить имя стартовой ветки
    const std::string& getStartBranch() const {
        return startBranch;
    }

    // Проверить существование ветки
    bool hasBranch(const std::string& branchName) const {
        return branches.find(branchName) != branches.end();
    }
};
