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
// === Phase 4: Critical path tracing on a sequential circuit ===
//   Build the flip-flop + combinational circuit from circuit_reader.h's example:
//     FF1 (CK, D, >Q)  CK→Q=0.3
//     INV1 (A, >Y)     A→Y=0.5
//     BUF1 (A, >Y)     A→Y=0.2
//     OR2  (A, B, >Y)  A→Y=0.4, B→Y=0.4
//     FF2 (CK, D, >Q)  CK→Q=0.3
//     Nets: FF1/Q → {BUF1/A, INV1/A}, BUF1/Y → OR2/A, INV1/Y → OR2/B, OR2/Y → FF2/D
//   Expected:
//     AT(FF2/D) through fast path (FF1 → BUF1 → OR2): 0.3+0.2+0.4 = 0.9
//     AT(FF2/D) through slow path (FF1 → INV1 → OR2): 0.3+0.5+0.4 = 1.2
//     Slack(FF2/D) = 5.0 - 1.2 = 3.8
//     trace_critical_path(tg, "FF2/D") should return [FF1/CK, FF1/Q, INV1/A, INV1/Y, OR2/B, OR2/Y, FF2/D]
//   Verify: critical path goes through INV1 (the slow path), not BUF1.

#include <iostream>
#include "sta/circuit_reader.h"
#include "sta/timing_engine.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: ToySTA <file.cir>\n";
        return 1;
    }

    tsta::TimingGraph tg;
    float period = tsta::CircuitReader::read_file(argv[1], tg);
    if (period == 0.0f) {
        std::cerr << "Warning: clock period not set (defaulting to 5.0)\n";
        period = 5.0f;
    }
    tg.finalize();

    tsta::TimingEngine engine;
    engine.propagate_arrival_time(tg);
    engine.propagate_required_time(tg, period);
    engine.print_report(tg);

    return 0;
}
