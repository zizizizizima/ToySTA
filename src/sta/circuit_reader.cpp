#include "sta/circuit_reader.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace tsta {

// ===================================================================
//  read_file()  —  top-level dispatch
// ===================================================================
//
// Your job: open the file, read it line by line, and dispatch each
// non-empty, non-comment line to the correct parser.
//
// Step-by-step:
//   1. Open an std::ifstream for `filename`
//   2. If it fails to open, print an error and return 0.0f
//   3. Declare float clock_period = 0.0f to accumulate the result
//   4. Loop: read each line with std::getline()
//   5. For each line:
//      a) Trim it (remove leading/trailing whitespace)
//      b) Skip if empty:   if (trimmed.empty()) continue;
//      c) Skip comments:   if (trimmed[0] == '#') continue;
//      d) Check the first token (up to first space) with a chain of
//         if/else-if using trimmed.compare(0, len, "CELL") == 0 etc.
//      e) Dispatch to the right parser:
//           "CELL"         -> parse_cell(trimmed, tg)
//           "ARC"          -> parse_arc(trimmed, tg)
//           "NET"          -> parse_net(trimmed, tg)
//           "CLOCK_PERIOD" -> clock_period = parse_clock_period(trimmed)
//           anything else  -> print a warning and ignore
//   6. Return clock_period
//
float CircuitReader::read_file(const std::string& filename, TimingGraph& tg) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: could not open file: " << filename << '\n';
        return 0.0f;
    }

    float clock_period = 0.0f;
    std::string line;
    while (std::getline(file, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#')
            continue;

        if (trimmed.compare(0, 4, "CELL") == 0)
            parse_cell(trimmed, tg);
        else if (trimmed.compare(0, 3, "ARC") == 0)
            parse_arc(trimmed, tg);
        else if (trimmed.compare(0, 3, "NET") == 0)
            parse_net(trimmed, tg);
        else if (trimmed.compare(0, 12, "CLOCK_PERIOD") == 0)
            clock_period = parse_clock_period(trimmed);
        else
            std::cerr << "Warning: unrecognized directive: " << trimmed << '\n';
    }
    return clock_period;
}

// ===================================================================
//  parse_cell()  —  CELL <name> (<pin1>, <pin2>, ..., <pinN>)
// ===================================================================
//
// Example line:  "CELL U1 (A, B, >Y)"
//
// Step-by-step:
//   1. Strip "CELL " from the beginning (line.substr(5))
//   2. Find the '(' — everything before it is the cell name (trim it)
//   3. Extract the parenthesized part with strip_parens()
//      e.g. strip_parens("(A, B, >Y)") → "A, B, >Y"
//   4. Split that by ',' to get individual pin tokens: ["A", " B", " >Y"]
//   5. For each token:
//      a) Trim it
//      b) If it starts with '>', strip the '>' and set dir = PinDir::Output
//         Otherwise, dir = PinDir::Input
//      c) Create a Pin{name, dir} and push it into a local vector
//   6. Build Cell{cell_name, pins_vector} and call tg.add_cell(cell)
//
void CircuitReader::parse_cell(const std::string& line, TimingGraph& tg) {
    std::string body = line.substr(5);   // skip "CELL "
    auto paren_pos = body.find('(');
    std::string cell_name = trim(body.substr(0, paren_pos));
    std::string pin_list = strip_parens(body.substr(paren_pos));
    auto tokens = split(pin_list, ',');
    std::vector<Pin> pins;
    for (auto& tok : tokens) {
        tok = trim(tok);
        PinDir dir = PinDir::Input;
        if (tok[0] == '>') { dir = PinDir::Output; tok = tok.substr(1); }
        pins.push_back({tok, dir});
    }
    tg.add_cell({cell_name, pins});
}

// ===================================================================
//  parse_arc()  —  ARC <from_pin> -> <to_pin> <delay>
// ===================================================================
//
// Example line:  "ARC U1/A -> U1/Y 0.3"
//
// Step-by-step:
//   1. Strip "ARC " from the beginning (line.substr(4))
//   2. Find the " -> " separator (string::find(" -> "))
//   3. Left part (before " -> ") is from_pin — trim it
//   4. Right part (after " -> ") contains "to_pin delay"
//   5. Right part is space-separated: split by ' '
//   6. First token (index 0) is to_pin, second token (index 1) is delay_str
//   7. Convert delay_str to float with std::stof()
//   8. ArcType: use ArcType::Combinational for all arcs (simplification)
//   9. Build TimingArc{from_pin, to_pin, type, delay}
//  10. Call tg.add_timing_arc(arc)
//
// Edge cases to think about:
//   - What if " -> " isn't found?  (malformed line — skip with a warning)
//   - What if the delay string isn't a valid float?  (std::stof throws)
//     -> You can wrap it in try/catch or just let it crash for now
//
void CircuitReader::parse_arc(const std::string& line, TimingGraph& tg) {
    std::string body = line.substr(4);   // skip "ARC "
    auto arrow = body.find(" -> ");
    std::string from_pin = trim(body.substr(0, arrow));
    std::string rest = trim(body.substr(arrow + 4));
    auto parts = split(rest, ' ');
    std::string to_pin = parts[0];
    float delay = std::stof(parts[1]);
    tg.add_timing_arc({from_pin, to_pin, ArcType::Combinational, delay});
}

