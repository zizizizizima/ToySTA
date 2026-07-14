#include "sta/timing_engine.h"
#include "sta/timing_graph.h"
#include <queue>
#include <limits>
#include <algorithm> // for std::reverse
#include <iostream>

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

    //模型简化，默认在单时钟域下进行计算
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
    
    float TimingEngine::slack(const Pin* pin) {
        if (pin->arrival_time.has_value() && pin->req_time.has_value()) {
            return pin->req_time.value() - pin->arrival_time.value();
        } else {
            return numeric_limits<float>::max();
        }
    }

    vector<string> TimingEngine::trace_critical_path(const TimingGraph& tg, const string& endpoint) const {
        vector<TimingGraph::TimingEdge> fanin_edges = tg.fanin(endpoint);
        vector<string> path;
        path.push_back(endpoint);

        optional<float> min_slack = nullopt;
        string critical_pin;
        while(!fanin_edges.empty()){
            min_slack = nullopt;
            for (const auto& edge : fanin_edges) {
                const string_view& previous_pin_name = edge.pin; //因为是从fanin_中取出的，所以是前驱节点
                const Pin* previous_pin = tg.lookup_pin(string(previous_pin_name));
                if(min_slack == nullopt || slack(previous_pin) < min_slack.value()){
                    min_slack = slack(previous_pin);
                    critical_pin = string(previous_pin_name);
                }
            }
            path.push_back(critical_pin);
            fanin_edges = tg.fanin(critical_pin);
        }
        reverse(path.begin(), path.end());
        return path;
    }

    void TimingEngine::print_report(const TimingGraph& tg) const {
        // Collect all (pin_name, slack) pairs, sort ascending (worst first)
        vector<pair<string, float>> pin_slacks;
        for (const auto& cell : tg.cells()) {
            for (const auto& pin : cell.Pins) {
                string qname = cell.name + "/" + pin.name;
                pin_slacks.emplace_back(qname, slack(&pin));
            }
        }
        sort(pin_slacks.begin(), pin_slacks.end(),
             [](const auto& a, const auto& b) { return a.second < b.second; });

        cout << "\n=== Timing Report ===\n";

        // Top 5 worst slacks
        cout << "Worst slacks:\n";
        int n = min(5, (int)pin_slacks.size());
        for (int i = 0; i < n; ++i) {
            cout << "  " << pin_slacks[i].first
                 << "  slack = " << pin_slacks[i].second << '\n';
        }

        // All pins detail
        cout << "\nAll pins:\n";
        for (const auto& cell : tg.cells()) {
            for (const auto& pin : cell.Pins) {
                string qname = cell.name + "/" + pin.name;
                string at = pin.arrival_time.has_value()
                                ? to_string(pin.arrival_time.value())
                                : "N/A";
                string rat = pin.req_time.has_value()
                                ? to_string(pin.req_time.value())
                                : "N/A";
                cout << "  " << qname
                     << "  AT=" << at
                     << "  RAT=" << rat
                     << "  slack=" << slack(&pin) << '\n';
            }
        }
    }
} // namespace tsta
