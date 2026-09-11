# Terminal-Chess

A high-performance command-line chess engine written in C++17. The engine plays a complete game of chess and communicates via the standard UCI (Universal Chess Interface) protocol, making it compatible with popular GUIs like Arena and CuteChess.

## Features

- **Board Representation**: Highly optimized 64-bit Bitboards.
- **Move Generation**: Fast magic-less bitboard move generation that passes the complete PERFT test suite.
- **Search Algorithm**: Alpha-beta minimax with Principal Variation Search (PVS) and Iterative Deepening.
- **Transposition Table**: 134MB Zobrist-hashed transposition table for caching identical branches and detecting repetitions.
- **Heuristics**:
  - **Null Move Pruning**: Aggressive cutoffs in non-tactical positions.
  - **Late Move Reductions (LMR)**: Reduced search depth for unpromising moves.
  - **Killer Heuristic**: Prioritizes moves that recently caused cutoffs in sibling nodes.
  - **MVV-LVA**: "Most Valuable Victim - Least Valuable Attacker" move ordering for highly efficient tactical capture resolution.

## Build Instructions

To compile the engine from source, simply use the provided `Makefile` on Linux, or the `compile.bat` script on Windows.
Requires `g++` with C++17 support.

**Linux / macOS:**
```bash
make
./Terminal-Chess bench
```

**Windows:**
```bat
.\compile.bat
.\Terminal-Chess.exe bench
```

## Usage

You can launch the engine in different modes:

- `./Terminal-Chess uci` : Start in Universal Chess Interface mode (default if no args passed).
- `./Terminal-Chess bench` : Run the internal performance benchmarking suite.
- `./Terminal-Chess perft` : Run the move-generation correctness test suite.
- `./Terminal-Chess play` : Play an interactive console game against the engine.

## Performance Benchmarks

*Hardware: Standard Desktop CPU. Flags: `-O3 -march=native -flto`*

### Search Efficiency (Depth 4)
The engine searches complex positions rapidly, averaging over **2.1 Million Nodes Per Second (NPS)**.

| Position | Depth | Nodes | NPS |
|----------|-------|-------|-----|
| Start Position | 4 | 4,247 | ~1.44M |
| Kiwipete | 4 | 42,417 | ~2.49M |
| Middlegame | 4 | 33,231 | ~1.99M |
| Endgame | 4 | 3,709 | ~1.95M |
| **Average** | | | **~2.17M** |

### Move Generation (PERFT)
The move generator is strictly validated against known PERFT values.

| FEN | Depth | Nodes | Result | Time |
|-----|-------|-------|--------|------|
| Startpos | 4 | 197,281 | PASS | 16ms |
| Kiwipete | 3 | 97,862 | PASS | 9ms |
| Endgame | 4 | 43,238 | PASS | 4ms |

## Telemetry
Based on 83,000 nodes searched across test positions:
- **Killer Hits**: 890
- **Null Cutoffs**: 27
- **LMR Reductions**: 160
- **Beta Cutoffs**: 3,763 (82% First-Move Cutoff rate)
