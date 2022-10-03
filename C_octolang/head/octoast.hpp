#pragma once

#include <string>
#include <vector>

namespace C_octolang {
    enum class AST_t {
        Classdef,
        Funcdef,
        Vardef,
        Vardecl,
        Keyword
    };

    class ASTTree {
    public:
        ASTTree* parent;
        std::vector<ASTTree> children;
        std::vector<std::string> attrs;
        std::string token;
        AST_t type;
        ASTTree* getRoot() {
            ASTTree* i = this;
            while (i->parent != nullptr) {
                i = i->parent;
            }
            return i;
        }
    };
}