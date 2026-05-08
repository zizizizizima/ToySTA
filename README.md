# ToySTA — Toy Static Timing Analysis Engine

maybe this toy project can be helpful to students who major in EDA who researches within the scope of physical synthesis algorithm development of digital IC.

## Project Structure

```
ToySTA/
├── CMakeLists.txt          (C++17, CMake)
├── .gitignore
└── src/
    ├── main.cpp            (test circuit + assertions)
    └── sta/
        ├── types.h          (Cell, Pin, Net, TimingArc)
        ├── timing_graph.h/cpp (graph builder + adjacency lists)
        └── timing_engine.h/cpp (propagation, reporting, critical path)
```

## Build

```bash
cmake -B build
cmake --build build
./build/ToySTA.exe
```

Or directly with g++:

```bash
g++ -std=c++17 -Isrc src/main.cpp src/sta/timing_graph.cpp src/sta/timing_engine.cpp -o ToySTA
```

## Phases

### Phase 0 — Project Scaffolding
- `CMakeLists.txt` with C++17
- Directory structure: `src/` for sources, `src/sta/` for library code
- Out-of-source build via `cmake -B build` (generated files go into `build/`, source stays clean)

**New C++ concepts:** CMake, namespaces, project organization.

### Phase 1 — Core Data Types + Timing Graph
- `enum class PinDir { Input, Output }`, `enum class ArcType { Combinational, Setup, Hold }`
- `Pin`, `Cell`, `Net`, `TimingArc` structs
- `TimingGraph` owns cells/nets/arcs in flat `std::vector`s
- `std::unordered_map<std::string, Pin*>` for name-to-pin lookup (qualified name: `"U1/A"`)
- `add_cell()`, `add_net()`, `add_timing_arc()`, `finalize()`, `lookup_pin()`
- `finalize()` resolves pin pointers and builds timing propagation graph adjacency lists

**New C++ concepts:** `enum class`, `std::vector`, `std::unordered_map`, `std::string`, `const` correctness.

### Phase 2 — Timing Propagation (AT, RAT, Slack)
- `mutable std::optional<float> arrival_time` and `req_time` on `Pin`
  - `mutable` because propagation modifies logically-const state
  - `std::optional` elegantly expresses "not yet computed"
- Forward propagation: AT(startpoint) = 0, AT(output) = max(AT(input) + arc_delay)
- Backward propagation: RAT(endpoint) = clock_period, RAT(input) = min(RAT(output) - arc_delay)
- Slack = RAT - AT (negative = setup violation)

**New C++ concepts:** `std::optional`, `mutable`, range-based for loops.

### Phase 3 — Smart Pointers + Unique Ownership
- Refactored `std::vector<Cell>` → `std::vector<std::unique_ptr<Cell>>`
- `add_cell()` now returns `Cell&` (factory pattern)
- `TimingGraph` becomes move-only (implicitly, because `unique_ptr` is not copyable)
- No functionality change — ownership model improvement only

**New C++ concepts:** `std::unique_ptr`, factory functions, move semantics.

### Phase 4 — Hardcoded Test Circuit + Reporting
- Multi-cell circuit with diverging/reconverging paths (U1→U2/U3→U4)
- Uses `std::sort` + lambda to sort pins by slack ascending (worst first)
- `trace_critical_path()`: walks backward from an endpoint, picking the predecessor with minimum slack at each step
- Prints top-5 worst slack pins and a detailed sorted report

**New C++ concepts:** `<algorithm>` (`std::sort`, `std::reverse`, `std::min`, `std::find`), lambdas, structured bindings.

## Key Design Decisions

### Worklist-based propagation (not DFS with visited set)
Initial implementation used DFS with a visited set, which is **wrong for reconvergent paths**: when a node is reachable from multiple paths, the first path to visit it marks it as visited and the later path is skipped. Fixed with a worklist (Bellman-Ford-style relaxation): update a node's value whenever a better (later AT / earlier RAT) is found, and push its neighbors back into the worklist.

### Pin lookup by qualified string name
All pins are identified by `"CellName/PinName"` strings (e.g. `"U1/A"`, `"U3/Y"`). Timing arcs and nets both reference pins by name. `finalize()` resolves these strings to `Pin*` pointers and builds adjacency lists for fast traversal during timing propagation.

### Timing propagation graph
Two adjacency lists built from arcs + nets:
- Timing arcs create edges `from_pin → to_pin` with the arc's delay
- Nets create edges `driver_pin (output) → load_pin (input)` with delay 0 (ideal wire)

## How Inputs & Outputs Map to Code

```
Netlist (cells + wires)   ──→  timing_graph.*
Cell delays (.lib)        ──→  types.h (TimingArc holds delay numbers)
SDCs (clock, constraints) ──→  timing_engine.* (clock_period param, RAT)

Arrival time report        ←──  timing_engine.cpp (propagate_arrival_time)
Required time report       ←──  timing_engine.cpp (propagate_required_time)
Critical path list         ←──  timing_engine.cpp (trace_critical_path)
```

### What each file does

| File | Role |
|------|------|
| `types.h` | **Vocabulary** — what a Cell, Pin, Net, and TimingArc *are*. No logic, just data. |
| `timing_graph.h/.cpp` | **Netlist database** — you feed in cells/nets/arcs, it builds a query-able graph with pin lookup and adjacency lists. `finalize()` compiles the netlist. |
| `timing_engine.h/.cpp` | **Analysis engine** — forward sweep (AT), backward sweep (RAT), critical path tracing. This is where the "actual STA" happens. |
| `main.cpp` | **Testbench** — constructs a hardcoded circuit, drives the flow, prints the report. |

### Data flow

```
main.cpp ──feeds──→ timing_graph.* ──compiled──→ timing_engine.* ──report──→ main.cpp prints it
  (netlist,          (builds graph     (computes AT/RAT,
   delays)            + adj lists)      slack, crit path)
```
