#version 330 core
out vec4 FragColor;

in float height;

uniform vec4 lineColor;
uniform float baseY;

void main()
{
    // Tint toward white the deeper the well goes
    float depth = clamp((baseY - height) * 0.3, 0.0, 0.8);
    vec3 color = mix(lineColor.rgb, vec3(1.0), depth);
    FragColor = vec4(color, lineColor.a);
}
