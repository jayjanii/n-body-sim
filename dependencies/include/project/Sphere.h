#ifndef SPHERE_CLASS_H
#define SPHERE_CLASS_H

#include "Object.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include <memory>

class Sphere : public Object {
public:
	float r;
	bool isStar;
	int sectors;
	int stacks;
	unsigned int indexCount;

	VAO meshVAO;
	std::unique_ptr<VBO> meshVBO;
	std::unique_ptr<EBO> meshEBO;

	Sphere(glm::vec3 position, glm::vec3 velocity, float mass, float radius,
		glm::vec3 color, bool isStar = false, int sectors = 36, int stacks = 18);
	~Sphere() override;

	void draw(int modelLoc, int colorLoc, int isStarLoc, float visualScale = 1.0f) override;

private:
	void generateMesh();
};

#endif