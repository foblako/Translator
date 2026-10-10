#pragma once

#include <unordered_map>
#include <string>
#include "DialogueScriptParser.h"

// Реестр всех веток диалога для быстрых переходов
class BranchRegistry {
    // Хранилище веток: имя ветки -> указатель на узел в дереве разбора
    std::unordered_map<std::string, DialogueScriptParser::BranchDeclContext*> vse_vetki;
    std::string startovaya_vetka; // Имя стартовой ветки

public:
    // Зарегистрировать новую ветку
    void zaregistrirovatVetku(const std::string& imya_vetki, DialogueScriptParser::BranchDeclContext* kontekst) {
        vse_vetki[imya_vetki] = kontekst;
    }

    // Получить ветку по имени
    DialogueScriptParser::BranchDeclContext* poluchitVetku(const std::string& imya_vetki) const {
        auto iterator = vse_vetki.find(imya_vetki);
        if (iterator != vse_vetki.end()) {
            return iterator->second;
        }
        return nullptr;
    }

    // Установить стартовую ветку
    void ustanovitStart(const std::string& imya_vetki) {
        startovaya_vetka = imya_vetki;
    }

    // Получить имя стартовой ветки
    const std::string& poluchitStart() const {
        return startovaya_vetka;
    }

    // Проверить существование ветки
    bool sushchestvuetVetka(const std::string& imya_vetki) const {
        return vse_vetki.find(imya_vetki) != vse_vetki.end();
    }
};
