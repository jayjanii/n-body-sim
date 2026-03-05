#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include "VBO.h"

class VAO {
public:
    unsigned int ID;
    VAO();
    void LinkAttrib(VBO& vbo, unsigned int layout, unsigned int numComponents, unsigned int type, int stride, void* offset);
    void Bind();
    void Unbind();
    void Delete();
};
#endif