#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include <vector>

class VBO {
public:
    unsigned int ID;
    VBO(const std::vector<float>& vertices);
    void Bind();
    void Unbind();
    void Delete();
};
#endif