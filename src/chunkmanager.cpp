#include "chunkmanager.h"
#include "world.h"

ChunkManager::ChunkManager() {

}

void ChunkManager::init() {
	visibleChunks = (Chunk**)malloc(sizeof(Chunk*));
	visibleChunks_size = 0;
}

void ChunkManager::update(Camera *camera, int playerMoved) {

	if (playerMoved) {
		requestChunkPositions(camera);

		visibleChunks_size = toLoadPositions_size;

		requestChunks();
	}

	buildUnbuiltChunks(camera);

	checkFarChunks(camera);
}

float ChunkManager::getChunkDistanceFromCamera(Chunk *chunk, Camera *camera) {
	return (camera->Position.x - chunk->position.x)*(camera->Position.x - chunk->position.x)
		+ (camera->Position.z - chunk->position.y)*(camera->Position.z - chunk->position.y);
}

// builds one unbuilt chunk (the closest one)
void ChunkManager::buildUnbuiltChunks(Camera *camera) {

	if (unbuiltChunks.size() != 0) {
		glm::ivec2 chunk_pos = getChunkPosition(&camera->Position);

		// sort the vector WARNING BETTER TO DO IT AT THE END OF REQUEST CHUNKS!!!!!!!
		std::sort(unbuiltChunks.begin(), unbuiltChunks.end(),
			[chunk_pos](const auto& chunk_a, const auto& chunk_b) {

			return (chunk_pos.x - chunk_a->position.x)*(chunk_pos.x - chunk_a->position.x)
				+ (chunk_pos.y - chunk_a->position.y)*(chunk_pos.y - chunk_a->position.y)
				<
				(chunk_pos.x - chunk_b->position.x)*(chunk_pos.x - chunk_b->position.x)
				+ (chunk_pos.y - chunk_b->position.y)*(chunk_pos.y - chunk_b->position.y);
		});

		Chunk *chunk = unbuiltChunks[0];
		unbuiltChunks.erase(unbuiltChunks.begin()); // remove it from the build list

		// build the chunk
		chunk->calculateMesh();
		/*std::thread t = chunk->calculateMeshThreaded();
		t.join();
		// t.detach();*/
	}
}


// request chunks into visible chunks
Chunk** ChunkManager::requestChunks() {

	int size = toLoadPositions_size;

	// don't do this every frame? only at start? idk (or use loadedChunks instead)
	visibleChunks = (Chunk**)malloc(size * sizeof(Chunk*));

	float debug_start_frame = static_cast<float>(glfwGetTime());

	for (int i = 0; i < size; i++) {

		int loadedSize = loadedChunks.size();
		int chunk_x = toLoadPositions[i].x;
		int chunk_y = toLoadPositions[i].y;

		Chunk *neighbors[4] = { NULL, NULL, NULL, NULL };

		// try to find the requested chunk in the currently loaded chunks
		// only stop when neighbors and found chunk are found
		Chunk *found = NULL;
		for (int j = 0;
			j < loadedSize && (found == NULL || neighbors[0] == NULL || neighbors[1] == NULL || neighbors[2] == NULL || neighbors[3] == NULL);
			j++)
		{					// && found == NULL

			if (found == NULL
				&& loadedChunks[j]->position.x == chunk_x
				&& loadedChunks[j]->position.y == chunk_y) {

				found = loadedChunks[j];
			}

			// check neighbors
			if (loadedChunks[j]->position.x == chunk_x
				&& loadedChunks[j]->position.y == chunk_y + 1) {
				neighbors[NEIGHBOR_UP] = loadedChunks[j];
			}
			if (loadedChunks[j]->position.x == chunk_x
				&& loadedChunks[j]->position.y == chunk_y - 1) {
				neighbors[NEIGHBOR_DOWN] = loadedChunks[j];
			}
			if (loadedChunks[j]->position.x == chunk_x - 1
				&& loadedChunks[j]->position.y == chunk_y) {
				neighbors[NEIGHBOR_LEFT] = loadedChunks[j];
			}
			if (loadedChunks[j]->position.x == chunk_x + 1
				&& loadedChunks[j]->position.y == chunk_y) {
				neighbors[NEIGHBOR_RIGHT] = loadedChunks[j];
			}
		}

		if (found) {
			// the requested chunk already exists in the loaded chunks
			// add the reference to the return chunks
			visibleChunks[i] = found;

			/*int placeStructures = (neighbors[NEIGHBOR_RIGHT] != NULL
				&& neighbors[NEIGHBOR_LEFT] != NULL
				&& neighbors[NEIGHBOR_UP] != NULL
				&& neighbors[NEIGHBOR_DOWN] != NULL);
			if (placeStructures && !found->structuresPlaced) {
				world->generateChunk(found, true);
				found->structuresPlaced = true;
				unbuiltChunks.push_back(found);
			}*/

			// add blocks that were cached (placed by other chunks before this one was loaded)
			int hash = world->getChunkPosHash(found->position.x, found->position.y);
			if (world->cachedBlocks.find(hash) != world->cachedBlocks.end()) { // if we found an entry for this chunk
				int size = world->cachedBlocks[hash].size();
				for (int i = 0; i < size; i++) {
					// std::cout << "TRY PLACE BLOCK AT: "
					found->setBlock(world->cachedBlocks[hash][i].x,
						world->cachedBlocks[hash][i].y,
						world->cachedBlocks[hash][i].z, world->cachedBlocks[hash][i].type);
				}
				world->cachedBlocks.erase(hash);

				// need to regenerate chunk mesh
				unbuiltChunks.push_back(found);
			}

		}
		else {
			// the requested chunk doesn't exist yet: create it
			Chunk *chunk = (Chunk*)malloc(sizeof(Chunk));
			chunk->resetBlockData();
			// chunk->isBuilt = false;
			chunk->position = glm::ivec2(chunk_x, chunk_y);

			world->generateChunk(chunk, true);

			loadedChunks.push_back(chunk);
			unbuiltChunks.push_back(chunk);
			visibleChunks[i] = chunk;
		}

		// update neighbors and link them
		if (neighbors[NEIGHBOR_UP] != NULL) {
			visibleChunks[i]->neighbors[NEIGHBOR_UP] = neighbors[NEIGHBOR_UP];
			neighbors[NEIGHBOR_UP]->neighbors[NEIGHBOR_DOWN] = visibleChunks[i];
		}
		if (neighbors[NEIGHBOR_DOWN] != NULL) {
			visibleChunks[i]->neighbors[NEIGHBOR_DOWN] = neighbors[NEIGHBOR_DOWN];
			neighbors[NEIGHBOR_DOWN]->neighbors[NEIGHBOR_UP] = visibleChunks[i];
		}
		if (neighbors[NEIGHBOR_LEFT] != NULL) {
			visibleChunks[i]->neighbors[NEIGHBOR_LEFT] = neighbors[NEIGHBOR_LEFT];
			neighbors[NEIGHBOR_LEFT]->neighbors[NEIGHBOR_RIGHT] = visibleChunks[i];
		}
		if (neighbors[NEIGHBOR_RIGHT] != NULL) {
			visibleChunks[i]->neighbors[NEIGHBOR_RIGHT] = neighbors[NEIGHBOR_RIGHT];
			neighbors[NEIGHBOR_RIGHT]->neighbors[NEIGHBOR_LEFT] = visibleChunks[i];
		}
	}

	// std::cout << "REQUEST CHUNKS TOOK: " << static_cast<float>(glfwGetTime()) - debug_start_frame << "\n";

	return visibleChunks;
}

