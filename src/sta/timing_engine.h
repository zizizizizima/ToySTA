#ifndef TSTA_TIMING_ENGINE_H
#define TSTA_TIMING_ENGINE_H

#include "sta/timing_graph.h"

namespace tsta {

// === Phase 2: Timing Propagation Engine ===
//
// Takes a finalized TimingGraph and computes timing values.
    class TimingEngine{
        public:
            //Forward propagation: computes arrival_time for all pins.
            void propagate_arrival_time(TimingGraph& tg);
            //Backward propagation: computes req_time for all pins.
            void propagate_required_time(TimingGraph& tg, float clock_period);
            //Slack = RAT - AT (negative = setup timing violation)
            static float slack(const Pin& pin);
    };
// class TimingEngine {
// public:
//     // 
//     //   Startpoints (pins with no fanin) get AT = 0
//     //   Then propagate forward: AT(succ) = max(AT(pred) + edge.delay)
//     //
//     // Use a WORKLIST approach (not DFS with visited set!):
//     //   1. Seed all startpoints with AT=0, push them onto a queue
//     //   2. Pop a node, look at its fanout, compute new AT for each successor
//     //   3. If new_AT > successor's current AT, update and push successor
//     //   4. Repeat until queue is empty
//     //
//     // Why worklist instead of DFS?
//     //   DFS with visited set is WRONG for reconvergent paths: when a node
//     //   can be reached from two different paths, the first visit marks it
//     //   visited and the second (slower/faster) path is never considered.
//     //
//     void propagate_arrival_time(TimingGraph& tg);
//
//     // Backward propagation: computes req_time for all pins.
//     //   Endpoints (pins with no fanout) get RAT = clock_period
//     //   Then propagate backward: RAT(pred) = min(RAT(succ) - edge.delay)
//     //
//     // Same worklist approach:
//     //   1. Seed endpoints with RAT=clock_period, push onto queue
//     //   2. Pop, look at fanin, compute new RAT for each predecessor
//     //   3. If new_RAT < predecessor's current RAT, update and push
//     //   4. Repeat
//     //
//     void propagate_required_time(TimingGraph& tg, float clock_period);
//
//     // Slack = RAT - AT (negative = setup timing violation)
//     static float slack(const Pin& pin);
//
// === Phase 4: Critical Path + Reporting ===
//
//     // Trace the worst path backward from endpoint to startpoint.
//     // At each step, pick the fanin predecessor with the smallest slack.
//     // Returns a vector of pin names from startpoint → endpoint.
//     // Use std::reverse() at the end (path is built backward).
//     std::vector<std::string> trace_critical_path(
//         const TimingGraph& tg, const std::string& endpoint) const;
//
//     // Print a timing report.
//     //   - Collect all (pin_name, slack) pairs
//     //   - Sort by slack ascending (worst first) using std::sort + lambda
//     //   - Print top 5 worst slacks, then all pins
//     void print_report(const TimingGraph& tg);
// };

// #include headers you need:
//   <queue> (worklist), <string>, <vector>, <algorithm> (Phase 4),
//   <limits> (for slack default return value)

} // namespace tsta

#endif // TSTA_TIMING_ENGINE_H
