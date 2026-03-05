#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include <vector>

class EBO {
public:
	unsigned int ID;
	EBO(const std::vector<unsigned int>& indices);
	void Bind();
	void Unbind();
	void Delete();
};

#endif