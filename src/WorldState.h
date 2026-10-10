#pragma once

#include <unordered_map>
#include <string>

// Класс для хранения состояния мира: инвентарь игрока и глобальные флаги
class WorldState {
    std::unordered_map<std::string, int> inventory; // Инвентарь: название предмета -> количество
    std::unordered_map<std::string, bool> flags;    // Флаги: название флага -> значение

public:
    // Добавить предмет в инвентарь
    void giveItem(const std::string& itemName) {
        inventory[itemName]++;
    }

    // Забрать предмет из инвентаря
    bool takeItem(const std::string& itemName, int count) {
        if (inventory[itemName] >= count) {
            inventory[itemName] -= count;
            return true;
        }
        return false; // Недостаточно предметов
    }

    // Проверить наличие предмета
    bool hasItem(const std::string& itemName) const {
        auto it = inventory.find(itemName);
        if (it != inventory.end()) {
            return it->second > 0;
        }
        return false;
    }

    // Установить значение флага
    void setFlag(const std::string& flagName, bool value) {
        flags[flagName] = value;
    }

    // Проверить значение флага
    bool hasFlag(const std::string& flagName) const {
        auto it = flags.find(flagName);
        if (it != flags.end()) {
            return it->second;
        }
        return false;
    }
};
