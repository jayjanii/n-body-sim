#include "Object.h"

Object::Object(glm::vec3 position, glm::vec3 velocity, glm::vec3 color)
	: pos(position), vel(velocity), color(color) {}

void Object::accelerate(float x, float y, float z, float dt) {
	vel.x += x * dt;
	vel.y += y * dt;
	vel.z += z * dt;
}

void Object::updatePos(float dt) {
	pos += vel * dt;
}