#ifndef GRID_CLASS_H
#define GRID_CLASS_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>
#include "Object.h"

class Grid {
public:
	int resolution;
	float halfSize;
	float baseY;
	float curvatureFactor;

	unsigned int vaoID;
	unsigned int vboID;
	unsigned int eboID;
	unsigned int indexCount;
	unsigned int vertexCount;

	Grid(int resolution, float halfSize, float baseY, float curvatureFactor);
	~Grid();

	// Call when resolution changes — destroys and recreates GPU buffers
	void rebuild();

	void update(const std::vector<std::unique_ptr<Object>>& objects);
	void draw(int camMatrixLoc, int lineColorLoc, int baseYLoc);

private:
	std::vector<float> vertices;
	void generateGrid();
};

#endif
