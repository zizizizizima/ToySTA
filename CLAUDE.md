# Collaboration Approach (in Terms of Claude)
Assist the user by modifying hints, comments, or offering design direction, rather than writing code directly.
# ToySTA: Toy Static Timing Analysis

## Project Goal

A minimal but functional STA engine for learning and experimentation.

## Design Decisions

### 1. Build System

CMake-based. Single executable targets.

### 2. Custom Circuit Format (`.cir`)

The user interacts with ToySTA via a **custom plain-text circuit file**, not a netlist parser.

```
# File: my_circuit.cir
CELL U1 (A, B, >Y)
ARC U1/A -> U1/Y 0.3
ARC U1/B -> U1/Y 0.3
CELL U2 (A, B, >Y)
ARC U2/A -> U2/Y 0.5
ARC U2/B -> U2/Y 0.5
NET N1 (U1/Y U2/A)
CLOCK_PERIOD 2.0
```

This avoids the complexity of Verilog parsing while keeping ToySTA a real CLI tool:

```
ToySTA my_circuit.cir
```

#### Format Reference

| Directive | Syntax | Description |
|-----------|--------|-------------|
| `CELL` | `CELL <name> (<pins...>)` | Declares a cell with pins. Output pins prefixed by `>`. |
| `ARC` | `ARC <cell/pin> -> <cell/pin> <delay>` | Timing arc within a cell. |
| `NET` | `NET <name> (<cell/pin ...>)` | Wire connecting pins. |
| `CLOCK_PERIOD` | `CLOCK_PERIOD <value>` | Clock period for required-time propagation. |
| `#` | `# comment` | Comment line (ignored). |

The `CircuitReader` class parses this format and calls `TimingGraph::add_cell()` / `add_net()` / `add_timing_arc()`.

### 3. Phases

- **Phase 1** — Core data types + graph construction (done)
- **Phase 2** — Timing propagation engine (AT/RAT/slack) (WIP)
- **Phase 3** — Smart-pointer refactor (cells_ → vector<unique_ptr<Cell>>)
- **Phase 4** — Critical path tracing + reporting
- **Phase 5** — `CircuitReader` (custom `.cir` format) + CLI entry point
