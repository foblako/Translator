#include <iostream>
#include <fstream>
#include "antlr4-runtime.h"
#include "SetLangLexer.h"
#include "SetLangParser.h"
#include "SetLangInterpreter.h"
#include "SetLangErrorListener.h"

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }

    std::ifstream stream(argv[1]);
    if (!stream.is_open()) {
        std::cerr << "Could not open file: " << argv[1] << std::endl;
        return 1;
    }

    antlr4::ANTLRInputStream input(stream);
    SetLangLexer lexer(&input);

    SetLangErrorListener errorListener;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&errorListener);

    antlr4::CommonTokenStream tokens(&lexer);
    SetLangParser parser(&tokens);

    parser.removeErrorListeners();
    parser.addErrorListener(&errorListener);

    SetLangParser::ProgramContext* tree = parser.program();

    if (errorListener.hasError) {
        std::cerr << "Parsing failed due to syntax errors." << std::endl;
        return 1;
    }

    SetLangInterpreter interpreter;
    try {
        interpreter.visit(tree);
    } catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
