// ToySTA — Main testbench
//
// You'll build a circuit here to test each phase.
//
// === Phase 1: Build a simple circuit ===
//   - Create a TimingGraph
//   - Add 2 cells: U1 (AND gate, pins: A B Y), U2 (OR gate, pins: A B Y)
//   - Add timing arcs: U1/A→U1/Y (0.3ns), U1/B→U1/Y (0.3ns),
//                      U2/A→U2/Y (0.5ns), U2/B→U2/Y (0.5ns)
//   - Add a net: N1 connecting U1/Y → U2/A
//   - Call tg.finalize()
//   - Test: lookup "U1/Y", check it's not null and has PinDir::Output
//
// === Phase 2: Test propagation ===
//   - Create TimingEngine, call propagate_arrival_time() and propagate_required_time(tg, 2.0f)
//   - Verify: U1/Y AT = 0.3, U2/Y AT = 0.8
//   - Verify: U1/A slack = 1.2, U2/Y slack = 1.2
//
// === Phase 3: No main.cpp changes needed (internal refactor) ===
//
// === Phase 4: Expand the circuit ===
//   Add more cells to create diverging + reconverging paths:
//     U1 (AND, 0.3) → U1/Y splits to U2 (BUF, 0.1) and U3 (OR, 0.5)
//     U2/Y → U4/A,  U3/Y → U4/B,  U4 (OR, 0.2) → Y
//   Use engine.print_report() to see sorted slack output.
//   Use engine.trace_critical_path(tg, "U4/Y") to find the worst path.
//   Verify: critical path goes through U3 (the slow path), not U2.

#include <iostream>

int main()
{
    std::cout << "ToySTA v0.1\n";
    return 0;
}
