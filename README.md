# Terminal-Based Chess AI

A command-line chess game with an intelligent AI opponent powered by the **minimax algorithm with alpha-beta pruning**. Play against a strategic AI or benchmark its performance with detailed analytics.

## Features

- **Interactive Chess Gameplay** – Play as White against the AI opponent (Black) in real-time
- **Advanced AI Engine** – Minimax algorithm with alpha-beta pruning for optimal move selection
- **Sophisticated Position Evaluation** – Material counting, piece-square tables, mobility analysis, and pawn structure assessment
- **Complete Move Support** – All legal moves including castling, en passant, and pawn promotion
- **Comprehensive Rule Implementation** – Check, checkmate, stalemate, and draw detection
- **Performance Benchmarking** – Analyze AI efficiency at different search depths with timing and node count metrics
- **Cross-Platform** – Runs on Windows, Linux, and macOS

## Quick Start

### Prerequisites

- **C++ Compiler** – GCC, Clang, or MSVC (C++11 or later)
- **Make** (optional) – for convenient build
- **Git** – to clone the repository

### Installation & Setup

1. **Clone the repository:**
   ```bash
   git clone https://github.com/manish-qw/Terminal-Based-Chess-.git
   cd Terminal-Based-Chess-
   ```

2. **Compile the project:**
   ```bash
   g++ -std=c++11 -O2 main.cpp board.cpp chess_ai.cpp game.cpp benchmark.cpp -o chess
   ```

3. **Run the game:**
   ```bash
   ./chess
   ```

## How to Play

### Game Basics

1. **Start the game** by running the executable
2. **You play as White** (first move)
3. **AI plays as Black** (responds to your moves)
4. Enter moves in **algebraic notation** (e.g., `e2e4`)

### Move Format

- **Regular Move:** `e2e4` (from square to square)
- **Pawn Promotion:** `e7e8Q` (promotes to Queen; use Q, R, B, or N)
- **Case-Insensitive:** Both lowercase and uppercase are accepted

### Game Display

The board is displayed using standard chess notation:
- Columns: `a-h` (left to right)
- Rows: `1-8` (bottom to top for White's perspective)

### Game Outcomes

- **Checkmate** – Game ends with winner declared
- **Stalemate** – Draw condition when no legal moves available
- **Insufficient Material** – Draw due to lack of pieces
- **Fifty-Move Rule** – Draw after 50 moves without capture/pawn move

### Benchmark Mode

Test the AI's search efficiency and performance:

1. **Uncomment benchmark in main.cpp:**
   ```cpp
   int main() {
       benchmark();  // Uncomment this line
       // Rest of main...
   }
   ```

2. **Recompile and run:**
   ```bash
   g++ -std=c++11 -O2 main.cpp board.cpp chess_ai.cpp game.cpp benchmark.cpp -o chess
   ./chess
   ```

3. **Output includes:**
   - Average time per move at each depth
   - Average nodes explored per move
   - Nodes per second (search speed)
   - Performance metrics for depths 1-4

## Project Structure

| File | Purpose |
|------|---------|
| `piece.h` | Core data structures: Piece, Move, GameState |
| `board.h` / `board.cpp` | Board management, move generation, position evaluation |
| `chess_ai.h` / `chess_ai.cpp` | AI engine: minimax algorithm with alpha-beta pruning |
| `game.h` / `game.cpp` | Game loop, user interaction, move validation |
| `benchmark.h` / `benchmark.cpp` | Performance testing and metrics |
| `main.cpp` | Program entry point |

## 🔧 Architecture & Algorithm

### Minimax with Alpha-Beta Pruning

The AI evaluates moves using a depth-limited minimax algorithm enhanced with alpha-beta pruning to reduce computational overhead:

- **Minimax**: Recursively evaluates all possible moves to a given depth
- **Alpha-Beta Pruning**: Eliminates branches that won't affect the final decision
- **Configurable Depth**: Adjustable search depth (higher = stronger but slower)

### Position Evaluation

The evaluation function considers multiple factors:
- **Material Value** – Numerical worth of each piece
- **Piece-Square Tables** – Positional bonuses based on piece type and location
- **Piece Mobility** – Number of available moves
- **Pawn Structure** – Pawns' formation and advancement

## Contributing

Contributions are welcome! To contribute:

1. **Fork** the repository
2. **Create a feature branch**: `git checkout -b feature/your-feature`
3. **Commit changes**: `git commit -m "Add descriptive message"`
4. **Push to branch**: `git push origin feature/your-feature`
5. **Open a Pull Request** with details about your changes

### Reporting Issues

Found a bug or have a suggestion? Please open an [Issue](https://github.com/manish-qw/Terminal-Based-Chess-/issues) with:
- Clear description of the problem
- Steps to reproduce (if applicable)
- Expected vs actual behavior

## Acknowledgments

- Chess programming fundamentals inspired by classic sources and community tutorials
- Built with pure C++ Standard Library for maximum portability
- Thanks to the open-source community for guidance and inspiration

---

**Questions or feedback?** Open an issue or reach out. Happy chess playing!
