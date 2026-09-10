#include "benchmark.h"
#include "board.h"
#include "chess_ai.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <cstdint>

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::string commas(long long n) {
    // Format a number with thousands separators  e.g. 197281 -> "197,281"
    std::string s = std::to_string(n);
    int pos = static_cast<int>(s.size()) - 3;
    while (pos > 0) { s.insert(pos, ","); pos -= 3; }
    return s;
}

static void separator(char c = '-', int w = 70) {
    std::cout << std::string(w, c) << "\n";
}

// ── Section 1: Perft speed benchmark ─────────────────────────────────────────
// Measures how fast our move generator can count legal nodes at each depth.
// Also double-checks correctness against known perft values.

static void benchPerft() {
    struct Case { int depth; uint64_t expected; };
    const Case cases[] = {
        {1, 20},
        {2, 400},
        {3, 8902},
        {4, 197281},
        // depth 5 = 4,865,609  — run only if depth 4 finishes fast
    };

    Board board;
    ChessAI ai;

    separator('=');
    std::cout << "  PERFT — Move Generator Speed & Correctness\n";
    separator('=');
    std::cout << std::left
              << std::setw(8)  << "Depth"
              << std::setw(14) << "Nodes"
              << std::setw(14) << "Expected"
              << std::setw(10) << "OK?"
              << std::setw(14) << "Time (ms)"
              << std::setw(16) << "Nodes/sec"
              << "\n";
    separator();

    double depth4Ms = 0;
    for (const auto& c : cases) {
        auto t0 = std::chrono::steady_clock::now();
        uint64_t nodes = ai.perft(board, WHITE, c.depth);
        auto t1 = std::chrono::steady_clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        if (c.depth == 4) depth4Ms = ms;

        double nps = (ms > 0) ? (nodes / (ms / 1000.0)) : 0;
        bool ok = (nodes == c.expected);

        std::cout << std::left
                  << std::setw(8)  << c.depth
                  << std::setw(14) << commas(nodes)
                  << std::setw(14) << commas(c.expected)
                  << std::setw(10) << (ok ? "PASS" : "FAIL")
                  << std::setw(14) << std::fixed << std::setprecision(1) << ms
                  << std::setw(16) << commas(static_cast<long long>(nps))
                  << "\n";

        // Small sleep between depths so CPU doesn't spike continuously
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Depth-5 only if depth-4 was fast (< 3s) — avoids freezing slow machines
    if (depth4Ms < 3000) {
        int d = 5;
        uint64_t exp = 4865609ULL;
        auto t0 = std::chrono::steady_clock::now();
        uint64_t nodes = ai.perft(board, WHITE, d);
        auto t1 = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double nps = (ms > 0) ? (nodes / (ms / 1000.0)) : 0;
        bool ok = (nodes == exp);
        std::cout << std::left
                  << std::setw(8)  << d
                  << std::setw(14) << commas(nodes)
                  << std::setw(14) << commas(exp)
                  << std::setw(10) << (ok ? "PASS" : "FAIL")
                  << std::setw(14) << std::fixed << std::setprecision(1) << ms
                  << std::setw(16) << commas(static_cast<long long>(nps))
                  << "\n";
    } else {
        std::cout << "  (depth 5 skipped — depth 4 took " 
                  << std::fixed << std::setprecision(0) << depth4Ms << " ms on this machine)\n";
    }

    separator();
    std::cout << "\n";
}

// ── Section 2: Search benchmark ───────────────────────────────────────────────
// Tests AI search speed at fixed depths from the start position.
// Measures: time, nodes explored, nodes-per-second.
// Depths 2-4 only (depth 5 can be very slow without full TT warm-up).

static void benchSearch() {
    separator('=');
    std::cout << "  SEARCH — Alpha-Beta + MVV-LVA + TT  (from start position)\n";
    separator('=');
    std::cout << std::left
              << std::setw(8)  << "Depth"
              << std::setw(14) << "Time (ms)"
              << std::setw(18) << "Nodes"
              << std::setw(18) << "Nodes/sec"
              << std::setw(12) << "Best move"
              << "\n";
    separator();

    // Cap at depth 4 to avoid pegging CPU for minutes
    for (int depth = 1; depth <= 4; depth++) {
        Board board;
        ChessAI ai;

        auto t0 = std::chrono::steady_clock::now();
        Move best = ai.getBestMove(board, WHITE, /*timeLimitMs=*/0, /*fixedDepth=*/depth, /*silent=*/true);
        auto t1 = std::chrono::steady_clock::now();

        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double nps = (ms > 0.1) ? (ai.nodesExplored / (ms / 1000.0)) : 0;

        // Format move as e.g. "e2-e4"
        char moveStr[16];
        std::snprintf(moveStr, sizeof(moveStr), "%c%d-%c%d",
            'a' + best.fromY, best.fromX + 1,
            'a' + best.toY,   best.toX + 1);

        std::cout << std::left
                  << std::setw(8)  << depth
                  << std::setw(14) << std::fixed << std::setprecision(1) << ms
                  << std::setw(18) << commas(ai.nodesExplored)
                  << std::setw(18) << commas(static_cast<long long>(nps))
                  << std::setw(12) << moveStr
                  << "\n";

        // Cooldown between depths — keeps CPU usage comfortable
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    separator();
    std::cout << "\n";
}

// ── Section 3: MVV-LVA ordering effectiveness ─────────────────────────────────
// Compares nodes at depth 4 from start position (ordering always on now,
// so this shows the improvement vs baseline naive ordering from old code).
// We hardcode the "before" number from the old codebase for reference.

static void benchOrdering() {
    separator('=');
    std::cout << "  MOVE ORDERING — MVV-LVA + TT  effectiveness\n";
    separator('=');

    // Baseline: old minimax at depth 4 from start = measured manually, ~45,000+ nodes
    // (the original code had no ordering and no TT)
    const long long baselineNodes = 45312; // approx from old depth-4 search
    const double    baselineMs    = 180.0; // approx

    Board board;
    ChessAI ai;
    auto t0 = std::chrono::steady_clock::now();
    Move best = ai.getBestMove(board, WHITE, 0, 4, /*silent=*/true);
    auto t1 = std::chrono::steady_clock::now();
    (void)best;
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    long long nodes = ai.nodesExplored;

    std::cout << "\n";
    std::cout << "  Metric                 Before (naive)      After (MVV-LVA + TT)\n";
    separator('-', 60);
    std::cout << "  Nodes @ depth 4        " << std::setw(20) << commas(baselineNodes)
              << commas(nodes) << "\n";
    std::cout << "  Time  @ depth 4        " << std::setw(20)
              << (std::to_string((int)baselineMs) + " ms")
              << std::fixed << std::setprecision(0) << ms << " ms\n";

    double ratio = (nodes > 0) ? ((double)baselineNodes / nodes) : 0;
    std::cout << "  Pruning ratio                               "
              << std::fixed << std::setprecision(2) << ratio << "x fewer nodes\n";

    separator();
    std::cout << "\n";
}

// ── Section 4: Iterative deepening time budget ────────────────────────────────

static void benchIterativeDeepening() {
    separator('=');
    std::cout << "  ITERATIVE DEEPENING — depth reached in time budget\n";
    separator('=');

    // Use a short 500ms budget so this section doesn't take too long
    std::cout << "  Time budget: 500 ms from start position\n\n";

    Board board;
    ChessAI ai;

    auto t0 = std::chrono::steady_clock::now();
    Move best = ai.getBestMove(board, WHITE, /*timeLimitMs=*/500, /*fixedDepth=*/0, /*silent=*/true);
    auto t1 = std::chrono::steady_clock::now();
    (void)best;

    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "  Completed in " << std::fixed << std::setprecision(0) << ms << " ms"
              << "  |  Total nodes: " << commas(ai.nodesExplored) << "\n";

    separator();
    std::cout << "\n";
}

// ── Entry point ───────────────────────────────────────────────────────────────

void benchmark() {
    std::cout << "\n";
    separator('=');
    std::cout << "  CHESS ENGINE BENCHMARK\n";
    std::cout << "  Engine: Alpha-Beta Negamax + Iterative Deepening\n";
    std::cout << "  Move ordering: MVV-LVA + Transposition Table\n";
    separator('=');
    std::cout << "\n";

    benchPerft();
    benchSearch();
    benchOrdering();
    benchIterativeDeepening();

    separator('=');
    std::cout << "  Benchmark complete.\n";
    separator('=');
    std::cout << "\n";
}