#include "OrbitTrail.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

OrbitTrail::OrbitTrail(unsigned int maxPoints, glm::vec3 color)
	: maxPoints(maxPoints), head(0), count(0), color(color), vaoID(0), vboID(0)
{
	points.resize(maxPoints);

	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboID);

	glBindVertexArray(vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, maxPoints * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

OrbitTrail::~OrbitTrail() {
	if (vaoID) glDeleteVertexArrays(1, &vaoID);
	if (vboID) glDeleteBuffers(1, &vboID);
}

void OrbitTrail::addPoint(glm::vec3 point) {
	points[head] = point;
	head = (head + 1) % maxPoints;
	if (count < maxPoints) count++;
}

void OrbitTrail::draw(int camMatrixLoc, int lineColorLoc) {
	if (count < 2) return;

	// Linearize the ring buffer into a contiguous draw
	std::vector<glm::vec3> ordered;
	ordered.reserve(count);

	if (count < maxPoints) {
		// Buffer not full — points are 0..count-1
		for (unsigned int i = 0; i < count; i++)
			ordered.push_back(points[i]);
	} else {
		// Buffer full — oldest is at head, wrap around
		for (unsigned int i = 0; i < count; i++)
			ordered.push_back(points[(head + i) % maxPoints]);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ordered.size() * sizeof(glm::vec3), ordered.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUniform4f(lineColorLoc, color.r, color.g, color.b, 0.7f);

	glBindVertexArray(vaoID);
	glDrawArrays(GL_LINE_STRIP, 0, static_cast<int>(ordered.size()));
	glBindVertexArray(0);
}
