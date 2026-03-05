#ifndef OBJECT_CLASS_H
#define OBJECT_CLASS_H

#include <glm/glm.hpp>

class Object {
public:
	glm::vec3 pos;
	glm::vec3 vel;
	glm::vec3 color;

	Object(glm::vec3 position, glm::vec3 velocity, glm::vec3 color);
	virtual ~Object() = default;

	void accelerate(float x, float y, float z, float dt);
	void updatePos(float dt);

	virtual void draw(int modelLoc, int colorLoc) = 0;
	virtual void boundaryCheck(float SIM_WIDTH, float SIM_HEIGHT) = 0;
};

#endif