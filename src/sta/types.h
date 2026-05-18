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
    enum class ArcType { Combinational, Setup, Hold };
    //e.g. Pin{"A", Input}
    struct Pin{
        string name;
        PinDir dir;
    };

//   Phase 2 will add: mutable std::optional<float> arrival_time, req_time
//   - "mutable" because propagation modifies state on a logically-const graph
//   - "std::optional" means "not computed yet"
//
    /* A device instance like U1 (AND2 gate), FF1 (flip-flop)
    e.g.Cell{"U1", {Pin{"A", Input}, Pin{"B", Input}, Pin{"Y", Output}}}*/
    struct Cell{
        string name;
        vector<Pin> Pins;
    };
//   - Phase 3: pins stay as std::vector<Pin> (value semantics inside Cell)

    /*A delay path WITHIN a cell (input pin → output  pin)
    Example: U1/A → U1/Y*/
    /*RULE: TimingArc: Each TimingArc directly creates one edge from its from_pin to its to_pin with the arc's delay.*/
    struct TimingArc{
        string from_pin;
        string to_pin;
        ArcType type;
        float delay;
    };

    /*A wire between cells
    Example: Connects U1/Y to U2/A*/
    /*RULE: Net: Each net connects an output pin (driver)
   to all input pins (loads) on that net, with zero
  delay (ideal wire assumption — no RC delay yet).*/
    struct Net{
        string name;
        vector<string> pin_names;
        vector<Pin*> connections;
    };

//   - finalize() resolves them to Pin* pointers.
// }

// #include headers you need:
//   <string>, <vector>, <optional> (for Phase 2)

} // namespace tsta

#endif // TSTA_TYPES_H