// calculates in which chunk the player currently is
glm::ivec2 ChunkManager::getChunkPosition(glm::vec3 *position) {
	int p_x, p_z;
	if (position->x < 0.0f) {
		if (position->x > -16.0f)
			p_x = -1;
		else p_x = static_cast<int>(position->x) / CHUNK_SIZE - 1;
	}
	else {
		p_x = static_cast<int>(position->x) / CHUNK_SIZE;
	}
	if (position->z < 0.0f) {
		if (position->z > -16.0f)
			p_z = -1;
		else p_z = static_cast<int>(position->z) / CHUNK_SIZE - 1;
	}
	else {
		p_z = static_cast<int>(position->z) / CHUNK_SIZE;
	}
	return glm::ivec2(p_x, p_z);
}

// WARNING: CURRENTLY NOT FREEING ANYTHING
void ChunkManager::checkFarChunks(Camera *camera) {

	std::vector<Chunk*> newChunks;
	std::vector<Chunk*> chunksToFree;

	// calculate in which chunk the player currently is
	glm::ivec2 chunk_pos = getChunkPosition(&camera->Position);

	int size = loadedChunks.size();
	for (int i = 0; i < size; i++) {
		if ((abs(loadedChunks[i]->position.x - chunk_pos.x) > RENDER_DISTANCE
			|| abs(loadedChunks[i]->position.y - chunk_pos.y) > RENDER_DISTANCE)
			&& !loadedChunks[i]->used) {

			loadedChunks[i]->removeNeighbors();
			chunksToFree.push_back(loadedChunks[i]);
		}
		else {
			newChunks.push_back(loadedChunks[i]);
		}
	}

	for (int i = 0; i < chunksToFree.size(); i++) {
		// free(chunksToFree[i]);
	}

	loadedChunks.clear();
	for (int i = 0; i < newChunks.size(); i++) {
		loadedChunks.push_back(newChunks[i]);
	}
}

// better as a camera member class?
void ChunkManager::requestChunkPositions(Camera *camera) {

	glm::ivec2 chunk_pos = getChunkPosition(&camera->Position);
	int one_dim = 2 * RENDER_DISTANCE + 1; // single dimension length (in chunks)
	int start_x = chunk_pos.x - RENDER_DISTANCE;
	int start_z = chunk_pos.y - RENDER_DISTANCE;
	int end_x = chunk_pos.x + RENDER_DISTANCE + 1;
	int end_z = chunk_pos.y + RENDER_DISTANCE + 1;

	free(toLoadPositions);
	toLoadPositions = (glm::ivec2*)malloc(one_dim * one_dim * sizeof(glm::ivec2));
	if (toLoadPositions == NULL) {
		std::cout << "Error requestChunkPositions(): could not allocate memory\n";
	}

	int size = 0;

	for (int x = start_x; x < end_x; x++) {
		for (int z = start_z; z < end_z; z++) {
			toLoadPositions[size].x = x;
			toLoadPositions[size].y = z;

			size++;
		}
	}

	toLoadPositions_size = size;
}

void ChunkManager::renderChunks(Shader* shader) {

	shader->use();
	if (visibleChunks != NULL) {

		for (int i = 0; i < visibleChunks_size; i++) {
			// MOVE THE CHECK SOMEWHERE ELSE?
			if (visibleChunks[i]->isBuilt) {
				visibleChunks[i]->render(shader);
			}
		}
	}
}

Chunk *ChunkManager::getPlayerChunk(Camera *camera) {

	glm::ivec2 chunk_pos = getChunkPosition(&camera->Position);
	int loadedSize = loadedChunks.size();

	Chunk *found = NULL;
	for (int i = 0; i < loadedSize && found == NULL; i++) {
		if (loadedChunks[i]->position.x == chunk_pos.x
			&& loadedChunks[i]->position.y == chunk_pos.y) {

			found = loadedChunks[i];
		}
	}

	return found;
}