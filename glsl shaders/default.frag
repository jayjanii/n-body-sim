#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec4 objectColor;
uniform int isStar;
uniform vec3 starPos;

void main()
{
    if (isStar == 1) {
        // Full emissive for the star — no shading
        FragColor = vec4(objectColor.rgb * 1.5, objectColor.a);
    } else {
        // Point light from the star position
        vec3 lightDir = normalize(starPos - FragPos);
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);

        float ambient = 0.08;
        vec3 result = (ambient + diff) * objectColor.rgb;
        FragColor = vec4(result, objectColor.a);
    }
}