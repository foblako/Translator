#pragma once

#include <unordered_map>
#include <string>
#include "DialogueScriptParser.h"

class BranchRegistry {
    std::unordered_map<std::string, DialogueScriptParser::BranchDeclContext*> branches;
    std::string startBranch;

public:
    void registerBranch(const std::string& name, DialogueScriptParser::BranchDeclContext* ctx) {
        branches[name] = ctx;
    }

    DialogueScriptParser::BranchDeclContext* getBranch(const std::string& name) const {
        auto it = branches.find(name);
        if (it != branches.end()) {
            return it->second;
        }
        return nullptr;
    }

    void setStartBranch(const std::string& name) {
        startBranch = name;
    }

    const std::string& getStartBranch() const {
        return startBranch;
    }

    bool hasBranch(const std::string& name) const {
        return branches.find(name) != branches.end();
    }
};
