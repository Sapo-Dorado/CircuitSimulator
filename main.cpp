#include <iostream>
#include "operations.h"
#include "wire.h"
#include "simulate.h"

// Demo usage only
int main() {
    // Inputs/constants
    Wire a("a");
    Wire b("b");
    a = 1;      // instantaneous
    b = 2;      // instantaneous

    // Combinational wire
    Wire sum("sum");
    sum = a + b; // updates immediately within a cycle

    // Registered accumulator
    Wire acc("acc", 0);
    acc << acc + sum; // next-cycle update

    // Conditional example: max of a and b
    Wire maxab("maxab");
    maxab = If(a > b, a, b);

    // Simulate and print histories
    auto histories = simulate(std::vector<const Wire*>{ &sum, &acc, &maxab }, 6);
    for (const auto& kv : histories) {
        std::cout << kv.first << ": ";
        for (size_t i = 0; i < kv.second.size(); ++i) {
            if (i) std::cout << ", ";
            std::cout << kv.second[i];
        }
        std::cout << "\n";
    }

    // Expected intuition:
    // sum = 3 every cycle (combinational)
    // acc starts at 0, then accumulates +3 each cycle: 0,3,6,9,12,15
    // maxab = 2 every cycle

    return 0;
}


