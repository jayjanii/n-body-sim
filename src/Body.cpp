#include "Body.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

Body::Body(glm::vec2 position, glm::vec2 velocity, float radius, glm::vec3 color)
	: pos(position), vel(velocity), r(radius), color(color) {
	res = 50;
}

void Body::accelerate(float x, float y, float dt) {
	vel.x += x * dt;
	vel.y += y * dt;
}

void Body::updatePos(float dt) {
	pos += vel * dt;
}

void Body::drawCircle(int modelLoc, int colorLoc, VAO& vao) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos.x, pos.y, 0.0f));
	model = glm::scale(model, glm::vec3(r, r, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(colorLoc, color.r, color.g, color.b, 1.0f);

    vao.Bind();
    glDrawArrays(GL_TRIANGLE_FAN, 0, res + 2);
}

void Body::boundaryCheck(float SIM_WIDTH, float SIM_HEIGHT) {
    if (pos.y - r < 0 || pos.y + r > SIM_HEIGHT) {
        if (pos.y - r < 0) {
            pos.y = r;
        }
        else {
            pos.y = SIM_HEIGHT - r;
        }
        vel.y *= -0.7f;
    }

    if (pos.x - r < 0 || pos.x + r > SIM_WIDTH) {
        if (pos.x - r < 0) {
            pos.x = r;
        }
        else {
            pos.x = SIM_WIDTH - r;
        }
        vel.x *= -0.5f;
    }
}

void Body::collisionCheck(Body& other) {
    float distance = glm::distance(this->pos, other.pos);
    float minimumDistance = this->r + other.r;

    if (distance < minimumDistance) {
        // Swap velocities (elastic collision placeholder for equal mass)
        std::swap(this->vel, other.vel);

        // Positional resolution to prevent sticking
        float overlap = minimumDistance - distance;
        glm::vec2 normal = glm::normalize(other.pos - this->pos);

        // Push both bodies apart equally
        float percent = 0.51f;
        this->pos -= normal * (overlap * percent);
        other.pos += normal * (overlap * percent);
    }
}