#ifndef BODY_CLASS_H
#define BODY_CLASS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "VAO.h"

class Body {
public:
	glm::vec2 pos;
	glm::vec2 vel;
	glm::vec3 color;
	float r;
	int res;

	Body(glm::vec2 position, glm::vec2 velocity, float radius, glm::vec3 color);

	void accelerate(float x, float y, float dt);
	void updatePos(float dt);
	void drawCircle(int modelLoc, int colorLoc, VAO& vao);
	void boundaryCheck(float SIM_WIDTH, float SIM_HEIGHT);
	void collisionCheck(Body& other);
};

#endif
