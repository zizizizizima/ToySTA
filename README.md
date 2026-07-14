# ToySTA — Toy Static Timing Analysis Engine

A minimal STA engine for learning EDA physical synthesis concepts. Reads a custom `.cir` circuit format and computes arrival time (AT), required time (RAT), slack, and critical paths.

## Quick Start

```bash
cmake -B build
cmake --build build
./build/ToySTA.exe test/example.cir
```

## .cir Format

```
CELL <name> (<input_pins..., >output_pins...)
ARC <cell/pin> -> <cell/pin> <delay>
NET <name> (<driver_pin load_pin...>)
CLOCK_PERIOD <value>
# comments start with #
```

Example (`test/example.cir`):

```
CELL FF1 (CK, D, >Q)
ARC FF1/CK -> FF1/Q 0.3
CELL INV1 (A, >Y)
ARC INV1/A -> INV1/Y 0.5
CELL OR2 (A, B, >Y)
ARC OR2/A -> OR2/Y 0.4
ARC OR2/B -> OR2/Y 0.4
CELL FF2 (CK, D, >Q)
ARC FF2/CK -> FF2/Q 0.3
NET N1 (FF1/Q INV1/A)
NET N2 (INV1/Y OR2/B)
NET N3 (OR2/Y FF2/D)
CLOCK_PERIOD 5.0
```

## Architecture

```
src/
├── main.cpp                    CLI entry point (reads .cir → runs analysis → prints report)
└── sta/
    ├── types.h                 Data types: Cell, Pin, Net, TimingArc
    ├── timing_graph.h/.cpp     Netlist database — build graph, lookup pins, traverse fanin/fanout
    ├── timing_engine.h/.cpp    Timing engine — AT/RAT propagation, slack, critical path tracing
    └── circuit_reader.h/.cpp   .cir file parser
```

## Data Flow

```
 .cir file ──→ CircuitReader ──→ TimingGraph ──→ finalize() ──→ TimingEngine ──→ print_report()
              (parse text)      (cells/nets/    (adjacency      (AT/RAT/slack/   (stdout)
                                 arcs)           lists)          crit path)
```

## Phases

| Phase | What | Status |
|-------|------|--------|
| 1 | Data types + graph construction | ✅ |
| 2 | AT/RAT propagation (worklist-based) | ✅ |
| 3 | Smart-pointer refactor (`unique_ptr<Cell>`) | ⏳ |
| 4 | Critical path tracing + timing report | ✅ |
| 5 | `.cir` parser + CLI entry point | ✅ |

## Key Design Points

- **Worklist propagation** (not DFS): handles reconvergent paths correctly by re-relaxing nodes when a better AT/RAT is found.
- **Pin-by-name lookup**: qualified names like `U1/A` are resolved to `Pin*` pointers during `finalize()`.
- **Ideal wires**: nets have zero delay (no RC extraction).
