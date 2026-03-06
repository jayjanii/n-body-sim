#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 camMatrix;
uniform vec3 billboardCenter;
uniform float billboardSize;
uniform vec3 camRight;
uniform vec3 camUp;

out vec2 UV;

void main()
{
    vec3 worldPos = billboardCenter
                  + camRight * aPos.x * billboardSize
                  + camUp    * aPos.y * billboardSize;
    gl_Position = camMatrix * vec4(worldPos, 1.0);
    UV = aPos.xy * 0.5 + 0.5;
}
