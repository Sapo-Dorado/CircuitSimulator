#pragma once

#include <string>
#include <vector>
#include <memory>

#include "operations.h"

struct Wire {
    std::string name;
    long long committed_value {0};
    std::shared_ptr<ExprNode> comb_expr; // instantaneous (combinational) definition
    std::shared_ptr<ExprNode> next_expr; // next-cycle (registered) definition

    static std::vector<Wire*>& registry() {
        static std::vector<Wire*> r; return r;
    }

    explicit Wire(std::string name_, long long init = 0): name(std::move(name_)), committed_value(init) {
        registry().push_back(this);
    }

    operator Expr() const { return Expr::wireRef(this); }

    // Instantaneous assignment (=)
    Wire& operator=(const Expr& rhs) { comb_expr = rhs.node; return *this; }
    Wire& operator=(long long rhs) { comb_expr = Expr(rhs).node; return *this; }

    // Next-cycle assignment (<<)
    Wire& operator<<(const Expr& rhs) { next_expr = rhs.node; return *this; }
    Wire& operator<<(long long rhs) { next_expr = Expr(rhs).node; return *this; }
};


