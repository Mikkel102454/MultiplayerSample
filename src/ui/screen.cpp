#include "ui/screen.h"

void Screen::draw() {
    for (auto& element : mElements) {
        element->draw();
    }
    onDraw();
}

void Screen::update() {
    for (auto& element : mElements) {
        element->update();
    }
    onUpdate();
}
