#pragma once

#include <unordered_map>
#include <string>

class WorldState {
    std::unordered_map<std::string, int> inventory;
    std::unordered_map<std::string, bool> flags;

public:
    void giveItem(const std::string& item) {
        inventory[item]++;
    }

    bool takeItem(const std::string& item, int count) {
        if (inventory[item] >= count) {
            inventory[item] -= count;
            return true;
        }
        return false;
    }

    bool hasItem(const std::string& item) const {
        auto it = inventory.find(item);
        if (it != inventory.end()) {
            return it->second > 0;
        }
        return false;
    }

    void setFlag(const std::string& flag, bool value) {
        flags[flag] = value;
    }

    bool hasFlag(const std::string& flag) const {
        auto it = flags.find(flag);
        if (it != flags.end()) {
            return it->second;
        }
        return false;
    }
};
