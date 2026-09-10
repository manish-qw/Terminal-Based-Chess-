#ifndef UCI_H
#define UCI_H

#include "board.h"
#include "chess_ai.h"
#include <string>

class UCI {
public:
    void loop();

private:
    static std::string moveToString(const Move& m);
    static Move        parseMove(Board& board, Color color, const std::string& s);
};

#endif // UCI_H
