#define _USE_MATH_DEFINES
#include "Sphere.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <utility>
#include <cmath>

static const float PI = static_cast<float>(M_PI);

Sphere::Sphere(glm::vec3 position, glm::vec3 velocity, float radius, glm::vec3 color,
	int sectors, int stacks)
	: Object(position, velocity, color), r(radius), sectors(sectors), stacks(stacks), indexCount(0)
{
	generateMesh();
}

Sphere::~Sphere() {
	meshVAO.Delete();
	if (meshVBO) meshVBO->Delete();
	if (meshEBO) meshEBO->Delete();
}

void Sphere::generateMesh() {
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	// Generate vertices: interleaved position (3) + normal (3)
	for (int i = 0; i <= stacks; i++) {
		float phi = PI * i / stacks;           // 0 to PI (top to bottom)
		float y = cosf(phi);
		float sinPhi = sinf(phi);

		for (int j = 0; j <= sectors; j++) {
			float theta = 2.0f * PI * j / sectors; // 0 to 2*PI

			float x = sinPhi * cosf(theta);
			float z = sinPhi * sinf(theta);

			// Position (unit sphere, scaled by model matrix at draw time)
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			// Normal (same as position for a unit sphere)
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
	}

	// Generate indices: two triangles per quad
	for (int i = 0; i < stacks; i++) {
		for (int j = 0; j < sectors; j++) {
			unsigned int first = i * (sectors + 1) + j;
			unsigned int second = first + sectors + 1;

			indices.push_back(first);
			indices.push_back(second);
			indices.push_back(first + 1);

			indices.push_back(first + 1);
			indices.push_back(second);
			indices.push_back(second + 1);
		}
	}

	indexCount = static_cast<unsigned int>(indices.size());

	// Upload to GPU
	meshVAO.Bind();

	meshVBO = std::make_unique<VBO>(vertices);   // binds GL_ARRAY_BUFFER

	// Position attribute  (location = 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute    (location = 1)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	meshEBO = std::make_unique<EBO>(indices);     // binds GL_ELEMENT_ARRAY_BUFFER (recorded in VAO)

	meshVAO.Unbind();
	meshVBO->Unbind();
	meshEBO->Unbind();
}

void Sphere::draw(int modelLoc, int colorLoc, float scale) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos.x, pos.y, pos.z));
	model = glm::scale(model, glm::vec3(r, r, r) * scale);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4f(colorLoc, color.r, color.g, color.b, 1.0f);

	meshVAO.Bind();
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	meshVAO.Unbind();
}

void Sphere::boundaryCheck(float SIM_WIDTH, float SIM_HEIGHT, float SIM_DEPTH) {
    if (pos.y - r < 0 || pos.y + r > SIM_HEIGHT) {
        pos.y  = (pos.y - r < 0) ? r : SIM_HEIGHT - r;
        vel.y *= -0.7f;
    }
    if (pos.x - r < 0 || pos.x + r > SIM_WIDTH) {
        pos.x  = (pos.x - r < 0) ? r : SIM_WIDTH - r;
        vel.x *= -0.5f;
    }
    if (pos.z - r < 0 || pos.z + r > SIM_DEPTH) {
        pos.z  = (pos.z - r < 0) ? r : SIM_DEPTH - r;
        vel.z *= -0.7f;
    }
}

void Sphere::collisionCheck(Sphere& other) {
	// TODO (optional): implement elastic collision response between two spheres
}