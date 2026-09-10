#include "chess_ai.h"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <random>
#include <memory>

// ── Constructor ──────────────────────────────────────────────────────────────────
ChessAI::ChessAI() : tt(std::make_unique<TTEntry[]>(TT_SIZE)) {}

// ── Zobrist implementation ───────────────────────────────────────────────────

void Zobrist::init() {
    std::mt19937_64 rng(0xDEADBEEFCAFEBABEULL); // fixed seed → reproducible
    for (int c = 0; c < 2; c++)
        for (int t = 0; t < 7; t++)
            for (int r = 0; r < 8; r++)
                for (int f = 0; f < 8; f++)
                    pieceKeys[c][t][r][f] = rng();
    sideKey = rng();
    for (auto& k : castleKeys)   k = rng();
    for (auto& k : enPassantKeys) k = rng();
}

uint64_t Zobrist::computeHash(const Board& board, Color sideToMove) const {
    uint64_t h = 0;
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            Piece p = board.getPiece(r, f);
            if (p.type != EMPTY)
                h ^= pieceKeys[p.color][p.type][r][f];
        }
    }
    if (sideToMove == BLACK) h ^= sideKey;
    if (board.gameState.whiteCanCastleKingside)  h ^= castleKeys[0];
    if (board.gameState.whiteCanCastleQueenside) h ^= castleKeys[1];
    if (board.gameState.blackCanCastleKingside)  h ^= castleKeys[2];
    if (board.gameState.blackCanCastleQueenside) h ^= castleKeys[3];
    if (board.gameState.hasEnPassant)
        h ^= enPassantKeys[board.gameState.enPassantY];
    return h;
}

// ── MVV-LVA ─────────────────────────────────────────────────────────────────
// Piece values used only for ordering (not evaluation)
// EMPTY=0, PAWN=1, KNIGHT=2, BISHOP=3, ROOK=4, QUEEN=5, KING=6
// Score = 10 * victim - attacker → high = try first

int ChessAI::mvvLva(PieceType victim, PieceType attacker) const {
    return 10 * static_cast<int>(victim) - static_cast<int>(attacker);
}

void ChessAI::orderMoves(std::vector<Move>& moves, const Board& board,
                          const Move& ttBest) const {
    // Score every move, then stable-sort descending
    auto score = [&](const Move& m) -> int {
        // TT best move: always first
        if (m.fromX == ttBest.fromX && m.fromY == ttBest.fromY &&
            m.toX   == ttBest.toX   && m.toY   == ttBest.toY   &&
            m.promotion == ttBest.promotion)
            return 2'000'000;

        Piece victim = board.getPiece(m.toX, m.toY);
        if (victim.type != EMPTY) {
            Piece attacker = board.getPiece(m.fromX, m.fromY);
            // 1 000 000 base ensures captures beat all quiet moves
            return 1'000'000 + mvvLva(victim.type, attacker.type);
        }
        if (m.promotion != EMPTY) return 900'000;
        return 0; // quiet move
    };

    std::stable_sort(moves.begin(), moves.end(),
                     [&](const Move& a, const Move& b) {
                         return score(a) > score(b);
                     });
}

// ── Time helpers ─────────────────────────────────────────────────────────────

bool ChessAI::timeUp() const {
    if (timeLimitMs == 0) return false; // fixed-depth mode
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - searchStart).count();
    return elapsed >= timeLimitMs;
}

// ── Negamax with alpha-beta + TT ─────────────────────────────────────────────

