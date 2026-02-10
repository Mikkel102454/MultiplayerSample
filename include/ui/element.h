#ifndef ELEMENT_H
#define ELEMENT_H

class Element {
public:
    virtual ~Element() = default;

    virtual void draw() = 0;
    virtual bool update() = 0;
};

#endif //ELEMENT_H