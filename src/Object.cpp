#include "Object.h"

Object::Object(glm::vec3 position, glm::vec3 velocity, float mass, glm::vec3 color)
	: pos(position), vel(velocity), mass(mass), color(color) {}

void Object::updatePos(float dt) {
	pos += vel * dt;
}