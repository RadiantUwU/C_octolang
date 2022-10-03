#pragma once

#include <vector>
#include <stack>

#include "generator.hpp"
#include "elseif.hpp"
#include "octolex.hpp"

namespace octo {
    enum class Parse_Inst_t {
        Error,
        Class_Def,
        Funcion_Def,
        Identifier,
        Operator,
        Begin_Expr,
        End_Expr,
        End,
        Number,
        Integer,
        String,
        Attribute
    };
    struct Parse_Inst {
        Parse_Inst_t ty;
        union {
            std::string s;
            int64_t i;
            double d;
        };
        Parse_Inst(Parse_Inst_t t, std::string s) noexcept : ty(t), s(s) {}
        Parse_Inst(Parse_Inst_t t, int64_t n) noexcept : ty(t), i(n) {}
        Parse_Inst(Parse_Inst_t t, double d) noexcept : ty(t), d(d) {}
        Parse_Inst(Parse_Inst_t t) noexcept : ty(t) {}
        ~Parse_Inst() noexcept {
            switch (ty) {
                case Parse_Inst_t::Identifier:
                case Parse_Inst_t::Operator:
                case Parse_Inst_t::String:
                case Parse_Inst_t::Attribute:
                    s.~basic_string();
            }
        }
        static Parse_Inst error() {
            return Parse_Inst(Parse_Inst_t::Error);
        }
    }
    class Parser {
    protected:
        std::stack<Parse_Inst,std::vector<Parse_Inst>> insts;
        Closeable_Generator_Ptr<token> lexer;
    public:
        Parser(closeable_generator<token> auto g) {
            lexer = to_cgen_ptr(g);
        }
        Parse_Inst next() {

        }
        bool done() {}
        void close() {}
    };
}