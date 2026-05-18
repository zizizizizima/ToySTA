#ifndef TSTA_TIMING_GRAPH_H
#define TSTA_TIMING_GRAPH_H

#include <string>
#include <vector>
#include <string_view>
#include <unordered_map>

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
            const Pin* lookup_pin(const string& name) const;//函数重载
            // Read access to internal data (for engine, reporting):
            const std::vector<Cell>& cells() const;
            const std::vector<Net>& nets() const;
            const std::vector<TimingArc>& arcs() const;
            
            // Timing propagation graph adjacency lists.
            // Built during finalize() from arcs + nets.
            struct TimingEdge{
                string_view to_pin; //the destination pin 
                float delay;
            };
            // How edges are built:
            //   1. Each TimingArc creates one edge from_pin → to_pin
            //   2. Each Net creates edges from every output pin to every input
            //      pin in that net (aka the "driver" drives the "loads").
            //      Net edges have delay = 0.0 (ideal wire).
            //
            // Use std::unordered_map<std::string, std::vector<TimingEdge>>
            // for both fanout_ and fanin_.
            /*In logic design,
            fanout usually means "how many loads a gate drives" (a
            number). Here it means the set of destination pins
            reachable from a given pin.
            fanout_["U1/Y"] = {"U2/A", 0.0}   // U1/Y can reach
            U2/A via this net
            fanin_["U2/A"]  = {"U1/Y", 0.0}   // U2/A can be
            reached from U1/Y
            Same words, different perspective — gate-level fanout
            is a count, timing-graph fanout is a list of edges.*/
            const vector<TimingEdge>& fanout(const string& pin) const;
            const vector<TimingEdge>& fanin(const string& pin) const;


        private:
            vector<Cell> cells_;
            vector<Net> nets_;
            vector<TimingArc> arcs_;
            unordered_map<string, Pin*> pin_map_;  // built in finalize()
            unordered_map<string, vector<TimingEdge>> fanout_;
            unordered_map<string, vector<TimingEdge>> fanin_;
    };


// class TimingGraph {
// public:
//     phase 1 API done.√
//     Phase 2 additions (for timing propagation) done.√

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
