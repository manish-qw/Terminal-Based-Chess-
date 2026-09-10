#ifndef CHESS_AI_H
#define CHESS_AI_H

#include "board.h"
#include <limits>
#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

// ── Transposition table ──────────────────────────────────────────────────────

enum TTFlag { TT_EXACT, TT_LOWER, TT_UPPER };

struct TTEntry {
    uint64_t hash;
    int      depth;
    int      score;
    TTFlag   flag;
    Move     bestMove;

    TTEntry() : hash(0), depth(-1), score(0), flag(TT_EXACT), bestMove(0,0,0,0) {}
};

// ── Zobrist hashing ──────────────────────────────────────────────────────────

struct Zobrist {
    uint64_t pieceKeys[2][7][8][8]; // [color][type][row][col]
    uint64_t sideKey;               // flip when it's Black's turn
    uint64_t castleKeys[4];         // WK, WQ, BK, BQ
    uint64_t enPassantKeys[8];      // file 0-7

    void init();
    uint64_t computeHash(const Board& board, Color sideToMove) const;
};

// ── Chess AI ─────────────────────────────────────────────────────────────────

class ChessAI {
public:
    long long nodesExplored = 0;

    ChessAI();

    // Time-based iterative deepening (timeLimitMs = 0 → use fixedDepth)
    Move getBestMove(Board& board, Color aiColor,
                     int timeLimitMs = 2000, int fixedDepth = 0);

    // Perft: count leaf nodes at depth N (used to validate move generator)
    uint64_t perft(Board& board, Color color, int depth);

private:
    // Iterative deepening helpers
    std::chrono::steady_clock::time_point searchStart;
    int  timeLimitMs = 2000;
    bool stopSearch  = false;

    // Transposition table on heap (1M entries × ~28 bytes = ~28 MB — use 64k to keep it lean)
    static constexpr int TT_SIZE = 65536;
    std::unique_ptr<TTEntry[]> tt;

    // Zobrist
    Zobrist zobrist;
    bool    zobristReady = false;

    // Move ordering — MVV-LVA score for a capture
    // Returns high positive for PxQ, low for QxP
    int mvvLva(PieceType victim, PieceType attacker) const;

    // Sort moves in-place: TT best-move first, then captures by MVV-LVA, then quiet
    void orderMoves(std::vector<Move>& moves, const Board& board,
                    const Move& ttBest) const;

    // Negamax with alpha-beta, TT lookup/store
    int negamax(Board& board, int depth, int alpha, int beta,
                Color currentTurn, uint64_t hash);

    bool timeUp() const;
};

#endif // CHESS_AI_H