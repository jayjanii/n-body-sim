#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec4 objectColor;

void main()
{
    // Ambient
    float ambient = 0.15;

    // Diffuse (directional light from upper-right)
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 result = (ambient + diff) * objectColor.rgb;
    FragColor = vec4(result, objectColor.a);
}