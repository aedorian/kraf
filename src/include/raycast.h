#ifndef _RAYCAST_H_
#define _RAYCAST_H_

#include "camera.h"
#include "world.h"
#include "shader.h"

class Raycast {

public:

	Chunk *hitChunk;
	glm::ivec3 hitBlockPos;
	glm::ivec3 hitFace; // offset to the adjacent block to the face hit by raycast

	Raycast() {};

	void init();

	void initVAO();

	// raycasts and updates hitChunk and hitBlockPos
	// returns 1 if hit something, 0 if not
	int raycast(GLFWwindow *window, World *world, Camera *camera);

	void renderBlock(glm::vec3 pos, glm::vec3 scale);

	void placeBlock(BlockType type);

	void breakBlock();

	Shader *getShader() {
		return &shader;
	};

private:

	Shader shader; // custom shader for raycasted block rendering

	unsigned int VBO, VAO;
	float* meshData;
	int meshData_size;
};

#endif /* _RAYCAST_H_ */