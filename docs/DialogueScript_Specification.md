# DialogueScript — Язык сценариев для диалогов

## 1. Назначение языка

**DialogueScript (DS)** — предметно-ориентированный язык (DSL) для описания нелинейных диалогов в компьютерных играх (RPG, квесты, визуальные новеллы).

Язык поддерживает:
- Реплики NPC с указанием имени говорящего
- Выбор игрока из нескольких вариантов (диалоговое дерево)
- Условные переходы на основе состояния мира (наличие предметов, флаги)
- Действия: выдача/изъятие предметов, установка флагов
- Переходы между ветками диалога

---

## 2. Синтаксис языка

### 2.1. Структура программы

Программа состоит из:
- **Объявления стартовой точки** (обязательно, ровно одна)
- **Объявления веток диалога** (одна или более)

```
start <имя_ветки>

<имя_ветки_1> {
    <операторы>
}

<имя_ветки_2> {
    <операторы>
}
```

### 2.2. Операторы

#### Реплика NPC
```
npc "<имя>": "<текст>"
```

**Пример:**
```
npc "Старик": "Приветствую тебя, путник!"
npc "Страж": "Стой! Кто идет?"
```

#### Выбор игрока
```
choice "<вопрос>" {
    option "<текст_варианта_1>" -> <имя_ветки_1>
    option "<текст_варианта_2>" -> <имя_ветки_2>
    ...
}
```

**Пример:**
```
choice "Что ты хочешь?" {
    option "Расскажи о лесах" -> forest_info
    option "Дать монету" -> give_coin
    option "Уйти" -> goodbye
}
```

#### Условный оператор
```
if <условие> {
    <операторы>
} else {
    <операторы>
}
```

**Пример:**
```
if has_item("key") {
    npc "Старик": "О, у тебя есть ключ!"
    goto temple
} else {
    npc "Старик": "Тебе нужен ключ."
}
```

#### Условие
```
has_item("<имя_предмета>")
has_flag("<имя_флага>")
```

#### Переход
```
goto <имя_ветки>
```

**Пример:**
```
goto forest_info
```

#### Действия
```
give_item("<имя_предмета>")
take_item("<имя_предмета>", <количество>)
set_flag("<имя_флага>", <true|false>)
```

**Примеры:**
```
give_item("magic_scroll")
take_item("gold_coin", 1)
set_flag("quest_started", true)
```

### 2.3. Комментарии

```
// Однострочный комментарий

/*
Многострочный
комментарий
*/
```

---

## 3. Семантика языка

### 3.1. Статический анализ (проверки при загрузке)

Транслятор выполняет следующие проверки:

1. **Наличие стартовой точки**
   - Должна быть ровно одна конструкция `start <ветка>`
   - Указанная ветка должна существовать

2. **Существование веток**
   - Все ветки, на которые ссылаются `goto` и `option`, должны быть определены
   - Имена веток должны быть уникальными

3. **Корректность действий**
   - `take_item` — количество должно быть положительным числом
   - `set_flag` — значение должно быть `true` или `false`

### 3.2. Динамическая интерпретация (выполнение)

#### Состояние мира (World State)
Интерпретатор хранит:
- **Инвентарь игрока**: `std::unordered_map<std::string, int>` (название предмета → количество)
- **Флаги мира**: `std::unordered_map<std::string, bool>` (название флага → значение)

#### Выполнение команд

| Команда | Действие |
|---------|----------|
| `npc "Name": "Text"` | Выводит в консоль: `[Name]: Text` |
| `choice ... { ... }` | Выводит варианты, ожидает ввод пользователя (номер варианта), переходит к выбранной ветке |
| `if has_item("item")` | Проверяет наличие предмета в инвентаре |
| `if has_flag("flag")` | Проверяет значение флага |
| `give_item("item")` | Добавляет предмет в инвентарь (количество +1) |
| `take_item("item", count)` | Убирает предмет из инвентаря (если недостаточно — ошибка выполнения) |
| `set_flag("flag", value)` | Устанавливает значение флага |
| `goto branch` | Переходит к выполнению указанной ветки |

#### Ошибки выполнения
- Попытка взять предмет, которого нет в инвентаре
- Попытка взять больше предметов, чем есть

---

## 4. Грамматика ANTLR4

