#include "sta/timing_engine.h"
#include "sta/timing_graph.h"
#include <queue>

using namespace std;
namespace tsta {
    // === Phase 2: Implement propagation ===
    // propagate_arrival_time():
    //   1. Reset all arrival_time in the graph
    //   2. Find startpoints (fanin(qualified_pin_name).empty())
    //   3. Seed: startpoint.arrival_time = 0.0f, push to worklist
    //   4. Worklist loop:
    //        pop cur, look up pin, get its AT
    //        for each edge in tg.fanout(cur):
    //            new_at = AT + edge.delay
    //            if new_at > dst.arrival_time (or dst hasn't been set):
    //                dst.arrival_time = new_at
    //                push edge.to_pin to worklist
    void TimingEngine::propagate_arrival_time(TimingGraph& tg) {
        // Implementation for forward propagation of arrival time
        for(const auto& cell : tg.cells()){
            for(const auto& pin : cell.Pins){
                pin.arrival_time.reset();
            }
        }
        
        queue<string> worklist;
        for(const auto& cell : tg.cells()){
            for(const auto& pin : cell.Pins){
                string qualified_name = cell.name + "/" + pin.name;
                if(tg.fanin(qualified_name).empty()){
                    pin.arrival_time = 0.0f;
                    worklist.push(qualified_name);
                }
            }
        }
        while (!worklist.empty()){
            string cur = worklist.front();
            worklist.pop();

            Pin* cur_pin = tg.lookup_pin(cur);
            float cur_at = cur_pin->arrival_time.value();
            //optional<float> AT = tg.fanout(cur).delay;
            for(const auto& edge : tg.fanout(cur)){
                //对每条出边单独处理
                string next_pin_name(edge.pin);
                Pin* next_pin = tg.lookup_pin(next_pin_name);
                float new_at = cur_at + edge.delay;
                //如果还没计算过，或者新计算的AT比原来的大，就更新
                if( !next_pin->arrival_time.has_value() ||
                    new_at > next_pin->arrival_time.value() ){
                    next_pin->arrival_time = new_at;
                    worklist.push(next_pin_name);
                }
            }
        }
    }

    void TimingEngine::propagate_required_time(TimingGraph& tg, float clock_period){
        for(const auto& cell : tg.cells()){
            for(const auto& pin : cell.Pins){
                pin.req_time.reset();
            }
        }
        // 2. Find endpoints (no fanout), seed RAT = clock_period
        queue<string> worklist;
        for (const auto& cell : tg.cells()){
            for (const auto& pin : cell.Pins){
                string qualified_name = cell.name + "/" + pin.name;
                if (tg.fanout(qualified_name).empty()){
                    pin.req_time = clock_period;
                    worklist.push(qualified_name);
                }
            }
        }
         // 3. Worklist: backward propagation
      while (!worklist.empty()) {
          string cur = worklist.front();
          worklist.pop();

          Pin* cur_pin = tg.lookup_pin(cur);
          float cur_rat = cur_pin->req_time.value();

          for (const auto& edge : tg.fanin(cur)) {
              // edge.pin is the SOURCE pin that feeds into cur
              string src_pin_name(edge.pin);
              Pin* src_pin = tg.lookup_pin(src_pin_name);
              float new_rat = cur_rat - edge.delay;

              // RAT: take the MIN (most restrictive)
              if (!src_pin->req_time.has_value() ||
                  new_rat < src_pin->req_time.value()) {
                  src_pin->req_time = new_rat;
                  worklist.push(src_pin_name);
              }
          }
        }
    }


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
