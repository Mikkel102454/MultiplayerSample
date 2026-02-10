#ifndef SCREEN_H
#define SCREEN_H
#include <vector>

#include "element.h"

class Screen {
public:
    virtual ~Screen() = default;

    void draw();
    void update();

protected:
    std::vector<Element*> mElements;

    virtual void onDraw() {}
    virtual void onUpdate() {}
};

#endif //SCREEN_H