## Circuit Simulator (C++) [Vibe Coded]

This repository contains a small C++ library for simulating digital circuits using operator-overloaded expressions. It supports:

- **instantaneous (combinational) assignments** via `=`
- **next-cycle (registered) assignments** via `<<`
- **arithmetic, bitwise, logical, and comparison** operators on expressions
- a ternary-style conditional via **`If(condition, thenExpr, elseExpr)`**
- **multi-cycle simulation** that returns value histories for selected wires

The API is light-weight and header-driven, so you can define circuits directly in C++ code with natural syntax.

### Core files
- `operations.h`: Expression types (`OpType`, `ExprNode`, `Expr`) and operator overloads, plus `If()`
- `wire.h`: `Wire` class and assignment semantics (`=`, `<<`)
- `simulate.h` / `simulate.cpp`: Evaluation engine and `simulate` API

## Quick start

### 1) Define a circuit
```cpp
#include "operations.h"
#include "wire.h"
#include "simulate.h"
#include <iostream>

int main() {
  // Inputs/constants
  Wire a("a");
  Wire b("b");
  a = 1;          // instantaneous
  b = 2;          // instantaneous

  // Combinational wire
  Wire sum("sum");
  sum = a + b;    // updates immediately within a cycle

  // Registered accumulator (next-cycle update)
  Wire acc("acc", 0);
  acc << acc + sum; // value takes effect next clock edge

  // Conditional example (ternary)
  Wire maxab("maxab");
  maxab = If(a > b, a, b);

  // Simulate 6 cycles for selected wires
  auto histories = simulate(std::vector<const Wire*>{ &sum, &acc, &maxab }, 6);
  for (const auto& kv : histories) {
    std::cout << kv.first << ": ";
    for (size_t i = 0; i < kv.second.size(); ++i) {
      if (i) std::cout << ", ";
      std::cout << kv.second[i];
    }
    std::cout << "\n";
  }
}
```

### 2) Build
```bash
clang++ -std=c++17 -O2 -Wall -Wextra main.cpp simulate.cpp -o simulator
```
or
```bash
g++ -std=c++17 -O2 -Wall -Wextra main.cpp simulate.cpp -o simulator
```

### 3) Run
```bash
./simulator
```

## Concepts

- **Wire**: Named signal that holds a 64-bit signed integer value. Can have:
  - a combinational (instantaneous) definition via `=`
  - a registered (next-cycle) definition via `<<`
- **Expr**: Expression graph built by operator overloading. Supports constants, wires, arithmetic/bitwise/logical operations, comparisons, shifts, negation, and `If()`.
- **Cycle**: Each call to `simulate(..., cycles)` evaluates combinational logic for the current committed wire values, records results, then applies any `<<` updates at the clock edge to become the next cycle’s committed values.

## API overview

### Wires
- **Constructors**:
  - `Wire(std::string name, long long initialValue = 0)`
- **Assignments**:
  - `wire = expr;`  instantaneous/combinational definition
  - `wire << expr;` next-cycle (registered) definition
  - You can also assign integer constants directly, e.g., `wire = 42;`, `wire << 42;`

### Expressions and operators
- Build expressions from constants and wires: `Expr` is implicitly constructible from `Wire` and `long long`.
- Supported operators (on `Expr`):
  - **Arithmetic**: `+`, `-`, `*`, `/`, `%`
  - **Bitwise**: `&`, `|`, `^`, `~`, `<<`, `>>`
  - **Logical**: `!`, `&&`, `||` (uses 0/1 semantics)
  - **Comparison**: `==`, `!=`, `<`, `<=`, `>`, `>=` (result is 0 or 1)
  - **Ternary**: `If(condition, thenExpr, elseExpr)`

### Simulation
```cpp
std::map<std::string, std::vector<long long>> simulate(const std::vector<const Wire*>& targets,
                                                       int cycles,
                                                       bool restore_state = true);

std::map<std::string, std::vector<long long>> simulate(const Wire& target,
                                                       int cycles,
                                                       bool restore_state = true);
```

- **targets**: The wires you want in the output. The simulator automatically includes all transitive dependencies.
- **cycles**: Number of clock cycles to simulate.
- **restore_state**: If true (default), committed wire values are restored to their pre-simulation values after the run. Set to `false` to leave state advanced after simulation.
- **return value**: A map of `wireName -> [values per cycle]` for each target and all dependencies discovered during evaluation.

## Semantics and details

- **Combinational evaluation**: Within a cycle, a wire’s combinational value (from `=`) is computed from the current committed values and other combinational expressions. Evaluation is memoized per wire per cycle.
- **Registered update (`<<`)**: Expressions assigned with `<<` are evaluated using the current cycle’s combinational context; their results become the next cycle’s committed values at the clock edge.
- **Initial values**: The second argument to `Wire(name, init)` sets the initial committed value, used at cycle 0.
- **Truthiness**: Non-zero is true; zero is false. Logical ops (`&&`, `||`, `!`) output 0/1.
- **Division/modulo by zero**: Defined to yield 0.
- **Types**: All values are `long long` (signed 64-bit).
- **Targets vs all wires**: The output includes all wires in the dependency closure of the targets, not just the targets themselves.
- **State restore**: By default, the simulation does not mutate your wires after it completes. Pass `restore_state=false` to advance state permanently.

## Example: Counter with enable
```cpp
Wire enable("enable");
Wire value("value", 0);

// External control
enable = 1;  // combinational input

// value increments when enable is true (1)
value << If(enable, value + 1, value);

auto hist = simulate(value, 5);
// value: 0, 1, 2, 3, 4
```

## Tips and pitfalls
- **Avoid combinational loops** (e.g., `w = w + 1;` without `<<`). They lead to undefined behavior (infinite recursion). Use `<<` for feedback paths.
- **Multiple definitions**: If you assign both `=` and `<<` to a wire, the `=` defines the immediate/combinational value for the current cycle, while `<<` defines how the committed value updates at the clock edge.
- **Constant drivers**: You can directly assign integers on either side of expressions (mixed operators are supported).
- **Observability**: If you only want histories for specific wires, pass exactly those as targets; dependencies will be included automatically.

## Extending the library
The code is intentionally compact and easy to extend:
- Add new operations by extending `OpType`, `eval_node` in `simulate.cpp`, and adding matching operator overloads in `operations.h`.
- Add width-aware semantics, overflow modes, or a boolean type if you need stronger typing.
- Swap the evaluation strategy (e.g., topological sort) for enhanced diagnostics or cycle detection.

## License
MIT


