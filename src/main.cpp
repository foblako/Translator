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

    std::ifstream potok_fajla(argv[1]);
    if (!potok_fajla.is_open()) {
        std::cerr << "Не удалось открыть файл: " << argv[1] << std::endl;
        return 1;
    }

    // Настраиваем ANTLR лексер
    antlr4::ANTLRInputStream vhodnoj_potok(potok_fajla);
    DialogueScriptLexer lekser(&vhodnoj_potok);

    DialogueErrorListener slushatel_oshibok;
    lekser.removeErrorListeners();
    lekser.addErrorListener(&slushatel_oshibok);

    // Настраиваем ANTLR парсер
    antlr4::CommonTokenStream potok_tokenov(&lekser);
    DialogueScriptParser parser(&potok_tokenov);

    parser.removeErrorListeners();
    parser.addErrorListener(&slushatel_oshibok);

    // Строим синтаксическое дерево
    DialogueScriptParser::ProgramContext* derevoRazbora = parser.program();

    // Проверяем наличие синтаксических ошибок
    if (slushatel_oshibok.est_oshibka) {
        std::cerr << "Синтаксический анализ завершился с ошибками." << std::endl;
        return 1;
    }

    // Инициализируем состояние мира и запускаем интерпретатор
    WorldState mir;
    BranchRegistry reestr_vetok;
    DialogueInterpreter interpretator(mir, reestr_vetok);

    try {
        interpretator.visit(derevoRazbora);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1; // Завершаемся с ошибкой при семантических сбоях (ошибка времени выполнения)
    }

    return 0; // Успешное выполнение
}