```antlr
grammar DialogueScript;

// ===== Парсер (синтаксис) =====
program
    : startDecl branchDecl+ EOF
    ;

startDecl
    : 'start' IDENTIFIER
    ;

branchDecl
    : IDENTIFIER '{' statement* '}'
    ;

statement
    : npcStmt
    | choiceStmt
    | ifStmt
    | gotoStmt
    | actionStmt
    ;

npcStmt
    : 'npc' STRING ':' STRING
    ;

choiceStmt
    : 'choice' STRING '{' option+ '}'
    ;

option
    : 'option' STRING '->' IDENTIFIER
    ;

ifStmt
    : 'if' condition '{' statement* '}' ('else' '{' statement* '}')?
    ;

condition
    : 'has_item' '(' STRING ')'
    | 'has_flag' '(' STRING ')'
    ;

gotoStmt
    : 'goto' IDENTIFIER
    ;

actionStmt
    : 'give_item' '(' STRING ')'
    | 'take_item' '(' STRING ',' INT ')'
    | 'set_flag' '(' STRING ',' BOOL ')'
    ;

// ===== Лексер (токены) =====
IDENTIFIER : [a-zA-Z_][a-zA-Z_0-9]* ;
STRING     : '"' (~["\r\n])* '"' ;
INT        : [0-9]+ ;
BOOL       : 'true' | 'false' ;

WS         : [ \t\r\n]+ -> skip ;
LINE_COMMENT : '//' ~[\r\n]* -> skip ;
BLOCK_COMMENT : '/*' .*? '*/' -> skip ;
```

---

## 5. Примеры программ

### 5.1. Корректная программа (examples/valid/quest_intro.txt)

```text
// Начало квеста "Знакомство со стражем"
start intro

intro {
    npc "Страж": "Стой! Кто идет?"
    choice "Ответить" {
        option "Я странник" -> traveler
        option "Враг!" -> enemy
    }
}

traveler {
    npc "Страж": "Добро пожаловать в наши земли."
    npc "Страж": "Вот карта для тебя."
    give_item("map")
    npc "Страж": "Удачи в пути!"
}

enemy {
    npc "Страж": "Тогда готовься к бою!"
    set_flag("combat_started", true)
}
```

### 5.2. Корректная программа с условиями (examples/valid/shop_dialogue.txt)

```text
start shop

shop {
    npc "Торговец": "Добро пожаловать в мою лавку!"

    if has_item("gold_coin") {
        npc "Торговец": "Вижу, у тебя есть монеты. Хочешь купить свиток?"
        choice "Купить?" {
            option "Да (5 монет)" -> buy_scroll
            option "Нет" -> leave
        }
    } else {
        npc "Торговец": "У тебя нет монет. Приходи позже."
    }
}

buy_scroll {
    take_item("gold_coin", 5)
    give_item("magic_scroll")
    npc "Торговец": "Вот твой свиток. Удачной охоты!"
}

leave {
    npc "Торговец": "Ну, как знаешь. Заходи еще!"
}
```

### 5.3. Некорректная программа — нет стартовой точки (examples/invalid/missing_start.txt)

```text
// Ошибка: отсутствует конструкция 'start'
branch_1 {
    npc "Старик": "Привет!"
}
```

**Ожидаемая ошибка:** `Semantic error: missing 'start' declaration`

### 5.4. Некорректная программа — переход в несуществующую ветку (examples/invalid/unknown_branch.txt)

```text
start intro

intro {
    npc "Старик": "Привет!"
    goto unknown_branch  // Ошибка: ветка 'unknown_branch' не определена
}
```

**Ожидаемая ошибка:** `Semantic error: undefined branch 'unknown_branch' at line 5`

### 5.5. Некорректная программа — синтаксическая ошибка (examples/invalid/syntax_error.txt)

```text
start intro

intro {
    npc "Старик" "Привет!"  // Ошибка: пропущено двоеточие
}
```

**Ожидаемая ошибка:** `Syntax error at line 4: mismatched input '"' expecting ':'`

### 5.6. Некорректная программа — дублирование ветки (examples/invalid/duplicate_branch.txt)

```text
start branch_1

branch_1 {
    npc "Старик": "Первый раз"
}

branch_1 {  // Ошибка: ветка 'branch_1' уже определена
    npc "Старик": "Второй раз"
}
```

**Ожидаемая ошибка:** `Semantic error: duplicate branch name 'branch_1' at line 7`

---

## 6. Архитектура проекта

### 6.1. Структура файлов

```
.
├── CMakeLists.txt
├── grammar/
│   └── DialogueScript.g4          # Грамматика ANTLR4
├── src/
│   ├── main.cpp                   # Точка входа
│   ├── DialogueInterpreter.h      # Visitor-интерпретатор (заголовочный файл)
│   ├── DialogueInterpreter.cpp    # Реализация Visitor
│   ├── WorldState.h               # Состояние мира (инвентарь, флаги)
│   └── BranchRegistry.h           # Реестр всех веток диалога
├── examples/
│   ├── valid/                     # Корректные программы
│   │   ├── quest_intro.txt
│   │   └── shop_dialogue.txt
│   └── invalid/                   # Некорректные программы
│       ├── missing_start.txt
│       ├── unknown_branch.txt
│       ├── syntax_error.txt
│       └── duplicate_branch.txt
└── docs/
    └── DialogueScript_Specification.md  # Этот документ
```

