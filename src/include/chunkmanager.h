#ifndef _CHUNK_MANAGER_H_
#define _CHUNK_MANAGER_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "chunk.h"
#include "renderer.h"
#include "camera.h"

#include <vector>
#include <algorithm>
#include <thread>

class World;

class ChunkManager {

public:

	World *world;

	std::vector<Chunk*> loadedChunks; // keep references to all the loaded chunks
	// std::vector<Chunk*> loadedChunks;
	std::vector<Chunk*> toUnloadChunks;
	std::vector<Chunk*> unbuiltChunks;

	glm::ivec2 *toLoadPositions; // chunk positions to load
	int toLoadPositions_size = 0;

	Chunk **visibleChunks;
	int visibleChunks_size;

	ChunkManager();

	void init();

	void update(Camera *camera, int playerMoved);

	float getChunkDistanceFromCamera(Chunk *chunk, Camera *camera);

	// builds one unbuilt chunk (the closest one)
	void buildUnbuiltChunks(Camera *camera);

	// request chunks into visible chunks
	Chunk** requestChunks();

	// calculates in which chunk the player currently is
	glm::ivec2 getChunkPosition(glm::vec3 *position);

	// WARNING: CURRENTLY NOT FREEING ANYTHING
	void checkFarChunks(Camera *camera);

	// better as a camera member class?
	void requestChunkPositions(Camera *camera);

	void renderChunks(Shader* shader);

	Chunk *getPlayerChunk(Camera *camera);
};

#endif /* _CHUNK_MANAGER_H_ */