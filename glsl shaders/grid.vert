#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 camMatrix;

out float height;

void main()
{
    gl_Position = camMatrix * vec4(aPos, 1.0);
    height = aPos.y;
}
