#ifndef PLAYER_H
#define PLAYER_H
#include "input/input.h"

class Player {
public:
    explicit Player();
    ~Player();

private:
    InputManager input;

};


#endif //PLAYER_H