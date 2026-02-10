#ifndef PLAYER_H
#define PLAYER_H
#include "input/input.h"

class Player {
public:
    explicit Player();
    ~Player() = default;

    void draw();
    void update();

private:
    int mPosX, mPozY;

    std::map<int, Player> mEnimies{};
};


#endif //PLAYER_H