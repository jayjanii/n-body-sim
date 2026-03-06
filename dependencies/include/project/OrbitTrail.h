#ifndef ORBIT_TRAIL_CLASS_H
#define ORBIT_TRAIL_CLASS_H

#include <glm/glm.hpp>
#include <vector>

class OrbitTrail {
public:
	unsigned int maxPoints;
	unsigned int head;
	unsigned int count;
	std::vector<glm::vec3> points;
	glm::vec3 color;

	unsigned int vaoID;
	unsigned int vboID;

	OrbitTrail(unsigned int maxPoints, glm::vec3 color);
	~OrbitTrail();

	void addPoint(glm::vec3 point);
	void draw(int camMatrixLoc, int lineColorLoc);
};

#endif
