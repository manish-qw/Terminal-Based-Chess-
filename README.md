# Terminal Chess AI

A command-line chess engine written in C++17. Plays a complete game of chess with an AI opponent
that uses **iterative-deepening alpha-beta minimax** with **MVV-LVA move ordering**.

```
8 r n b q k b n r
7 p p p p p p p p
6 . . . . . . . .
5 . . . . . . . .
4 . . . . P . . .
3 . . . . . . . .
2 P P P P . P P P
1 R N B Q K B N R
  a b c d e f g h

AI is thinking... (depth 6, 0.91s)
AI played: d7 to d5
```

## Features

| Feature | Detail |
|---|---|
| Move generation | All legal moves: castling, en passant, promotion |
| Search | Alpha-beta minimax with iterative deepening |
| Move ordering | MVV-LVA (captures sorted best-victim/worst-attacker first) |
| Evaluation | Material + piece-square tables + mobility + pawn structure |
| Time control | Configurable time budget (default 2 s) |
| Correctness | Perft test suite validates the move generator |
| CI | GitHub Actions builds and runs perft on every push |

## Build

```bash
git clone https://github.com/manish-qw/Terminal-Based-Chess-.git
cd Terminal-Based-Chess-
make
```

Requires GCC/Clang with C++17 support. No external dependencies.

## Run

```bash
./chess           # play a game (you are White)
./chess --perft 4 # verify move generator: must print 197281
```

### Move format

```
e2 e4          # regular move
e7 e8 Q        # pawn promotion (Q / R / B / N)
quit           # exit
```

## How the AI works

### Iterative Deepening
The engine searches depth 1, then 2, then 3 … until the time budget expires.
It always has a best move ready, so it never "thinks forever".

### MVV-LVA Move Ordering
Captures are scored by `10 × victim_value − attacker_value`.
This means `PxQ` (+49) is tried before `QxP` (-4), so alpha-beta prunes far more branches.

### Perft
`perft(N)` counts every legal leaf node at depth N from a position.
The start position has exactly **20 / 400 / 8 902 / 197 281 / 4 865 609** nodes at depths 1–5.
If your move generator is wrong (bad castling, en passant, check legality), perft will catch it.

## Project structure

```
piece.h          — Piece, Move, GameState structs
board.h/cpp      — Board, move generation, evaluation
chess_ai.h/cpp   — Minimax, MVV-LVA ordering, iterative deepening, perft
game.h/cpp       — Game loop, user input
benchmark.h/cpp  — Depth/time benchmarks
main.cpp         — Entry point
Makefile         — Build
.github/         — CI
```

## Perft reference values (start position)

| Depth | Nodes |
|-------|-------|
| 1 | 20 |
| 2 | 400 |
| 3 | 8,902 |
| 4 | 197,281 |
| 5 | 4,865,609 |
