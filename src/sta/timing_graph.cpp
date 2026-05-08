#include "sta/timing_graph.h"

namespace tsta {

// === Phase 1: Implement graph construction ===
//
// add_cell():
//   Move name and pins into a new Cell, push into cells_.
//   Phase 3: use std::make_unique<Cell>, push into cells_,
//            return Cell& (deref the unique_ptr).
//
// add_net():
//   Move name and pin_names into a new Net, push into nets_.
//   connections vector will be filled during finalize().
//
// add_timing_arc():
//   Move strings + type + delay into a new TimingArc, push into arcs_.
//
// finalize():
//   1. Clear pin_map_, fanout_, fanin_
//   2. For each cell, for each pin:
//        qualified_name = cell.name + "/" + pin.name
//        pin_map_[qualified_name] = &pin
//        reset pin.arrival_time and pin.req_time (Phase 2)
//   3. For each arc:
//        fanout_[from_pin].push_back({to_pin, delay})
//        fanin_[to_pin].push_back({from_pin, delay})
//   4. For each net:
//        a) Resolve pin names to Pin* pointers:
//             for each pin_name: lookup in pin_map_, push into net.connections
//        b) Collect driver pins (PinDir::Output) and load pins (everything else)
//        c) For each driver, for each load:
//             fanout_[driver].push_back({load, 0.0f})
//             fanin_[load].push_back({driver, 0.0f})
//
// lookup_pin():
//   Find the name in pin_map_, return the Pin* (or nullptr).
//
// fanout() / fanin() (Phase 2):
//   Look up in the map, return the vector (or empty vector if not found).
//   Use a static empty vector for the not-found case to avoid returning nullptr.

} // namespace tsta
