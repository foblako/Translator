#pragma once

#include "antlr4-runtime.h"
#include <iostream>

// Слушатель ошибок для перехвата синтаксических ошибок во время парсинга
class DialogueErrorListener : public antlr4::BaseErrorListener {
public:
    bool hasError = false; // Флаг наличия синтаксических ошибок

    void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                     size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override {
        hasError = true;
        std::cerr << "Синтаксическая ошибка на строке " << line << ":" << charPositionInLine << " - " << msg << std::endl;
    }
};
