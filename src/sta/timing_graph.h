#ifndef TSTA_TIMING_GRAPH_H
#define TSTA_TIMING_GRAPH_H

#include <string>
#include <vector>

#include "types.h"

using namespace std;

namespace tsta {

// === Phase 1: Timing Graph ===
//
// The central data structure. Owns all cells, nets, timing arcs, and
// provides pin lookup + adjacency lists for timing propagation.
//
// Key design:
//   - Pins are identified by qualified name "CellName/PinName" (e.g. "U1/A")
//   - finalize() must be called after building, before propagation
//   - finalize() is safe to call multiple times (rebuilds on each call)
//
// === Phase 1 API (build the graph) ===
    class TimingGraph {
    public:
        void add_cell(Cell cell);
        void add_net(Net net);
        void add_timing_arc(TimingArc arc);
        void finalize();// Call this after all cells/nets/arcs are added:s
        // Pin lookup by qualified name:
        Pin* lookup_pin(const string& name);
        const Pin* lookup_pin(const std::string& name) const;//函数重载
        // Read access to internal data (for engine, reporting):
        const std::vector<Cell>& cells() const;
        const std::vector<Net>& nets() const;
        const std::vector<TimingArc>& arcs() const;
// class TimingGraph {
// public:
//     phase 1 API done.√
// === Phase 2 additions (for timing propagation) ===
//
//     // Timing propagation graph adjacency lists.
//     // Built during finalize() from arcs + nets.
//     struct TimingEdge {
//         std::string_view to_pin;  // the destination pin name
//         float delay;
//     };
//     const std::vector<TimingEdge>& fanout(const std::string& pin) const;
//     const std::vector<TimingEdge>& fanin(const std::string& pin) const;
//
//     // How edges are built:
//     //   1. Each TimingArc creates one edge from_pin → to_pin
//     //   2. Each Net creates edges from every output pin to every input
//     //      pin in that net (aka the "driver" drives the "loads").
//     //      Net edges have delay = 0.0 (ideal wire).
//     //
//     // Use std::unordered_map<std::string, std::vector<TimingEdge>>
//     // for both fanout_ and fanin_.
//
// === Phase 3: Smart pointer refactor ===
//     Change std::vector<Cell> cells_ → std::vector<std::unique_ptr<Cell>> cells_
//     add_cell() returns Cell& instead of void
//     Iteration becomes cell->pins / cell->name instead of cell.pins / cell.name
//
// private:
//     std::vector<Cell> cells_;       // Phase 3: vector<unique_ptr<Cell>>
//     std::vector<Net> nets_;
//     std::vector<TimingArc> arcs_;
//     std::unordered_map<std::string, Pin*> pin_map_;  // built in finalize()
//     std::unordered_map<std::string, std::vector<TimingEdge>> fanout_;  // Phase 2
//     std::unordered_map<std::string, std::vector<TimingEdge>> fanin_;   // Phase 2
// };

// #include headers you need:
//   <memory> (Phase 3), <string>, <string_view>, <unordered_map>, <vector>
//   "sta/types.h"

} // namespace tsta

#endif // TSTA_TIMING_GRAPH_H
