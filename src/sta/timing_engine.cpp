#include "sta/timing_engine.h"

namespace tsta {

// === Phase 2: Implement propagation ===
//
// propagate_arrival_time():
//   1. Reset all arrival_time in the graph (std::optional.reset())
//   2. Find startpoints (fanin(qualified_pin_name).empty())
//   3. Seed: startpoint.arrival_time = 0.0f, push to worklist
//   4. Worklist loop:
//        pop cur, look up pin, get its AT
//        for each edge in tg.fanout(cur):
//            new_at = AT + edge.delay
//            if new_at > dst.arrival_time (or dst hasn't been set):
//                dst.arrival_time = new_at
//                push edge.to_pin to worklist
//
// propagate_required_time():
//   Same structure but:
//   1. Find endpoints (fanout(qn).empty())
//   2. Seed: endpoint.req_time = clock_period
//   3. Worklist loop:
//        pop cur, get its RAT
//        for each edge in tg.fanin(cur):
//            new_rat = RAT - edge.delay
//            if new_rat < src.req_time (or src hasn't been set):
//                src.req_time = new_rat
//                push edge.to_pin to worklist  // note: to_pin is the src here
//
// slack():
//   If both arrival_time and req_time have values: return req - arrival
//   Otherwise: return std::numeric_limits<float>::max()
//
// === Phase 4: Implement reporting ===
//
// trace_critical_path():
//   Start from endpoint, walk backward:
//     for each fanin edge, look up the source pin, compute its slack
//     pick the one with the smallest slack
//     add it to path, set current = that pin
//     stop when fanin is empty (reached startpoint)
//   Reverse the path before returning.
//
//   Edge case: if multiple predecessors have the same slack,
//   any one is fine (they're all equally critical).
//
// print_report():
//   Use std::vector<std::pair<std::string, float>> to collect pin_slacks.
//   Use std::sort with a lambda that compares .second.
//   Print "Worst slacks:" header, then first 5 entries.
//   Then print "All pins:" with AT, RAT, slack detail.

} // namespace tsta
