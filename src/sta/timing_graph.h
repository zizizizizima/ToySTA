#ifndef TSTA_TIMING_GRAPH_H
#define TSTA_TIMING_GRAPH_H

#include <string>
#include <vector>
#include <string_view>
#include <unordered_map>

#include "types.h"

using namespace std;

namespace tsta {
    class TimingGraph {
        public:
            // How edges are built:
            //   1. Each TimingArc creates one edge: key pin → edge.pin
            //   2. Each Net creates edges from every output pin to every input
            struct TimingEdge{
                string_view pin; // In fanout_: destination reachable from key pin.
                                 // In fanin_:   source that can reach the key pin.
                float delay;
            };

            void add_cell(Cell cell);
            void add_net(Net net);
            void add_timing_arc(TimingArc arc);
            void finalize();// Call this after all cells/nets/arcs are added:s
            // Pin lookup by qualified name:
            Pin* lookup_pin(const string& name);
            const Pin* lookup_pin(const string& name) const;//函数重载
            // Read access to internal data (for engine, reporting):
            const std::vector<Cell>& cells() const {return cells_;}
            const std::vector<Net>& nets() const {return nets_;}
            const std::vector<TimingArc>& arcs() const {return arcs_;}
            const vector<TimingEdge>& fanout(const string& pin) const;
            const vector<TimingEdge>& fanin(const string& pin) const;

        private:
            vector<Cell> cells_;// Phase 3: vector<unique_ptr<Cell>>
            vector<Net> nets_;
            vector<TimingArc> arcs_;
            unordered_map<string, Pin*> pin_map_;  // built in finalize()
            // fanout_ and fanin_ are timing propagation graph adjacency lists, built during finalize() from arcs + nets.
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
// };

// #include headers you need:
//   <memory> (Phase 3), <string>, <string_view>, <unordered_map>, <vector>
//   "sta/types.h"

} // namespace tsta

#endif // TSTA_TIMING_GRAPH_H
