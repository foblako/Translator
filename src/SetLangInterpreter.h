#pragma once

#include "SetLangBaseVisitor.h"
#include <iostream>
#include <variant>
#include <set>
#include <string>
#include <map>
#include <stdexcept>
#include <algorithm>

// The value type that variables and expressions can evaluate to
using Value = std::variant<int, bool, std::set<int>>;

class SetLangInterpreter : public SetLangBaseVisitor {
private:
    std::map<std::string, Value> variables;

    // Helpers to unwrap values
    int asInt(const Value& v) {
        if (std::holds_alternative<int>(v)) return std::get<int>(v);
        throw std::runtime_error("Type error: Expected int");
    }

    bool asBool(const Value& v) {
        if (std::holds_alternative<bool>(v)) return std::get<bool>(v);
        throw std::runtime_error("Type error: Expected bool");
    }

    std::set<int> asSet(const Value& v) {
        if (std::holds_alternative<std::set<int>>(v)) return std::get<std::set<int>>(v);
        throw std::runtime_error("Type error: Expected set");
    }

public:
    std::any visitAssignStmt(SetLangParser::AssignStmtContext *ctx) override {
        std::string varName = ctx->assignment()->ID()->getText();
        Value val = std::any_cast<Value>(visit(ctx->assignment()->expr()));
        variables[varName] = val;
        return {};
    }

    std::any visitPrintStmt(SetLangParser::PrintStmtContext *ctx) override {
        Value val = std::any_cast<Value>(visit(ctx->expr()));
        if (std::holds_alternative<int>(val)) {
            std::cout << std::get<int>(val) << std::endl;
        } else if (std::holds_alternative<bool>(val)) {
            std::cout << (std::get<bool>(val) ? "true" : "false") << std::endl;
        } else if (std::holds_alternative<std::set<int>>(val)) {
            const auto& s = std::get<std::set<int>>(val);
            std::cout << "{";
            bool first = true;
            for (int v : s) {
                if (!first) std::cout << ", ";
                std::cout << v;
                first = false;
            }
            std::cout << "}" << std::endl;
        }
        return {};
    }

    std::any visitIntExpr(SetLangParser::IntExprContext *ctx) override {
        return Value(std::stoi(ctx->INT()->getText()));
    }

    std::any visitIdExpr(SetLangParser::IdExprContext *ctx) override {
        std::string varName = ctx->ID()->getText();
        if (variables.find(varName) == variables.end()) {
            throw std::runtime_error("Undefined variable: " + varName);
        }
        return variables[varName];
    }

    std::any visitParensExpr(SetLangParser::ParensExprContext *ctx) override {
        return visit(ctx->expr());
    }

    std::any visitSetExpr(SetLangParser::SetExprContext *ctx) override {
        std::set<int> result;
        for (auto* exprCtx : ctx->expr()) {
            Value val = std::any_cast<Value>(visit(exprCtx));
            result.insert(asInt(val));
        }
        return Value(result);
    }

    std::any visitMulDivExpr(SetLangParser::MulDivExprContext *ctx) override {
        Value left = std::any_cast<Value>(visit(ctx->expr(0)));
        Value right = std::any_cast<Value>(visit(ctx->expr(1)));

        // Multiplications and divisions are only on integers or sets.
        // Let's implement set intersection for `*` if both are sets.
        if (std::holds_alternative<std::set<int>>(left) && std::holds_alternative<std::set<int>>(right)) {
            if (ctx->children[1]->getText() == "*") {
                std::set<int> l = asSet(left);
                std::set<int> r = asSet(right);
                std::set<int> result;
                std::set_intersection(l.begin(), l.end(), r.begin(), r.end(), std::inserter(result, result.begin()));
                return Value(result);
            } else {
                throw std::runtime_error("Type error: Cannot divide sets");
            }
        }

        int l = asInt(left);
        int r = asInt(right);
        if (ctx->children[1]->getText() == "*") {
            return Value(l * r);
        } else {
            if (r == 0) throw std::runtime_error("Division by zero");
            return Value(l / r);
        }
    }

    std::any visitAddSubExpr(SetLangParser::AddSubExprContext *ctx) override {
        Value left = std::any_cast<Value>(visit(ctx->expr(0)));
        Value right = std::any_cast<Value>(visit(ctx->expr(1)));

        // Let's implement set union for `+` and set difference for `-` if both are sets.
        if (std::holds_alternative<std::set<int>>(left) && std::holds_alternative<std::set<int>>(right)) {
            std::set<int> l = asSet(left);
            std::set<int> r = asSet(right);
            std::set<int> result;
            if (ctx->children[1]->getText() == "+") {
                std::set_union(l.begin(), l.end(), r.begin(), r.end(), std::inserter(result, result.begin()));
            } else {
                std::set_difference(l.begin(), l.end(), r.begin(), r.end(), std::inserter(result, result.begin()));
            }
            return Value(result);
        }

        int l = asInt(left);
        int r = asInt(right);
        if (ctx->children[1]->getText() == "+") {
            return Value(l + r);
        } else {
            return Value(l - r);
        }
    }

    std::any visitEqNeqExpr(SetLangParser::EqNeqExprContext *ctx) override {
        Value left = std::any_cast<Value>(visit(ctx->expr(0)));
        Value right = std::any_cast<Value>(visit(ctx->expr(1)));

        bool isEqual = false;
        if (left.index() != right.index()) {
            isEqual = false;
        } else {
            if (std::holds_alternative<int>(left)) {
                isEqual = (std::get<int>(left) == std::get<int>(right));
            } else if (std::holds_alternative<bool>(left)) {
                isEqual = (std::get<bool>(left) == std::get<bool>(right));
            } else if (std::holds_alternative<std::set<int>>(left)) {
                isEqual = (std::get<std::set<int>>(left) == std::get<std::set<int>>(right));
            }
        }

        if (ctx->children[1]->getText() == "==") {
            return Value(isEqual);
        } else {
            return Value(!isEqual);
        }
    }

    std::any visitIfStmt(SetLangParser::IfStmtContext *ctx) override {
        Value cond = std::any_cast<Value>(visit(ctx->expr()));
        if (asBool(cond)) {
            visit(ctx->block(0));
        } else if (ctx->block().size() > 1) {
            visit(ctx->block(1));
        }
        return {};
    }

    std::any visitWhileStmt(SetLangParser::WhileStmtContext *ctx) override {
        while (true) {
            Value cond = std::any_cast<Value>(visit(ctx->expr()));
            if (!asBool(cond)) break;
            visit(ctx->block());
        }
        return {};
    }

    std::any visitBlock(SetLangParser::BlockContext *ctx) override {
        for (auto* stmtCtx : ctx->statement()) {
            visit(stmtCtx);
        }
        return {};
    }
};