int ChessAI::negamax(Board& board, int depth, int alpha, int beta,
                      Color currentTurn, uint64_t hash) {
    nodesExplored++;

    if (stopSearch) return 0;

    // TT lookup
    TTEntry& entry = tt[hash % TT_SIZE];
    Move ttBest(0, 0, 0, 0);
    if (entry.hash == hash && entry.depth >= depth) {
        if (entry.flag == TT_EXACT) return entry.score;
        if (entry.flag == TT_LOWER && entry.score > alpha) alpha = entry.score;
        if (entry.flag == TT_UPPER && entry.score < beta)  beta  = entry.score;
        if (alpha >= beta) return entry.score;
        ttBest = entry.bestMove;
    } else if (entry.hash == hash) {
        ttBest = entry.bestMove; // still use best move for ordering
    }

    // Terminal / leaf
    if (depth == 0) {
        int s = board.evaluate();
        return (currentTurn == WHITE) ? s : -s;
    }
    if (board.isCheckmate(currentTurn)) return -10'000 - depth; // sooner mate = worse
    if (board.isStalemate(currentTurn) || board.isDraw()) return 0;

    std::vector<Move> moves = board.generateLegalMoves(currentTurn);
    if (moves.empty()) return 0;
    orderMoves(moves, board, ttBest);

    int origAlpha = alpha;
    int bestScore = std::numeric_limits<int>::min() + 1;
    Move bestMove = moves[0];

    for (const Move& m : moves) {
        if (stopSearch) break;

        GameState prevState = board.gameState;
        Piece captured = board.getPiece(m.toX, m.toY);

        // Incremental hash update
        uint64_t newHash = hash;
        Piece moving = board.getPiece(m.fromX, m.fromY);
        newHash ^= zobrist.pieceKeys[moving.color][moving.type][m.fromX][m.fromY];
        if (captured.type != EMPTY)
            newHash ^= zobrist.pieceKeys[captured.color][captured.type][m.toX][m.toY];
        // Note: promotion/castling/en passant hashes handled approximately;
        // exact incremental hash is complex — recompute from scratch for simplicity
        newHash = zobrist.computeHash(board, currentTurn); // recompute after moves below

        board.makeMove(m);
        newHash = zobrist.computeHash(board, currentTurn == WHITE ? BLACK : WHITE);

        int score = -negamax(board, depth - 1, -beta, -alpha,
                              currentTurn == WHITE ? BLACK : WHITE, newHash);
        board.undoMove(m, captured, prevState);

        if (score > bestScore) {
            bestScore = score;
            bestMove  = m;
        }
        if (score > alpha) alpha = score;
        if (alpha >= beta) break; // beta cutoff
    }

    if (!stopSearch) {
        // Store TT entry
        TTFlag flag = TT_EXACT;
        if      (bestScore <= origAlpha) flag = TT_UPPER;
        else if (bestScore >= beta)      flag = TT_LOWER;
        entry.hash     = hash;
        entry.depth    = depth;
        entry.score    = bestScore;
        entry.flag     = flag;
        entry.bestMove = bestMove;
    }

    return bestScore;
}

// ── Iterative deepening root ─────────────────────────────────────────────────

Move ChessAI::getBestMove(Board& board, Color aiColor,
                           int timeLimitMsArg, int fixedDepth) {
    // Init Zobrist once
    if (!zobristReady) { zobrist.init(); zobristReady = true; }
    // Clear TT
    std::fill(tt.get(), tt.get() + TT_SIZE, TTEntry());

    nodesExplored = 0;
    stopSearch    = false;
    timeLimitMs   = timeLimitMsArg;
    searchStart   = std::chrono::steady_clock::now();

    Move bestMove(0, 0, 0, 0);
    std::vector<Move> legalMoves = board.generateLegalMoves(aiColor);
    if (legalMoves.empty()) return bestMove;

    int maxDepth = (fixedDepth > 0) ? fixedDepth : 64; // 64 = effectively unlimited

    for (int depth = 1; depth <= maxDepth; depth++) {
        if (timeUp()) break;

        int alpha = std::numeric_limits<int>::min() + 1;
        int beta  = std::numeric_limits<int>::max() - 1;

        Move depthBest = bestMove;
        int  depthBestScore = std::numeric_limits<int>::min() + 1;

        // Re-order root moves using previous iteration's TT best
        std::vector<Move> rootMoves = legalMoves;
        orderMoves(rootMoves, board, bestMove);

        for (const Move& m : rootMoves) {
            if (stopSearch || timeUp()) { stopSearch = true; break; }

            GameState prevState = board.gameState;
            Piece captured = board.getPiece(m.toX, m.toY);
            board.makeMove(m);

            uint64_t newHash = zobrist.computeHash(board, aiColor == WHITE ? BLACK : WHITE);
            int score = -negamax(board, depth - 1, -beta, -alpha,
                                  aiColor == WHITE ? BLACK : WHITE, newHash);
            board.undoMove(m, captured, prevState);

            if (score > depthBestScore) {
                depthBestScore = score;
                depthBest      = m;
            }
            if (score > alpha) alpha = score;
        }

        if (!stopSearch) {
            bestMove = depthBest;
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - searchStart).count();
            std::cout << "  depth " << depth
                      << "  score " << depthBestScore
                      << "  nodes " << nodesExplored
                      << "  time "  << elapsed << "ms\n";
        }
    }

    std::cout << "Total nodes explored: " << nodesExplored << "\n";
    return bestMove;
}

// ── Perft ────────────────────────────────────────────────────────────────────
// Counts legal leaf nodes at the given depth.
// Used to validate the move generator against known perft values.

uint64_t ChessAI::perft(Board& board, Color color, int depth) {
    if (depth == 0) return 1;

    std::vector<Move> moves = board.generateLegalMoves(color);
    if (depth == 1) return moves.size(); // optimisation: skip make/undo at leaves

    uint64_t nodes = 0;
    Color nextColor = (color == WHITE) ? BLACK : WHITE;

    for (const Move& m : moves) {
        GameState prevState = board.gameState;
        Piece captured = board.getPiece(m.toX, m.toY);
        board.makeMove(m);
        nodes += perft(board, nextColor, depth - 1);
        board.undoMove(m, captured, prevState);
    }
    return nodes;
}