### 6.2. Ключевые классы

#### WorldState
Хранит состояние мира (инвентарь игрока и флаги).

```cpp
class WorldState {
    std::unordered_map<std::string, int> inventory;
    std::unordered_map<std::string, bool> flags;

public:
    void giveItem(const std::string& item);
    bool takeItem(const std::string& item, int count);
    bool hasItem(const std::string& item) const;
    void setFlag(const std::string& flag, bool value);
    bool hasFlag(const std::string& flag) const;
};
```

#### BranchRegistry
Хранит все ветки диалога (AST) и стартовую точку.

```cpp
class BranchRegistry {
    std::unordered_map<std::string, DialogueScriptParser::BranchDeclContext*> branches;
    std::string startBranch;

public:
    void registerBranch(const std::string& name, DialogueScriptParser::BranchDeclContext* ctx);
    DialogueScriptParser::BranchDeclContext* getBranch(const std::string& name) const;
    void setStartBranch(const std::string& name);
    const std::string& getStartBranch() const;
};
```

#### DialogueInterpreter
Visitor, который обходит дерево разбора и выполняет команды.

```cpp
class DialogueInterpreter : public DialogueScriptBaseVisitor {
    WorldState& world;
    BranchRegistry& registry;

public:
    DialogueInterpreter(WorldState& world, BranchRegistry& registry);

    antlrcpp::Any visitProgram(DialogueScriptParser::ProgramContext* ctx) override;
    antlrcpp::Any visitStartDecl(DialogueScriptParser::StartDeclContext* ctx) override;
    antlrcpp::Any visitBranchDecl(DialogueScriptParser::BranchDeclContext* ctx) override;
    antlrcpp::Any visitNpcStmt(DialogueScriptParser::NpcStmtContext* ctx) override;
    antlrcpp::Any visitChoiceStmt(DialogueScriptParser::ChoiceStmtContext* ctx) override;
    antlrcpp::Any visitIfStmt(DialogueScriptParser::IfStmtContext* ctx) override;
    antlrcpp::Any visitGotoStmt(DialogueScriptParser::GotoStmtContext* ctx) override;
    antlrcpp::Any visitActionStmt(DialogueScriptParser::ActionStmtContext* ctx) override;

private:
    void executeBranch(const std::string& branchName);
    bool evaluateCondition(DialogueScriptParser::ConditionContext* ctx);
};
```

### 6.3. Алгоритм работы

1. **Загрузка и парсинг**
   - Чтение файла с программой
   - Лексический и синтаксический анализ (ANTLR4)
   - Построение дерева разбора (AST)

2. **Статический анализ**
   - Обход дерева с помощью Visitor
   - Регистрация всех веток в `BranchRegistry`
   - Проверка существования стартовой точки
   - Проверка ссылок на ветки в `goto` и `option`

3. **Интерпретация**
   - Запуск выполнения со стартовой ветки
   - Обход дерева и выполнение команд
   - Взаимодействие с пользователем (вывод реплик, ожидание выбора)
   - Обновление состояния мира (инвентарь, флаги)

4. **Завершение**
   - При достижении конца ветки без `goto` — завершение диалога
   - Возврат кода 0 при успешном выполнении
   - Возврат ненулевого кода при ошибке

---

## 7. Пример использования

### 7.1. Компиляция

```bash
cmake -B build
cmake --build build
```

### 7.2. Запуск

```bash
./build/main examples/valid/quest_intro.txt
```

### 7.3. Пример вывода

```
[Страж]: Стой! Кто идет?

1) Я странник
2) Враг!

Ваш выбор (1-2): 1

[Страж]: Добро пожаловать в наши земли.
[Страж]: Вот карта для тебя.
(Предмет добавлен в инвентарь: map)
[Страж]: Удачи в пути!

Диалог завершен.
```

---

## 8. Технические требования

### 8.1. Инструменты разработки
- **Язык реализации**: C++17
- **Генератор парсеров**: ANTLR4
- **Система сборки**: CMake 3.14+
- **Компилятор**: GCC 7+ / Clang 5+ / MSVC 2017+

### 8.2. Требования к транслятору
- Принимает путь к файлу с программой в качестве аргумента командной строки
- При успешном разборе и выполнении — завершается с кодом возврата 0
- При ошибке (синтаксической или семантической) — выводит сообщение об ошибке в `stderr` и завершается с ненулевым кодом возврата

### 8.3. Обработка ошибок
- **Синтаксические ошибки**: обнаруживаются ANTLR4, выводятся с указанием строки и колонки
- **Семантические ошибки**: обнаруживаются Visitor (отсутствие ветки, дублирование имен и т.д.)
- **Ошибки выполнения**: обнаруживаются во время интерпретации (недостаточно предметов и т.д.)
