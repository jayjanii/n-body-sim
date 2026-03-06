#define _USE_MATH_DEFINES
#include "Sphere.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

static const float PI = static_cast<float>(M_PI);

Sphere::Sphere(glm::vec3 position, glm::vec3 velocity, float mass, float radius,
	glm::vec3 color, bool isStar, int sectors, int stacks)
	: Object(position, velocity, mass, color), r(radius), isStar(isStar),
	  sectors(sectors), stacks(stacks), indexCount(0)
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

	for (int i = 0; i <= stacks; i++) {
		float phi = PI * i / stacks;
		float y = cosf(phi);
		float sinPhi = sinf(phi);

		for (int j = 0; j <= sectors; j++) {
			float theta = 2.0f * PI * j / sectors;

			float x = sinPhi * cosf(theta);
			float z = sinPhi * sinf(theta);

			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
	}

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

	meshVAO.Bind();

	meshVBO = std::make_unique<VBO>(vertices);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	meshEBO = std::make_unique<EBO>(indices);

	meshVAO.Unbind();
	meshVBO->Unbind();
	meshEBO->Unbind();
}

void Sphere::draw(int modelLoc, int colorLoc, int isStarLoc, float visualScale) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::scale(model, glm::vec3(r * visualScale));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4f(colorLoc, color.r, color.g, color.b, 1.0f);
	glUniform1i(isStarLoc, isStar ? 1 : 0);

	meshVAO.Bind();
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	meshVAO.Unbind();
}