// ===================================================================
//  parse_net()  —  NET <name> (<driver> <load1> <load2> ...)
// ===================================================================
//
// Example line:  "NET N1 (U1/Y U2/A)"
//                        ^^^  ^^^^^^^^^^^^^
//                       name   driver  loads
//
// Step-by-step:
//   1. Strip "NET " from the beginning (line.substr(4))
//   2. Find the '(' — everything before it is the net name (trim it)
//   3. Extract the parenthesized part with strip_parens()
//      e.g. strip_parens("(U1/Y U2/A)") → "U1/Y U2/A"
//   4. Split by ' '  (space, NOT comma) to get individual pin names
//   5. The first token is the driver (an Output pin); the rest are loads.
//      Build Net{net_name, driver, loads_vector}. The qualified names are
//      later resolved to Pin* by TimingGraph::finalize() via pin_map_.
//   6. Call tg.add_net(net)
//
// Note: Net pins are space-separated (CELL pins are comma-separated).
// Different delimiters for different purposes!
//
void CircuitReader::parse_net(const std::string& line, TimingGraph& tg) {
    std::string body = line.substr(4);   // skip "NET "
    auto paren_pos = body.find('(');
    std::string net_name = trim(body.substr(0, paren_pos));
    std::string pin_list = strip_parens(body.substr(paren_pos));
    auto tokens = split(pin_list, ' ');
    std::string driver = tokens[0];
    std::vector<std::string> loads(tokens.begin() + 1, tokens.end());
    tg.add_net({net_name, driver, loads});
}

// ===================================================================
//  parse_clock_period()  —  CLOCK_PERIOD <value>
// ===================================================================
//
// Example line:  "CLOCK_PERIOD 2.0"
//
// Step-by-step:
//   1. Strip "CLOCK_PERIOD " from the beginning (line.substr(13))
//   2. Trim the result
//   3. Convert to float with std::stof()
//   4. Return the float
//
float CircuitReader::parse_clock_period(const std::string& line) {
    std::string body = line.substr(13);  // skip "CLOCK_PERIOD "
    return std::stof(trim(body));
}

// ===================================================================
//  Helper: trim()
// ===================================================================
//
// Remove leading and trailing whitespace.
//
// Hints:
//   1. Find the first non-whitespace character (str.find_first_not_of(" \t\r\n"))
//   2. If none found (npos), return empty string
//   3. Find the last non-whitespace character (str.find_last_not_of(" \t\r\n"))
//   4. Return str.substr(start, end - start + 1)
//
// For both find_first_not_of and find_last_not_of, if no such character
// is found, they return string::npos — handle that edge case!
//
std::string CircuitReader::trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// ===================================================================
//  Helper: strip_parens()
// ===================================================================
//
// If the string starts with '(' and ends with ')', strip them.
// Otherwise return the string as-is.
//
// Hints:
//   1. Check if str[0] == '(' and str[str.size()-1] == ')'
//   2. If both true, return str.substr(1, str.size() - 2)
//   3. Otherwise return str
//
// Edge case: what if the string is just "()" — empty parens?
//   str.substr(1, 0) is fine, returns "".
//
std::string CircuitReader::strip_parens(const std::string& str) {
    if (str.size() >= 2 && str[0] == '(' && str[str.size() - 1] == ')')
        return str.substr(1, str.size() - 2);
    return str;
}

// ===================================================================
//  Helper: split()
// ===================================================================
//
// Split a string by a delimiter character and return a vector of tokens.
//
// Hints:
//   1. Create an empty vector<string> result
//   2. Use std::stringstream or a manual loop to find delimiters
//   3. For each token, trim it and push it into result
//   4. Return result
//
// With std::stringstream (works great for space-delimited):
//     istringstream stream(str);
//     string token;
//     while (getline(stream, token, delimiter))
//         result.push_back(trim(token));
//
// Manual approach (works for any delimiter):
//     size_t start = 0, end;
//     while ((end = str.find(delimiter, start)) != string::npos) {
//         result.push_back(trim(str.substr(start, end - start)));
//         start = end + 1;
//     }
//     result.push_back(trim(str.substr(start)));
//
// Edge case: if delimiter is ' ' (space), consecutive spaces produce
// empty tokens. After trimming, empty tokens should be skipped.
// Add a check:  if (token.empty()) continue;
//
std::vector<std::string> CircuitReader::split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    size_t start = 0, end;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        std::string token = trim(str.substr(start, end - start));
        if (!token.empty())
            result.push_back(token);
        start = end + 1;
    }
    std::string token = trim(str.substr(start));
    if (!token.empty())
        result.push_back(token);
    return result;
}

// ===================================================================
//  Some thoughts before you start coding:
//
//  1. Order of implementation (easiest first):
//       trim() → strip_parens() → split() → parse_clock_period()
//       → parse_cell() → parse_net() → parse_arc() → read_file()
//
//  2. After all parsers work, update main.cpp to:
//       int main(int argc, char* argv[]) {
//           if (argc < 2) { cerr << "Usage: ToySTA <file.cir>\n"; return 1; }
//           TimingGraph tg;
//           float period = CircuitReader::read_file(argv[1], tg);
//           tg.finalize();
//           // ... propagation ...
//       }
//
//  3. Write a small .cir test file (e.g. test/circuit1.cir) and run it
//     to verify parsing works. Look at main.cpp's Phase 1 comment for
//     the canonical test case.
//
//  4. Things that can go wrong:
//      - Extra spaces or tabs between tokens (trim() handles it)
//      - Empty lines at end of file (skip in read_file())
//      - Missing parentheses (strip_parens returns as-is → weird splits)
//        → For a toy, a warning is enough, you don't need full error recovery
//      - Missing file (read_file checks ifstream and returns 0)
//      - Negative delay values? That's unusual for arcs but not illegal
//        → stof handles negative numbers fine
//      - CLOCK_PERIOD = 0? Not useful but not a crash.
//        → 0.0f is the default return, so it degrades gracefully
// ===================================================================

} // namespace tsta
