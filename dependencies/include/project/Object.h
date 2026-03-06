#ifndef OBJECT_CLASS_H
#define OBJECT_CLASS_H

#include <glm/glm.hpp>

class Object {
public:
	glm::vec3 pos;
	glm::vec3 vel;
	glm::vec3 color;
	float mass;

	Object(glm::vec3 position, glm::vec3 velocity, float mass, glm::vec3 color);
	virtual ~Object() = default;

	void updatePos(float dt);

	virtual void draw(int modelLoc, int colorLoc, int isStarLoc, float visualScale = 1.0f) = 0;
};

#endif