# Установка ANTLR4 для C++

Для сборки проекта требуются следующие зависимости:

- Компилятор с поддержкой C++17 (GCC, Clang, или MSVC).
- CMake (версии 3.16 и выше).
- Java JRE (версии 11 и выше) для генерации кода ANTLR из грамматики.

## Linux (Ubuntu / Debian)

```bash
sudo apt update
sudo apt install -y build-essential cmake default-jre
```

## macOS

```bash
brew install cmake openjdk
```

## Windows

- Установите CMake с официального сайта.
- Установите Java (например, скачав OpenJDK).
- Убедитесь, что пути к `cmake` и `java` добавлены в переменную среды PATH.

Проект использует CMake `FetchContent` для автоматического скачивания C++ Runtime библиотеки ANTLR4 при первой сборке, поэтому устанавливать её вручную не требуется.
Также автоматически скачивается Java-утилита генератора, благодаря чему вам не нужно ничего делать кроме установки Java.

## Установка инструмента для отладки

Для отладки вы можете использовать `antlr4-parse`:

```bash
pip install antlr4-tools
```
