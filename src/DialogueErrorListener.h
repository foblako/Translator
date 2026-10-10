#pragma once

#include "antlr4-runtime.h"
#include <iostream>

// Слушатель ошибок для перехвата синтаксических ошибок во время парсинга
class DialogueErrorListener : public antlr4::BaseErrorListener {
public:
    bool est_oshibka = false; // Флаг наличия синтаксических ошибок

    void syntaxError(antlr4::Recognizer *raspoznavatel, antlr4::Token *oshibochnyj_simvol, size_t stroka,
                     size_t poziciya_v_stroke, const std::string &soobshchenie, std::exception_ptr e) override {
        est_oshibka = true;
        std::cerr << "Синтаксическая ошибка на строке " << stroka << ":" << poziciya_v_stroke << " - " << soobshchenie << std::endl;
    }
};
