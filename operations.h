#pragma once

#include <memory>

struct Wire; // forward declaration

// Expression node types
enum class OpType {
    Constant,
    WireRef,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    BitAnd,
    BitOr,
    BitXor,
    BitNot,
    Neg,
    Shl,
    Shr,
    LogAnd,
    LogOr,
    LogNot,
    Eq,
    Ne,
    Lt,
    Le,
    Gt,
    Ge,
    Select // condition ? thenExpr : elseExpr
};

struct ExprNode {
    OpType op {OpType::Constant};
    long long constant_value {0};
    const Wire* wire {nullptr};
    std::shared_ptr<ExprNode> a;
    std::shared_ptr<ExprNode> b;
    std::shared_ptr<ExprNode> c;
};

struct Expr {
    std::shared_ptr<ExprNode> node;

    Expr() = default;
    explicit Expr(long long v) { node = std::make_shared<ExprNode>(); node->op = OpType::Constant; node->constant_value = v; }
    explicit Expr(std::shared_ptr<ExprNode> n): node(std::move(n)) {}

    static Expr wireRef(const Wire* w);
};

// Helpers
inline Expr make_unary(OpType op, const Expr& x) {
    auto n = std::make_shared<ExprNode>();
    n->op = op; n->a = x.node; return Expr(n);
}

inline Expr make_binary(OpType op, const Expr& x, const Expr& y) {
    auto n = std::make_shared<ExprNode>();
    n->op = op; n->a = x.node; n->b = y.node; return Expr(n);
}

inline Expr make_select(const Expr& cond, const Expr& t, const Expr& e) {
    auto n = std::make_shared<ExprNode>();
    n->op = OpType::Select; n->a = cond.node; n->b = t.node; n->c = e.node; return Expr(n);
}

// Public conditional builder (If expression)
inline Expr If(const Expr& condition, const Expr& thenExpr, const Expr& elseExpr) {
    return make_select(condition, thenExpr, elseExpr);
}

// Expression operator overloads
inline Expr operator+(const Expr& x, const Expr& y) { return make_binary(OpType::Add, x, y); }
inline Expr operator-(const Expr& x, const Expr& y) { return make_binary(OpType::Sub, x, y); }
inline Expr operator*(const Expr& x, const Expr& y) { return make_binary(OpType::Mul, x, y); }
inline Expr operator/(const Expr& x, const Expr& y) { return make_binary(OpType::Div, x, y); }
inline Expr operator%(const Expr& x, const Expr& y) { return make_binary(OpType::Mod, x, y); }

inline Expr operator&(const Expr& x, const Expr& y) { return make_binary(OpType::BitAnd, x, y); }
inline Expr operator|(const Expr& x, const Expr& y) { return make_binary(OpType::BitOr, x, y); }
inline Expr operator^(const Expr& x, const Expr& y) { return make_binary(OpType::BitXor, x, y); }
inline Expr operator~(const Expr& x) { return make_unary(OpType::BitNot, x); }

inline Expr operator<<(const Expr& x, const Expr& y) { return make_binary(OpType::Shl, x, y); }
inline Expr operator>>(const Expr& x, const Expr& y) { return make_binary(OpType::Shr, x, y); }

inline Expr operator-(const Expr& x) { return make_unary(OpType::Neg, x); }
inline Expr operator!(const Expr& x) { return make_unary(OpType::LogNot, x); }
inline Expr operator&&(const Expr& x, const Expr& y) { return make_binary(OpType::LogAnd, x, y); }
inline Expr operator||(const Expr& x, const Expr& y) { return make_binary(OpType::LogOr, x, y); }

inline Expr operator==(const Expr& x, const Expr& y) { return make_binary(OpType::Eq, x, y); }
inline Expr operator!=(const Expr& x, const Expr& y) { return make_binary(OpType::Ne, x, y); }
inline Expr operator<(const Expr& x, const Expr& y) { return make_binary(OpType::Lt, x, y); }
inline Expr operator<=(const Expr& x, const Expr& y) { return make_binary(OpType::Le, x, y); }
inline Expr operator>(const Expr& x, const Expr& y) { return make_binary(OpType::Gt, x, y); }
inline Expr operator>=(const Expr& x, const Expr& y) { return make_binary(OpType::Ge, x, y); }

// Provide mixed operators with integers (on right-hand side)
inline Expr operator+(const Expr& x, long long y) { return x + Expr(y); }
inline Expr operator-(const Expr& x, long long y) { return x - Expr(y); }
inline Expr operator*(const Expr& x, long long y) { return x * Expr(y); }
inline Expr operator/(const Expr& x, long long y) { return x / Expr(y); }
inline Expr operator%(const Expr& x, long long y) { return x % Expr(y); }
inline Expr operator&(const Expr& x, long long y) { return x & Expr(y); }
inline Expr operator|(const Expr& x, long long y) { return x | Expr(y); }
inline Expr operator^(const Expr& x, long long y) { return x ^ Expr(y); }
inline Expr operator<<(const Expr& x, long long y) { return x << Expr(y); }
inline Expr operator>>(const Expr& x, long long y) { return x >> Expr(y); }
inline Expr operator==(const Expr& x, long long y) { return x == Expr(y); }
inline Expr operator!=(const Expr& x, long long y) { return x != Expr(y); }
inline Expr operator<(const Expr& x, long long y) { return x < Expr(y); }
inline Expr operator<=(const Expr& x, long long y) { return x <= Expr(y); }
inline Expr operator>(const Expr& x, long long y) { return x > Expr(y); }
inline Expr operator>=(const Expr& x, long long y) { return x >= Expr(y); }

// Mixed operators with integers (on left-hand side)
inline Expr operator+(long long x, const Expr& y) { return Expr(x) + y; }
inline Expr operator-(long long x, const Expr& y) { return Expr(x) - y; }
inline Expr operator*(long long x, const Expr& y) { return Expr(x) * y; }
inline Expr operator/(long long x, const Expr& y) { return Expr(x) / y; }
inline Expr operator%(long long x, const Expr& y) { return Expr(x) % y; }
inline Expr operator&(long long x, const Expr& y) { return Expr(x) & y; }
inline Expr operator|(long long x, const Expr& y) { return Expr(x) | y; }
inline Expr operator^(long long x, const Expr& y) { return Expr(x) ^ y; }
inline Expr operator<<(long long x, const Expr& y) { return Expr(x) << y; }
inline Expr operator>>(long long x, const Expr& y) { return Expr(x) >> y; }
inline Expr operator==(long long x, const Expr& y) { return Expr(x) == y; }
inline Expr operator!=(long long x, const Expr& y) { return Expr(x) != y; }
inline Expr operator<(long long x, const Expr& y) { return Expr(x) < y; }
inline Expr operator<=(long long x, const Expr& y) { return Expr(x) <= y; }
inline Expr operator>(long long x, const Expr& y) { return Expr(x) > y; }
inline Expr operator>=(long long x, const Expr& y) { return Expr(x) >= y; }

// Implementation of wireRef
inline Expr Expr::wireRef(const Wire* w) {
    auto n = std::make_shared<ExprNode>();
    n->op = OpType::WireRef; n->wire = w; return Expr(n);
}


