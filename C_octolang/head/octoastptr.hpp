#pragma once

#include "octoast.hpp"

namespace C_octolang {
    class ASTPointer {
    protected:
        ASTTree* obj;
    public:
        ASTPointer(ASTTree* _obj) {
            obj = _obj;
        }
        ASTPointer(ASTTree& _obj) {
            obj = &_obj;
        }
        bool errorcheck() {
            return obj == nullptr;
        }
        ASTPointer& back() {
            obj = obj->parent;
            return *this;
        }
        ASTPointer& pointto(ASTTree* obj) {
            this->obj = obj;
            return *this;
        }
        ASTPointer& front() {
            obj = &*(obj->children.end() - 1);
            return *this;
        }
        ASTPointer& initblank() {
            obj->children.push_back(ASTTree());
            obj->children.back().parent = obj;
            return front();
        }
        ASTPointer& at(uint32_t ind) {
            auto& x = obj->children;
            obj = &x[x.size() - ind];
            return *this;
        }
        ASTTree& index(uint32_t ind) {
            auto& x = obj->children;
            return x[x.size() - ind];
        }
        ASTPointer& operator[](uint32_t ind) {
            return at(ind);
        }
        operator ASTTree& () {
            return *obj;
        }
        operator ASTTree* () {
            return obj;
        }
        ASTTree& operator*() {
            return *this;
        }
        ASTTree* operator&() {
            return this;
        }
        ASTTree* operator->() {
            return obj;
        }
    };
}