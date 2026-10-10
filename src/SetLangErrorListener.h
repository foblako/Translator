#pragma once

#include "antlr4-runtime.h"
#include <iostream>

class SetLangErrorListener : public antlr4::BaseErrorListener {
public:
    bool hasError = false;

    void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                     size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override {
        hasError = true;
        std::cerr << "Syntax error at line " << line << ":" << charPositionInLine << " - " << msg << std::endl;
    }
};
