#include "renderer.h"

void prepareShaderMatrices(Shader* shader, Camera* camera) {

	// set projection, view matrices
	shader->use();
	glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)WIN_WIDTH / (float)WIN_HEIGHT, NEAR_PLANE, FAR_PLANE);
	shader->setMat4("projection", projection);
	glm::mat4 view = camera->GetViewMatrix();
	shader->setMat4("view", view);

}

void BlockModel::init() {

	initBlockVAO();
}

void BlockModel::initBlockVAO() {

	meshData = (float*)malloc(216 * sizeof(float));
	if (meshData == NULL) {
		std::cout << "Error allocating mesh data memory\n";
	}

	int size = 0;

	// add each face to block
	for (int i = 0; i < 6; i++) {

		for (int j = 0; j < N_FACE_DATA; j += 5) {

			// position
			meshData[size++] = faceData[i][j];
			meshData[size++] = faceData[i][j + 1];
			meshData[size++] = faceData[i][j + 2];
			// texture uv
			meshData[size++] = faceData[i][j + 3];
			meshData[size++] = faceData[i][j + 4];
			// texture offset index
			meshData[size++] = i;
		}
	}
	
	meshData_size = size;

	std::cout << size << "\n";

	for (int i = 0; i < size; i++) {
		std::cout << meshData[i] << "\n";
	}

	// make opengl data
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, meshData_size * sizeof(float), meshData, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture array index
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void BlockModel::renderBlock(Shader *shader, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, BlockType type) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::rotate(model, glm::radians(rot.x), glm::vec3(1.0, 0.0, 0.0));
	model = glm::rotate(model, glm::radians(rot.y), glm::vec3(0.0, 1.0, 0.0));
	model = glm::rotate(model, glm::radians(rot.z), glm::vec3(0.0, 0.0, 1.0));
	model = glm::scale(model, scale);

	shader->use();
	shader->setMat4("model", model);

	// send face type in shader array (better than rebuilding VAO each frame)
	for (int i = 0; i < 6; i++) {
		std::stringstream ss;
		ss << "texOffsets[" << i << "]";
		shader->setVec2(ss.str().c_str(), glm::vec2(faceTexture[type][i].x, faceTexture[type][i].y));
	}

	glBindVertexArray(VAO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}