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
    // Проверяем, что передан путь к файлу
    if (argc < 2) {
        std::cerr << "Использование: " << argv[0] << " <путь_к_файлу>" << std::endl;
        return 1;
    }

    std::ifstream fileStream(argv[1]);
    if (!fileStream.is_open()) {
        std::cerr << "Не удалось открыть файл: " << argv[1] << std::endl;
        return 1;
    }

    // Настраиваем ANTLR лексер
    antlr4::ANTLRInputStream input(fileStream);
    DialogueScriptLexer lexer(&input);

    DialogueErrorListener errorListener;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&errorListener);

    // Настраиваем ANTLR парсер
    antlr4::CommonTokenStream tokens(&lexer);
    DialogueScriptParser parser(&tokens);

    parser.removeErrorListeners();
    parser.addErrorListener(&errorListener);

    // Строим синтаксическое дерево
    DialogueScriptParser::ProgramContext* tree = parser.program();

    // Проверяем наличие синтаксических ошибок
    if (errorListener.hasError) {
        std::cerr << "Синтаксический анализ завершился с ошибками." << std::endl;
        return 1;
    }

    // Инициализируем состояние мира и запускаем интерпретатор
    WorldState world;
    BranchRegistry registry;
    DialogueInterpreter interpreter(world, registry);

    try {
        interpreter.visit(tree);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1; // Завершаемся с ошибкой при семантических сбоях (ошибка времени выполнения)
    }

    return 0; // Успешное выполнение
}
