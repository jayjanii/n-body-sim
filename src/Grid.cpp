#define _USE_MATH_DEFINES
#include "Grid.h"
#include <glad/glad.h>
#include <cmath>

Grid::Grid(int resolution, float halfSize, float baseY, float curvatureFactor)
	: resolution(resolution), halfSize(halfSize), baseY(baseY),
	  curvatureFactor(curvatureFactor), vaoID(0), vboID(0), eboID(0),
	  indexCount(0), vertexCount(0)
{
	generateGrid();
}

Grid::~Grid() {
	if (vaoID) glDeleteVertexArrays(1, &vaoID);
	if (vboID) glDeleteBuffers(1, &vboID);
	if (eboID) glDeleteBuffers(1, &eboID);
}

void Grid::generateGrid() {
	vertexCount = (resolution + 1) * (resolution + 1);
	vertices.resize(vertexCount * 3);
	std::vector<unsigned int> indices;

	float step = (2.0f * halfSize) / resolution;

	// Flat grid in the XZ plane
	for (int i = 0; i <= resolution; i++) {
		for (int j = 0; j <= resolution; j++) {
			int idx = (i * (resolution + 1) + j) * 3;
			vertices[idx + 0] = -halfSize + j * step;
			vertices[idx + 1] = baseY;
			vertices[idx + 2] = -halfSize + i * step;
		}
	}

	// Horizontal lines (along X for each Z row)
	for (int i = 0; i <= resolution; i++) {
		for (int j = 0; j < resolution; j++) {
			unsigned int a = i * (resolution + 1) + j;
			indices.push_back(a);
			indices.push_back(a + 1);
		}
	}
	// Vertical lines (along Z for each X column)
	for (int j = 0; j <= resolution; j++) {
		for (int i = 0; i < resolution; i++) {
			unsigned int a = i * (resolution + 1) + j;
			indices.push_back(a);
			indices.push_back(a + (resolution + 1));
		}
	}

	indexCount = static_cast<unsigned int>(indices.size());

	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboID);
	glGenBuffers(1, &eboID);

	glBindVertexArray(vaoID);

	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Grid::update(const std::vector<std::unique_ptr<Object>>& objects) {
	float step = (2.0f * halfSize) / resolution;

	for (int i = 0; i <= resolution; i++) {
		for (int j = 0; j <= resolution; j++) {
			int idx = (i * (resolution + 1) + j) * 3;
			float gx = -halfSize + j * step;
			float gz = -halfSize + i * step;

			float totalDip = 0.0f;

			for (auto& obj : objects) {
				float rs = obj->mass * curvatureFactor;
				if (rs <= 0.0001f) continue;

				float dx = gx - obj->pos.x;
				float dz = gz - obj->pos.z;
				float r = sqrtf(dx * dx + dz * dz);

				// Flamm's paraboloid: z(r) = 2*sqrt(rs*(r-rs)) for r >= rs
				// Dip = z(r_max) - z(r)  so the grid is flat at the edge and dips near mass
				float rClamped = fmaxf(r, rs + 0.001f);
				float rMax = halfSize * 1.5f;

				float zAtR   = 2.0f * sqrtf(rs * (rClamped - rs));
				float zAtMax = 2.0f * sqrtf(rs * (rMax - rs));

				totalDip += (zAtMax - zAtR);
			}

			vertices[idx + 0] = gx;
			vertices[idx + 1] = baseY - totalDip;
			vertices[idx + 2] = gz;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Grid::draw(int camMatrixLoc, int lineColorLoc, int baseYLoc) {
	glUniform4f(lineColorLoc, 0.15f, 0.4f, 0.7f, 0.6f);
	glUniform1f(baseYLoc, baseY);

	glm::mat4 identity = glm::mat4(1.0f);
	// camMatrix is already set by the caller via the grid shader

	glBindVertexArray(vaoID);
	glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Grid::rebuild() {
	if (vaoID) { glDeleteVertexArrays(1, &vaoID); vaoID = 0; }
	if (vboID) { glDeleteBuffers(1, &vboID);       vboID = 0; }
	if (eboID) { glDeleteBuffers(1, &eboID);       eboID = 0; }

	vertices.clear();
	generateGrid();   // re-creates everything using current member values
}
