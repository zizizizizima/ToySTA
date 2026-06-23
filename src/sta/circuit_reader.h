#ifndef TSTA_CIRCUIT_READER_H
#define TSTA_CIRCUIT_READER_H

#include <string>
#include <vector>

#include "timing_graph.h"

namespace tsta {

// === Phase 5: CircuitReader ===
//
// Reads a custom .cir circuit file and populates a TimingGraph.
//
// Example .cir file:
//
//     CELL U1 (A, B, >Y)
//     ARC U1/A -> U1/Y 0.3
//     ARC U1/B -> U1/Y 0.3
//     CELL U2 (A, B, >Y)
//     ARC U2/A -> U2/Y 0.5
//     ARC U2/B -> U2/Y 0.5
//     NET N1 (U1/Y U2/A)
//     CLOCK_PERIOD 2.0
//
// See CLAUDE.md for full format reference.
//
// Usage:
//     TimingGraph tg;
//     float period = CircuitReader::read_file("my_circuit.cir", tg);
//     tg.finalize();
//     TimingEngine engine;
//     engine.propagate_arrival_time(tg);
//     engine.propagate_required_time(tg, period);
class CircuitReader {
public:
    // Reads a .cir file and populates the TimingGraph.
    // Returns the clock period parsed from the file (0.0f if not specified).
    static float read_file(const std::string& filename, TimingGraph& tg);

private:
    // === File-level helpers ===

    // Split a string by delimiter (e.g. split("A,B", ',') → {"A", "B"})
    static std::vector<std::string> split(const std::string& str, char delimiter);

    // Remove leading/trailing whitespace from a string
    static std::string trim(const std::string& str);

    // Remove parentheses from the ends of a string if present
    // e.g. strip_parens("(A, B, >Y)") → "A, B, >Y"
    static std::string strip_parens(const std::string& str);

    // === Line parsers (each handles one directive type) ===

    // Parses: CELL <name> (<pin1>, <pin2>, ..., <pinN>)
    //   - Pins prefixed with '>' are Output, others are Input
    //   - Calls tg.add_cell(...)
    //
    // Hints:
    //   1. Use split() and trim() to tokenize the line
    //   2. Cell name is after "CELL " — grab the substring
    //   3. Pin list is inside parentheses — use strip_parens()
    //   4. Split pin list by ',' and trim each pin name
    //   5. Check first char of each pin: '>' means Output, strip it
    //   6. Build a Cell struct and call tg.add_cell()
    static void parse_cell(const std::string& line, TimingGraph& tg);

    // Parses: ARC <from_pin> -> <to_pin> <delay>
    //   - from_pin and to_pin are qualified names (e.g. "U1/A")
    //   - delay is a floating-point number
    //   - Calls tg.add_timing_arc(...)
    //
    // Hints:
    //   1. The " -> " separator is the key marker — find it
    //   2. Left of " -> " is from_pin, right is the rest: "to_pin delay"
    //   3. The rest splits into to_pin and delay (space-separated)
    //   4. Use std::stof() to convert the delay string to float
    //   5. ArcType: for simplicity, treat all arcs as Combinational
    //   6. Build a TimingArc struct and call tg.add_timing_arc()
    static void parse_arc(const std::string& line, TimingGraph& tg);

    // Parses: NET <name> (<pin_name1> <pin_name2> ...)
    //   - Pins inside parens are space-separated qualified names
    //   - Calls tg.add_net(...)
    //
    // Hints:
    //   1. Net name is after "NET " — grab up to the '('
    //   2. Pin names are inside parentheses — use strip_parens()
    //   3. Split by space (not comma!) to get individual pin names
    //   4. Build a Net struct and call tg.add_net()
    static void parse_net(const std::string& line, TimingGraph& tg);

    // Parses: CLOCK_PERIOD <value>
    //   - Returns the floating-point value
    //   - Returns 0.0f if parsing fails
    static float parse_clock_period(const std::string& line);
};

} // namespace tsta

#endif // TSTA_CIRCUIT_READER_H
