#version 330 core
out vec4 FragColor;

in vec2 UV;

uniform vec4 glowColor;

void main()
{
    float dist = length(UV - vec2(0.5));
    float intensity = exp(-dist * dist * 10.0);
    FragColor = vec4(glowColor.rgb * intensity, intensity * 0.7);
}
