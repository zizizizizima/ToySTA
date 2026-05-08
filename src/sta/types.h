#ifndef TSTA_TYPES_H
#define TSTA_TYPES_H

#include <string>
#include <vector>

using namespace std;


namespace tsta {

// === Phase 1: Core Data Types ===
//
// These are the building blocks of your timing graph.
//
// TODO: Define these types:

    enum class PinDir {Input, Output};
// enum class PinDir { Input, Output }
//   - Every pin has a direction.
    enum class ArcType { Combinational, Setup, Hold };
// enum class ArcType { Combinational, Setup, Hold }
//   - What kind of timing arc is this? Start with just Combinational.
    struct Pin{
        string name;
        PinDir dir;
    };
// struct Pin {
//     std::string name;
//     PinDir dir;
//   - A pin belongs to a cell (e.g., "A", "B", "Y").
//   - Don't store the cell name here — the graph's lookup map will handle
//     qualified names like "U1/A".
//
//   Phase 2 will add: mutable std::optional<float> arrival_time, req_time
//   - "mutable" because propagation modifies state on a logically-const graph
//   - "std::optional" means "not computed yet"
//
    struct Cell{
        string name;
        vector<Pin> Pins;
    };
// struct Cell {
//     std::string name;
//     std::vector<Pin> pins;
//   - e.g. Cell{"U1", {Pin{"A", Input}, Pin{"B", Input}, Pin{"Y", Output}}}
//   - Phase 3: pins stay as std::vector<Pin> (value semantics inside Cell)

    struct TimingArc{
        string from_pin;
        string to_pin;
        ArcType type;
        float delay;
    };
// struct TimingArc {
//     std::string from_pin;   // qualified name, e.g. "U1/A"
//     std::string to_pin;     // qualified name, e.g. "U1/Y"
//     ArcType type;
//     float delay;            // in nanoseconds
//   - Represents a timing path from an input pin to an output pin within a cell.
//
    struct Net{
        string name;
        vector<string> pin_names;
        vector<Pin*> connections;
    };
// struct Net {
//     std::string name;
//     std::vector<std::string> pin_names;  // pin names during construction
//     std::vector<Pin*> connections;       // resolved by finalize()
//   - A wire connecting pins (usually one output to one or more inputs).
//   - During graph building, store pin names as strings.
//   - finalize() resolves them to Pin* pointers.
// }

// #include headers you need:
//   <string>, <vector>, <optional> (for Phase 2)

} // namespace tsta

#endif // TSTA_TYPES_H
