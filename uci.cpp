#include "uci.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>

// ── Helpers ───────────────────────────────────────────────────────────────────

std::string UCI::moveToString(const Move& m) {
    std::string s;
    s += (char)('a' + m.fromY);
    s += (char)('1' + m.fromX);
    s += (char)('a' + m.toY);
    s += (char)('1' + m.toX);
    if (m.promotion != EMPTY) {
        switch (m.promotion) {
            case QUEEN:  s += 'q'; break;
            case ROOK:   s += 'r'; break;
            case BISHOP: s += 'b'; break;
            case KNIGHT: s += 'n'; break;
            default: break;
        }
    }
    return s;
}

Move UCI::parseMove(Board& board, Color color, const std::string& s) {
    std::vector<Move> legal = board.generateLegalMoves(color);
    for (const Move& m : legal) {
        if (moveToString(m) == s) return m;
    }
    return Move(0, 0, 0, 0); // null move — caller should check
}

// ── UCI main loop ─────────────────────────────────────────────────────────────
// Implements the subset of UCI required for cutechess-cli / Arena:
//   uci  isready  ucinewgame  position  go  stop  quit

void UCI::loop() {
    // Unbuffered I/O — GUIs expect instant responses
    std::setvbuf(stdin,  nullptr, _IONBF, 0);
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    Board   board;
    ChessAI ai;
    Color   turn = WHITE;

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        // ── uci ──────────────────────────────────────────────────────────────
        if (cmd == "uci") {
            std::cout << "id name TerminalChessAI\n";
            std::cout << "id author manish-qw\n";
            std::cout << "uciok\n";
        }

        // ── isready ──────────────────────────────────────────────────────────
        else if (cmd == "isready") {
            std::cout << "readyok\n";
        }

        // ── ucinewgame ───────────────────────────────────────────────────────
        else if (cmd == "ucinewgame") {
            board = Board();   // reset board
            turn  = WHITE;
        }

        // ── position [startpos | fen <fen>] [moves <m1> <m2> ...]  ───────────
        else if (cmd == "position") {
            std::string token;
            iss >> token;

            board = Board();
            turn  = WHITE;

            if (token == "fen") {
                // We don't support arbitrary FEN (no FEN parser yet),
                // only startpos is used by cutechess for normal games.
                // Skip until "moves" keyword.
                while (iss >> token && token != "moves") {}
            } else if (token == "startpos") {
                iss >> token; // consume "moves" if present
            }

            // Apply move list
            // "token" may already be "moves" from startpos branch
            if (token == "moves") {
                std::string mv;
                while (iss >> mv) {
                    Move m = parseMove(board, turn, mv);
                    // If move is null (0,0,0,0) it means parse failed — skip
                    if (m.fromX == 0 && m.fromY == 0 && m.toX == 0 && m.toY == 0
                        && mv != "a1a1") { // guard: a1a1 is a valid (if weird) move notation check
                        break;
                    }
                    board.makeMove(m);
                    turn = (turn == WHITE) ? BLACK : WHITE;
                }
            }
        }

        // ── go [movetime <ms>] [wtime <ms>] [btime <ms>] [depth <d>] ────────
        else if (cmd == "go") {
            int timeLimitMs = 2000; // default 2 s
            int fixedDepth  = 0;

            std::string token;
            while (iss >> token) {
                if (token == "movetime") {
                    int ms; if (iss >> ms) timeLimitMs = ms;
                } else if (token == "depth") {
                    int d; if (iss >> d) { fixedDepth = d; timeLimitMs = 0; }
                } else if ((token == "wtime" && turn == WHITE) ||
                           (token == "btime" && turn == BLACK)) {
                    // Use ~5 % of remaining clock, floor at 100 ms
                    int ms; if (iss >> ms) timeLimitMs = std::max(100, ms / 20);
                } else if (token == "infinite") {
                    timeLimitMs = 0; fixedDepth = 64; // search forever
                }
            }

            Move best = ai.getBestMove(board, turn, timeLimitMs, fixedDepth, /*silent=*/true);
            std::cout << "bestmove " << moveToString(best) << "\n";
        }

        // ── stop ─────────────────────────────────────────────────────────────
        else if (cmd == "stop") {
            // Nothing to do (we search synchronously); required by protocol
        }

        // ── quit ─────────────────────────────────────────────────────────────
        else if (cmd == "quit") {
            break;
        }
    }
}
