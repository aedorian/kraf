#include "raycast.h"
#include "chunk.h"
#include "block.h"

int Raycast::raycast(GLFWwindow *window, World *world, Camera *camera) {
	glm::vec3 cameraPos = camera->Position;
	Chunk *currentChunk = world->chunkManager.getPlayerChunk(camera);
	glm::vec3 dir = glm::normalize(camera->Front);

	glm::ivec3 blockPos = glm::ivec3(
		floor(cameraPos.x) - currentChunk->position.x * CHUNK_SIZE,
		floor(cameraPos.y),
		floor(cameraPos.z) - currentChunk->position.y * CHUNK_SIZE);
	glm::ivec3 prevBlockPos = glm::ivec3(0);

	// debug current chunk
	/*
	shader.setVec3("color", glm::vec3(0.0, 0.0, 1.0));
	renderBlock(
		glm::vec3(currentChunk->position.x * CHUNK_SIZE + 8, 50, currentChunk->position.y * CHUNK_SIZE + 8),
		glm::vec3(16, 100, 16));
	*/

	shader.setVec3("color", glm::vec3(0.0, 0.0, 0.0));

	int dist = 0;
	int maxDist = 90;
	// while distance is respected and current block isn't solid
	while (dist < maxDist
		&& !currentChunk->isSolid(currentChunk->getBlock(blockPos.x, blockPos.y, blockPos.z))) {

		cameraPos += dir * 0.05f;

		prevBlockPos = glm::ivec3(blockPos.x, blockPos.y, blockPos.z);

		blockPos = glm::ivec3(
			floor(cameraPos.x) - currentChunk->position.x * CHUNK_SIZE,
			floor(cameraPos.y),
			floor(cameraPos.z) - currentChunk->position.y * CHUNK_SIZE);

		// change chunks if the ray is out of the current chunk's bounds
		if (currentChunk != NULL && blockPos.x > CHUNK_SIZE - 1) {
			currentChunk = currentChunk->neighbors[NEIGHBOR_RIGHT];
			blockPos.x = blockPos.x - CHUNK_SIZE;
		}
		if (currentChunk != NULL && blockPos.x < 0) {
			currentChunk = currentChunk->neighbors[NEIGHBOR_LEFT];
			blockPos.x = blockPos.x + CHUNK_SIZE;
		}
		if (currentChunk != NULL && blockPos.z > CHUNK_SIZE - 1) {
			currentChunk = currentChunk->neighbors[NEIGHBOR_UP];
			blockPos.z = blockPos.z - CHUNK_SIZE;
		}
		if (currentChunk != NULL && blockPos.z < 0) {
			currentChunk = currentChunk->neighbors[NEIGHBOR_DOWN];
			blockPos.z = blockPos.z + CHUNK_SIZE;
		}

		dist++;

	}


	// returns
	if (currentChunk->isSolid(currentChunk->getBlock(blockPos.x, blockPos.y, blockPos.z))) {
		// render the block wireframe
		renderBlock(
			glm::vec3(blockPos.x + currentChunk->position.x * CHUNK_SIZE,
				blockPos.y,
				blockPos.z + currentChunk->position.y * CHUNK_SIZE),
			glm::vec3(1, 1, 1));

		// set hit block data
		hitBlockPos = blockPos;
		hitChunk = currentChunk;

		// calculate the face that was hit
		int off_x, off_y, off_z;
		off_x = blockPos.x - prevBlockPos.x;
		off_y = blockPos.y - prevBlockPos.y;
		off_z = blockPos.z - prevBlockPos.z;
		hitFace = glm::ivec3(-off_x, -off_y, -off_z);

		return 1;
	}
	else {
		hitChunk = NULL;

		return 0;
	}

}

void Raycast::renderBlock(glm::vec3 pos, glm::vec3 scale) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::scale(model, scale);

	shader.setMat4("model", model);

	glLineWidth(3);
	glBindVertexArray(VAO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Raycast::breakBlock() {
	hitChunk->breakBlock(hitBlockPos.x,
		hitBlockPos.y,
		hitBlockPos.z,
		1); // recalculate meshes and neighbouring chunk meshes
}

void Raycast::placeBlock(BlockType type) {
	std::cout << "HITFACE: x = " << hitFace.x << " y = " << hitFace.y << " z = " << hitFace.z << "\n";
	hitChunk->placeBlock(hitBlockPos.x + hitFace.x,
		hitBlockPos.y + hitFace.y,
		hitBlockPos.z + hitFace.z,
		type,
		1); // recalculate meshes and neighbouring chunk meshes
}

void Raycast::init() {
	shader = Shader("shaders/ray_v.vert", "shaders/ray_f.frag");

	initVAO();
}

void Raycast::initVAO() {

	meshData = (float*)malloc(180 * sizeof(float));
	if (meshData == NULL) {
		std::cout << "Error allocating mesh data memory\n";
	}

	int dataSize = 180;
	for (int i = 0; i < dataSize; i++) {
		meshData[i] = blockVertices[i];
	}

	// make opengl data
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(float), meshData, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}