#include "simulate.h"
#include "wire.h"
#include "operations.h"

#include <unordered_map>
#include <unordered_set>
#include <deque>

struct EvalContext {
    std::unordered_map<const Wire*, long long> comb_cache;
};

static long long eval_node(const std::shared_ptr<ExprNode>& n, EvalContext& ctx);

static long long wire_value(const Wire* w, EvalContext& ctx) {
    if (w->comb_expr) {
        auto it = ctx.comb_cache.find(w);
        if (it != ctx.comb_cache.end()) return it->second;
        long long v = eval_node(w->comb_expr, ctx);
        ctx.comb_cache.emplace(w, v);
        return v;
    }
    return w->committed_value;
}

static long long truthy(long long v) { return v != 0 ? 1 : 0; }

static long long eval_node(const std::shared_ptr<ExprNode>& n, EvalContext& ctx) {
    switch (n->op) {
        case OpType::Constant: return n->constant_value;
        case OpType::WireRef: return wire_value(n->wire, ctx);
        case OpType::Add: return eval_node(n->a, ctx) + eval_node(n->b, ctx);
        case OpType::Sub: return eval_node(n->a, ctx) - eval_node(n->b, ctx);
        case OpType::Mul: return eval_node(n->a, ctx) * eval_node(n->b, ctx);
        case OpType::Div: {
            long long d = eval_node(n->b, ctx); return d == 0 ? 0 : (eval_node(n->a, ctx) / d);
        }
        case OpType::Mod: {
            long long d = eval_node(n->b, ctx); return d == 0 ? 0 : (eval_node(n->a, ctx) % d);
        }
        case OpType::BitAnd: return eval_node(n->a, ctx) & eval_node(n->b, ctx);
        case OpType::BitOr: return eval_node(n->a, ctx) | eval_node(n->b, ctx);
        case OpType::BitXor: return eval_node(n->a, ctx) ^ eval_node(n->b, ctx);
        case OpType::BitNot: return ~eval_node(n->a, ctx);
        case OpType::Neg: return -eval_node(n->a, ctx);
        case OpType::Shl: return eval_node(n->a, ctx) << eval_node(n->b, ctx);
        case OpType::Shr: return eval_node(n->a, ctx) >> eval_node(n->b, ctx);
        case OpType::LogAnd: return truthy(eval_node(n->a, ctx)) && truthy(eval_node(n->b, ctx));
        case OpType::LogOr: return truthy(eval_node(n->a, ctx)) || truthy(eval_node(n->b, ctx));
        case OpType::LogNot: return !truthy(eval_node(n->a, ctx));
        case OpType::Eq: return eval_node(n->a, ctx) == eval_node(n->b, ctx);
        case OpType::Ne: return eval_node(n->a, ctx) != eval_node(n->b, ctx);
        case OpType::Lt: return eval_node(n->a, ctx) < eval_node(n->b, ctx);
        case OpType::Le: return eval_node(n->a, ctx) <= eval_node(n->b, ctx);
        case OpType::Gt: return eval_node(n->a, ctx) > eval_node(n->b, ctx);
        case OpType::Ge: return eval_node(n->a, ctx) >= eval_node(n->b, ctx);
        case OpType::Select: return truthy(eval_node(n->a, ctx)) ? eval_node(n->b, ctx) : eval_node(n->c, ctx);
    }
    return 0;
}

static void gather_from_expr(const std::shared_ptr<ExprNode>& n, std::unordered_set<const Wire*>& wires);

static void gather_from_expr(const std::shared_ptr<ExprNode>& n, std::unordered_set<const Wire*>& wires) {
    if (!n) return;
    if (n->op == OpType::WireRef && n->wire) {
        if (wires.insert(n->wire).second) {
            gather_from_expr(n->wire->comb_expr, wires);
            gather_from_expr(n->wire->next_expr, wires);
        }
    } else {
        if (n->a) gather_from_expr(n->a, wires);
        if (n->b) gather_from_expr(n->b, wires);
        if (n->c) gather_from_expr(n->c, wires);
    }
}

static std::vector<const Wire*> closure_from_targets(const std::vector<const Wire*>& targets) {
    std::unordered_set<const Wire*> set;
    for (auto* w : targets) if (w) set.insert(w);
    size_t prev = 0;
    while (prev != set.size()) {
        prev = set.size();
        auto snapshot = std::vector<const Wire*>(set.begin(), set.end());
        for (const Wire* w : snapshot) {
            gather_from_expr(w->comb_expr, set);
            gather_from_expr(w->next_expr, set);
        }
    }
    return std::vector<const Wire*>(set.begin(), set.end());
}

std::map<std::string, std::vector<long long>> simulate(const std::vector<const Wire*>& targets, int cycles, bool restore_state) {
    std::vector<const Wire*> all_wires = closure_from_targets(targets);

    std::unordered_map<const Wire*, long long> saved;
    if (restore_state) {
        for (const Wire* w : all_wires) saved[w] = w->committed_value;
    }

    std::map<std::string, std::vector<long long>> history;
    for (const Wire* w : all_wires) history[w->name] = {};

    for (int t = 0; t < cycles; ++t) {
        EvalContext ctx;

        for (const Wire* w : all_wires) {
            long long v = wire_value(w, ctx);
            history[w->name].push_back(v);
        }

        std::unordered_map<const Wire*, long long> next_values;
        for (const Wire* w : all_wires) {
            if (w->next_expr) {
                long long nv = eval_node(w->next_expr, ctx);
                next_values[w] = nv;
            }
        }

        for (const auto& kv : next_values) {
            const_cast<Wire*>(kv.first)->committed_value = kv.second;
        }
    }

    if (restore_state) {
        for (const auto& kv : saved) {
            const_cast<Wire*>(kv.first)->committed_value = kv.second;
        }
    }

    return history;
}

std::map<std::string, std::vector<long long>> simulate(const Wire& target, int cycles, bool restore_state) {
    return simulate(std::vector<const Wire*>{ &target }, cycles, restore_state);
}


