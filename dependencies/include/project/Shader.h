#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include <string>

class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexFile, const char* fragmentFile);

    void Activate();
    void Delete();
};
#endif