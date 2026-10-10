#pragma once

#include <unordered_map>
#include <string>

// Класс для хранения состояния мира: инвентарь игрока и глобальные флаги
class WorldState {
    std::unordered_map<std::string, int> inventar_igroka; // Инвентарь: название предмета -> количество
    std::unordered_map<std::string, bool> global_flagi;   // Флаги: название флага -> значение

public:
    // Добавить предмет в инвентарь
    void datPredmet(const std::string& nazvanie_predmeta) {
        inventar_igroka[nazvanie_predmeta]++;
    }

    // Забрать предмет из инвентаря
    bool zabratPredmet(const std::string& nazvanie_predmeta, int kolichestvo) {
        if (inventar_igroka[nazvanie_predmeta] >= kolichestvo) {
            inventar_igroka[nazvanie_predmeta] -= kolichestvo;
            return true;
        }
        return false; // Недостаточно предметов
    }

    // Проверить наличие предмета
    bool estPredmet(const std::string& nazvanie_predmeta) const {
        auto iterator = inventar_igroka.find(nazvanie_predmeta);
        if (iterator != inventar_igroka.end()) {
            return iterator->second > 0;
        }
        return false;
    }

    // Установить значение флага
    void ustanovitFlag(const std::string& nazvanie_flaga, bool znachenie) {
        global_flagi[nazvanie_flaga] = znachenie;
    }

    // Проверить значение флага
    bool proveritFlag(const std::string& nazvanie_flaga) const {
        auto iterator = global_flagi.find(nazvanie_flaga);
        if (iterator != global_flagi.end()) {
            return iterator->second;
        }
        return false;
    }
};
