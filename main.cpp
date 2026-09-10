#include "game.h"
#include "benchmark.h"
#include "chess_ai.h"
#include "board.h"
#include "uci.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

// Run perft test and print results for depths 1..N
static void runPerft(int maxDepth) {
    Board board;
    ChessAI ai;

    // Known correct values from the start position
    const uint64_t expected[] = { 0, 20, 400, 8902, 197281, 4865609 };

    std::cout << "Perft test from start position:\n";
    std::cout << "------------------------------------\n";

    bool allPassed = true;
    for (int d = 1; d <= maxDepth && d <= 5; d++) {
        uint64_t nodes = ai.perft(board, WHITE, d);
        bool pass = (nodes == expected[d]);
        std::cout << "  perft(" << d << ") = " << nodes;
        if (d <= 5) {
            std::cout << "  expected=" << expected[d]
                      << (pass ? "  PASS" : "  FAIL");
            if (!pass) allPassed = false;
        }
        std::cout << "\n";
    }
    std::cout << "------------------------------------\n";
    std::cout << (allPassed ? "All perft checks PASSED.\n" : "Some perft checks FAILED.\n");
}

int main(int argc, char* argv[]) {
    // ./chess --perft N   → run perft and exit
    if (argc >= 2 && std::strcmp(argv[1], "--perft") == 0) {
        int depth = (argc >= 3) ? std::atoi(argv[2]) : 4;
        runPerft(depth);
        return 0;
    }

    // ./chess --bench     → run depth benchmark and exit
    if (argc >= 2 && std::strcmp(argv[1], "--bench") == 0) {
        benchmark();
        return 0;
    }

    // ./chess --uci       → UCI protocol mode (for cutechess-cli / Arena)
    if (argc >= 2 && std::strcmp(argv[1], "--uci") == 0) {
        UCI uci;
        uci.loop();
        return 0;
    }

    std::cout << "Terminal Chess AI  (--perft N | --bench | --uci)\n\n";
    Game g;
    g.play();
    return 0;
}