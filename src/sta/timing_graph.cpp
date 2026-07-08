#include "sta/timing_graph.h"

namespace tsta {

// === Phase 1: Implement graph construction ===
//
    //   Move name and pins into a new Cell, push into cells_.
    void TimingGraph::add_cell(Cell cell) {
        // Implementation for adding a cell to the graph
        cells_.push_back(cell);
    }
    //   Move name and pin_names into a new Net, push into nets_.
    void TimingGraph::add_net(Net net){
        nets_.push_back(net);
    }
    //   Move strings + type + delay into a new TimingArc, push into arcs_.
    void TimingGraph::add_timing_arc(TimingArc arc){
        arcs_.push_back(arc);
    }

    //finalize() builds the timing graph's internal data structures for propagation.
    //我默认经过电路读取之后，timinggraph的所有矢量私有成员cells_、nets_、arcs_的pin_name都是合法的qualified name，形如"U1/A"。
    void TimingGraph::finalize(void){
        //1. Clear pin_map_, fanout_, fanin_
        pin_map_.clear();
        fanout_.clear();
        fanin_.clear();
        // 2. For each cell, for each pin:
        //     qualified_name = cell.name + "/" + pin.name
        //     pin_map_[qualified_name] = &pin
        //     reset pin.arrival_time and pin.req_time
        string qualified_name;
        for (auto& cell : cells_) {
            for (auto& pin : cell.Pins) {
                qualified_name = cell.name + "/" + pin.name;
                pin_map_[qualified_name] = &pin;
                pin.arrival_time = nullopt;
                pin.req_time = nullopt;
            }
        }
        // 3. For each arc:
        // fanout_[from_pin].push_back({to_pin, delay})
        // fanin_[to_pin].push_back({from_pin, delay})
        for (const auto& arc : arcs_){
            fanout_[arc.from_pin].push_back({arc.to_pin, arc.delay});
            fanin_[arc.to_pin].push_back({arc.from_pin, arc.delay});
        }
        // 4. For each net:
        for(const auto& net : nets_){
            for(const auto& to_pin : net.to_pins){
                fanout_[net.from_pin].push_back({to_pin, 0.0f});
                fanin_[to_pin].push_back({net.from_pin, 0.0f});
            }
        }                    
    }

//   Phase 3: use std::make_unique<Cell>, push into cells_,
//            return Cell& (deref the unique_ptr).

//   connections vector will be filled during finalize().
// finalize():
 
    //Find the name in pin_map_, return the Pin* (or nullptr).
    Pin* TimingGraph::lookup_pin(const string& name){
        auto it = pin_map_.find(name);
        return it != pin_map_.end() ? it->second : nullptr;
    }
    const Pin* TimingGraph::lookup_pin(const string& name) const{
        auto it = pin_map_.find(name);
        return it != pin_map_.end() ? it->second : nullptr;
    }

    //Look up in the map, return the vector (or empty vector if not found). Use a static empty vector for the not-found case to avoid returning nullptr.
    const vector<TimingGraph::TimingEdge>& TimingGraph::fanout(const string& pin) const{
        static const vector<TimingGraph::TimingEdge> empty;
        auto it = fanout_.find(pin);
        return it != fanout_.end() ? it->second : empty;
    }

    const vector<TimingGraph::TimingEdge>& TimingGraph::fanin(const string& pin) const{
        static const vector<TimingGraph::TimingEdge> empty;
        auto it = fanin_.find(pin);
        return it != fanin_.end() ? it->second : empty;
    }


} // namespace tsta
