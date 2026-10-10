#include <iostream>
#include <fstream>
#include "antlr4-runtime.h"
#include "DialogueScriptLexer.h"
#include "DialogueScriptParser.h"
#include "DialogueInterpreter.h"
#include "DialogueErrorListener.h"
#include "WorldState.h"
#include "BranchRegistry.h"

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
    DialogueScriptLexer lexer(&input);

    DialogueErrorListener errorListener;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&errorListener);

    antlr4::CommonTokenStream tokens(&lexer);
    DialogueScriptParser parser(&tokens);

    parser.removeErrorListeners();
    parser.addErrorListener(&errorListener);

    DialogueScriptParser::ProgramContext* tree = parser.program();

    if (errorListener.hasError) {
        std::cerr << "Parsing failed due to syntax errors." << std::endl;
        return 1;
    }

    WorldState world;
    BranchRegistry registry;
    DialogueInterpreter interpreter(world, registry);

    try {
        interpreter.visit(tree);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
