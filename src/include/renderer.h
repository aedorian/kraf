#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "block.h"

#include "window.h"



#define RENDER_DISTANCE 6 // 5

#define NEAR_PLANE 0.1f
#define FAR_PLANE 300.0f



// used to update view and projection matrices
void prepareShaderMatrices(Shader* shader, Camera* camera);



class BlockModel {

public:

	BlockModel() {}

	void init();

	void initBlockVAO();

	void renderBlock(Shader *shader, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, BlockType type);

private:

	unsigned int VBO, VAO;
	float* meshData;
	int meshData_size;
};

#endif /* _RENDERER_H_ */