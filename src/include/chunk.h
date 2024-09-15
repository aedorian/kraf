#ifndef _CHUNK_H_
#define _CHUNK_H_

#include <GLFW/glfw3.h>

#include <vector>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "block.h"

#include <FastNoise/FastNoise.h>

#define CHUNK_SIZE 16
#define HEIGHT_LIMIT 100

#define NEIGHBOR_UP 0
#define NEIGHBOR_DOWN 1
#define NEIGHBOR_LEFT 2
#define NEIGHBOR_RIGHT 3

// return codes for setBlockWithNeighbors()
#define BLOCK_PLACED -2
#define BLOCK_NOT_PLACED -1

enum BiomeType {
	PLAINS,
	FOREST,
	DESERT,
	JUNGLE
};

class Chunk {

public:

	/* for OpenGL */
	unsigned int VBO, VAO;
	float* meshData;
	int meshData_size;
	int n_meshTriangles;

	glm::ivec2 position; // x, z
	bool isBuilt; // has the chunk been generated?
	bool used; // has the chunk been modified?
	bool structuresPlaced;

	Chunk *neighbors[4]; // up, down, left, right

	Chunk() {
		resetBlockData();
		isBuilt = false;
	}

	// initiate block data to empty
	void resetBlockData() {
		memset(blockData, BlockType::AIR, CHUNK_SIZE * HEIGHT_LIMIT * CHUNK_SIZE);
		isBuilt = false;
		used = false;
		structuresPlaced = false;
		neighbors[NEIGHBOR_UP] = nullptr;
		neighbors[NEIGHBOR_DOWN] = nullptr;
		neighbors[NEIGHBOR_LEFT] = nullptr;
		neighbors[NEIGHBOR_RIGHT] = nullptr;
	}

	// fill with test chunk data
	void fillChunk() {

		
		for (int x = 0; x < CHUNK_SIZE; x++) {
			for (int y = 0; y < 5; y++) {
				for (int z = 0; z < CHUNK_SIZE; z++) {
					setBlock(x, y, z, BlockType::STONE);
				}
			}
		}

		setBlock(5, 5, 5, BlockType::LOG);
		setBlock(5, 6, 5, BlockType::LOG);
		setBlock(5, 7, 5, BlockType::LOG);
		setBlock(5, 8, 5, BlockType::LOG);
		setBlock(6, 8, 5, BlockType::LEAVES);
		setBlock(6, 8, 6, BlockType::LEAVES);
		setBlock(6, 8, 4, BlockType::LEAVES);
		setBlock(5, 8, 6, BlockType::LEAVES);
		setBlock(5, 8, 4, BlockType::LEAVES);
		setBlock(4, 8, 4, BlockType::LEAVES);
		setBlock(4, 8, 5, BlockType::LEAVES);
		setBlock(4, 8, 6, BlockType::LEAVES);
		setBlock(5, 9, 5, BlockType::LEAVES);

		setBlock(8, 5, 8, BlockType::LEAVES);
		setBlock(9, 5, 8, BlockType::LEAVES);
		setBlock(9, 6, 8, BlockType::LEAVES);

		setBlock(10, 9, 5, BlockType::GRASS);

		setBlock(8, 4, 8, BlockType::DEBUG);

		setBlock(0, 4, 0, BlockType::SAND);
		setBlock(15, 4, 0, BlockType::SAND);
		setBlock(0, 4, 15, BlockType::SAND);
		setBlock(15, 4, 15, BlockType::SAND);

		/*setBlock(5, 5, 5, BlockType::DEBUG);
		setBlock(6, 5, 5, BlockType::DEBUG);
		setBlock(5, 5, 6, BlockType::DEBUG);
		setBlock(5, 5, 4, BlockType::DEBUG);
		setBlock(4, 5, 5, BlockType::DEBUG);
		setBlock(5, 6, 5, BlockType::DEBUG);
		setBlock(5, 4, 5, BlockType::DEBUG);*/
	}

	void setBlock(int x, int y, int z, BlockType type) {

		if (x < 0 || y < 0 || z < 0 || x > CHUNK_SIZE - 1 || y > HEIGHT_LIMIT - 1 || z > CHUNK_SIZE - 1)
			return;

		blockData[x][y][z] = type;
	}

	// places a block if there isn't already one there
	void setBlockWithCheck(int x, int y, int z, BlockType type) {

		if (blockData[x][y][z] == BlockType::AIR) {
			setBlock(x, y, z, type);
		}
	}

	// try to place a block in the chunk
	// if the coords are out of bounds, returns a NEIGHBOR index for the world class to process
	int setBlockWithNeighbors(int x, int y, int z, BlockType type, int *xChunk, int *zChunk) {

		// if not placeable in this chunk, change chunks until it is placeable

		if (y < 0 || y > HEIGHT_LIMIT - 1)
			return BLOCK_NOT_PLACED;

		if (x < 0) {
			*xChunk -= 1;
			setBlockWithNeighbors(x + 16, y, z, type, xChunk, zChunk);
			return BLOCK_NOT_PLACED;
		}

		if (x > CHUNK_SIZE - 1) {
			*xChunk += 1;
			setBlockWithNeighbors(x - 16, y, z, type, xChunk, zChunk);
			return BLOCK_NOT_PLACED;
		}

		if (z < 0) {
			*zChunk -= 1;
			setBlockWithNeighbors(x, y, z + 16, type, xChunk, zChunk);
			return BLOCK_NOT_PLACED;
		}

		if (z > CHUNK_SIZE - 1) {
			*zChunk += 1;
			setBlockWithNeighbors(x, y, z - 16, type, xChunk, zChunk);
			return BLOCK_NOT_PLACED;
		}

		if (*xChunk == 0 && *zChunk == 0)
			setBlock(x, y, z, type);

		return BLOCK_PLACED;
	}

	int placeBlock(int x, int y, int z, BlockType type, int recalculateMeshes) {

		// if not placeable in this chunk, change chunks until it is placeable
		Chunk *currentChunk = this;

		if (y < 0 || y > HEIGHT_LIMIT - 1)
			return BLOCK_NOT_PLACED;

		// coordinates are not valid: need to change chunks
		if (x < 0) {
			if (currentChunk->neighbors[NEIGHBOR_LEFT] != NULL) {
				currentChunk = currentChunk->neighbors[NEIGHBOR_LEFT];
				x = x + 16;
			}
			else return BLOCK_NOT_PLACED;
		}
		if (x >= CHUNK_SIZE) {
			if (currentChunk->neighbors[NEIGHBOR_RIGHT] != NULL) {
				currentChunk = currentChunk->neighbors[NEIGHBOR_RIGHT];
				x = x - 16;
			}
			else return BLOCK_NOT_PLACED;
		}
		if (z < 0) {
			if (currentChunk->neighbors[NEIGHBOR_DOWN] != NULL) {
				currentChunk = currentChunk->neighbors[NEIGHBOR_DOWN];
				z = z + 16;
			}
			else return BLOCK_NOT_PLACED;
		}
		if (z >= CHUNK_SIZE) {
			if (currentChunk->neighbors[NEIGHBOR_UP] != NULL) {
				currentChunk = currentChunk->neighbors[NEIGHBOR_UP];
				z = z - 16;
			}
			else return BLOCK_NOT_PLACED;
		}

		currentChunk->setBlock(x, y, z, type);

		if (recalculateMeshes) {
			recalculateNeighboringMeshes(x, z);
		}

		return BLOCK_PLACED;
	}

	// breaks a block and recalculates meshes if needed
	void breakBlock(int x, int y, int z, int recalculateMeshes) {

		if (x < 0 || y < 0 || z < 0 || x > CHUNK_SIZE - 1 || y > HEIGHT_LIMIT - 1 || z > CHUNK_SIZE - 1)
			return;

		blockData[x][y][z] = BlockType::AIR;
		used = true; // chunk has now been modified

		if (recalculateMeshes) {
			recalculateNeighboringMeshes(x, z);
		}
	}

	Block getBlock(int x, int y, int z) {
		if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < HEIGHT_LIMIT && z >= 0 && z < CHUNK_SIZE) {
			return blockData[x][y][z];
		}
		else return BlockType::AIR;
	}

	Block getBlockWithNeighbors(int x, int y, int z) {

		Chunk *currentChunk = this;

		if (y < 0 || y >= HEIGHT_LIMIT) {
			return BlockType::AIR;
		}

		if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < HEIGHT_LIMIT && z >= 0 && z < CHUNK_SIZE) {
			return blockData[x][y][z];
		}
		
		// not valid: need to change chunks
		if (x < 0) {
			if (currentChunk->neighbors[NEIGHBOR_LEFT] != NULL) {
				currentChunk = currentChunk->neighbors[NEIGHBOR_LEFT];
				x = x + 16;
			}
			else return BlockType::AIR;
		}
		if (x >= CHUNK_SIZE) {
			if (currentChunk->neighbors[NEIGHBOR_RIGHT] != NULL) {
				currentChunk = currentChunk->neighbors[NEIGHBOR_RIGHT];
				x = x - 16;
			}
			else return BlockType::AIR;
		}
		if (z < 0) {
			if (currentChunk->neighbors[NEIGHBOR_DOWN] != NULL) {
				currentChunk = currentChunk->neighbors[NEIGHBOR_DOWN];
				z = z + 16;
			}
			else return BlockType::AIR;
		}
		if (z >= CHUNK_SIZE) {
			if (currentChunk->neighbors[NEIGHBOR_UP] != NULL) {
				currentChunk = currentChunk->neighbors[NEIGHBOR_UP];
				z = z - 16;
			}
			else return BlockType::AIR;
		}

		return currentChunk->getBlock(x, y, z);
	}

	void recalculateNeighboringMeshes(int x, int z) {
		calculateMesh();
		// calculate the neighbouring chunk's meshes too
		if (x == CHUNK_SIZE - 1) {
			if (neighbors[NEIGHBOR_RIGHT] != NULL) {
				neighbors[NEIGHBOR_RIGHT]->calculateMesh();
			}
		}
		if (x == 0) {
			if (neighbors[NEIGHBOR_LEFT] != NULL) {
				neighbors[NEIGHBOR_LEFT]->calculateMesh();
			}
		}
		if (z == CHUNK_SIZE - 1) {
			if (neighbors[NEIGHBOR_UP] != NULL) {
				neighbors[NEIGHBOR_UP]->calculateMesh();
			}
		}
		if (z == 0) {
			if (neighbors[NEIGHBOR_DOWN] != NULL) {
				neighbors[NEIGHBOR_DOWN]->calculateMesh();
			}
		}
	}

	void render(Shader* shader) {

		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		model = glm::translate(model, glm::vec3(position.x * CHUNK_SIZE, 0.0f, position.y * CHUNK_SIZE));
		shader->setMat4("model", model);

		glBindVertexArray(VAO);

		glDrawArrays(GL_TRIANGLES, 0, n_meshTriangles);
	}

	/* check if a face has no solid block in front of it */
	int checkFaceFree(int x, int y, int z, int face) {

		/* order: back, front, left, right, bottom, top */
		switch (face) {

		case 0: // back
			if (z - 1 < 0) {
				if (neighbors[NEIGHBOR_DOWN] != NULL) {
					if (isSolid(neighbors[NEIGHBOR_DOWN]->getBlock(x, y, 15))) {
						return 0;
					}
				}
				else return 1;
			}
			if (isSolid(getBlock(x, y, z - 1))) {
				return 0;
			}
			break;
		case 1: // front
			if (z + 1 == CHUNK_SIZE) {
				if (neighbors[NEIGHBOR_UP] != NULL) {
					if (isSolid(neighbors[NEIGHBOR_UP]->getBlock(x, y, 0))) {
						return 0;
					}
				}
				else return 1;
			}
			if (isSolid(getBlock(x, y, z + 1))) {
				return 0;
			}
			break;
		case 2: // left
			if (x - 1 < 0) {
				if (neighbors[NEIGHBOR_LEFT] != NULL) {
					if (isSolid(neighbors[NEIGHBOR_LEFT]->getBlock(15, y, z))) {
						return 0;
					}
				}
				else return 1;
			}
			if (isSolid(getBlock(x - 1, y, z))) {
				return 0;
			}
			break;
		case 3: // right
			if (x + 1 == CHUNK_SIZE) {
				if (neighbors[NEIGHBOR_RIGHT] != NULL) {
					if (isSolid(neighbors[NEIGHBOR_RIGHT]->getBlock(0, y, z))) {
						return 0;
					}
				}
				else return 1;
			}
			if (isSolid(getBlock(x + 1, y, z))) {
				return 0;
			}
			break;
		case 4: // bottom
			if (y - 1 < 0) {
				return 1;
			}
			if (isSolid(getBlock(x, y - 1, z))) {
				return 0;
			}
			break;
		case 5: // top
			if (y + 1 == HEIGHT_LIMIT) {
				return 1;
			}
			if (isSolid(getBlock(x, y + 1, z))) {
				return 0;
			}
			break;
		}

		return 1;
	}

	BiomeType getBiome(float temperature, float humidity) {
		if (temperature < 0.5f) {
			if (humidity < 0.5f) {
				return BiomeType::PLAINS;
			}
			else {
				return BiomeType::FOREST;
			}
		}
		else {
			if (humidity < 0.5f) {
				return BiomeType::DESERT;
			}
			else {
				return BiomeType::JUNGLE;
			}
		}
		/*if (humidity < 0.7f) {
			return BiomeType::PLAINS;
		}
		else {
			return BiomeType::FOREST;
		}*/
	}

	// calculates the ambient occlusion value (4 possible values) for a vertex
	float calculateAO(glm::vec3 vert, glm::ivec3 blockPos) {
		// calculate "direction" of block center to vertex position
		glm::ivec3 v = glm::ivec3(vert.x * 2, vert.y * 2, vert.z * 2);

		glm::ivec3 cornerPos = blockPos + v;
		glm::ivec3 sidePos1 = blockPos + glm::ivec3(v.x, v.y, 0);
		glm::ivec3 sidePos2 = blockPos + glm::ivec3(0, v.y, v.z);

		int side1 = isSolid(getBlockWithNeighbors(sidePos1.x, sidePos1.y, sidePos1.z));
		int side2 = isSolid(getBlockWithNeighbors(sidePos2.x, sidePos2.y, sidePos2.z));
		int corner = isSolid(getBlockWithNeighbors(cornerPos.x, cornerPos.y, cornerPos.z));
		
		if (side1 && side2) {
			return 0.7f; // 0
		}
		else if ((side1 && corner) || (side2 && corner)) {
			return 0.8f; // 1
		}
		else if (side1 || corner || side2) {
			return 0.9f; // 2
		}
		else return 1.0f; // 3
	}

	void calculateMesh() {

		/* DATA IS: 3 float (pos), 2 float (texcoord), 2 float (tex offset) */
		/* OPTIMIZE: MAKE LAST FLOATS INT? */

		float *data;

		// N_FACE_DATA + 1 is because ambient occlusion (one float) was added
		data = (float*)malloc(CHUNK_SIZE * CHUNK_SIZE * HEIGHT_LIMIT * (N_FACE_DATA + 1) * sizeof(float));
		if (data == NULL) {
			std::cout << "Error calculateMesh(): could not reserve memory\n";
		}

		n_meshTriangles = 0;
		int dSize = 0;

		float start_time = static_cast<float>(glfwGetTime());

		for (int x = 0; x < CHUNK_SIZE; x++) {
			for (int y = 0; y < HEIGHT_LIMIT; y++) {
				for (int z = 0; z < CHUNK_SIZE; z++) {
					
					// for each block
					Block block = getBlock(x, y, z);
					if (block != BlockType::AIR) {

						// calculate local position of the block (in the chunk)
						glm::vec3 blockPos = glm::vec3(x, y, z);

						/* for each face */
						/* order: back, front, left, right, bottom, top */
						int end = blockMesh(block) ? 6 : 2;
						for (int i = 0; i < end; i++) {

							if (blockMesh(block)) {
								// block mesh
								if (checkFaceFree(blockPos.x, blockPos.y, blockPos.z, i)) {
									for (int j = 0; j < N_FACE_DATA; j += 5) {

										// position
										data[dSize++] = (faceData[i][j] + blockPos.x);
										data[dSize++] = (faceData[i][j + 1] + blockPos.y);
										data[dSize++] = (faceData[i][j + 2] + blockPos.z);
										n_meshTriangles++;
										// texture uv
										data[dSize++] = (faceData[i][j + 3]);
										data[dSize++] = (faceData[i][j + 4]);
										// texture offset
										data[dSize++] = (faceTexture[block][i].x);
										data[dSize++] = (faceTexture[block][i].y);
										// ambient occlusion
										// get the point 
										data[dSize++] = calculateAO(glm::vec3(faceData[i][j],
											faceData[i][j + 1],
											faceData[i][j + 2]),
											blockPos);
									}
								}
							}
							else {
								// cross mesh
								for (int j = 0; j < N_FACE_DATA; j += 5) {

									// position
									data[dSize++] = (crossFaceData[i][j] + blockPos.x);
									data[dSize++] = (crossFaceData[i][j + 1] + blockPos.y);
									data[dSize++] = (crossFaceData[i][j + 2] + blockPos.z);
									n_meshTriangles++;
									// texture uv
									data[dSize++] = (crossFaceData[i][j + 3]);
									data[dSize++] = (crossFaceData[i][j + 4]);
									// texture offset
									data[dSize++] = (faceTexture[block][i].x);
									data[dSize++] = (faceTexture[block][i].y);
									// ambient occlusion (always 1.0 for cross meshes)
									data[dSize++] = 1.0;
								}
							}
						}
					}

				}
			}
		}

		// now translate data for OpenGL

		int dataSize = dSize;
		data = (float*)realloc(data, dSize * sizeof(float));

		/*
		std::cout << "(" << static_cast<float>(glfwGetTime()) - start_time << ") ";
		std::cout << "mesh built | dataSize = " << dataSize << " - ";
		std::cout << "n_meshTriangles = " << n_meshTriangles << "\n";
		*/


		// make opengl data

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(float), data, GL_STATIC_DRAW);


		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// texture coord attribute
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture offset attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);
		// ambient occlusion attribute
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
		glEnableVertexAttribArray(3);

		isBuilt = true;
	}

	void removeNeighbors() {
		// reset neighbors to avoid pointers referencing nothing
		if (neighbors[NEIGHBOR_DOWN] != nullptr) {
			neighbors[NEIGHBOR_DOWN]->neighbors[NEIGHBOR_UP] = nullptr;
		}
		if (neighbors[NEIGHBOR_UP] != nullptr) {
			neighbors[NEIGHBOR_UP]->neighbors[NEIGHBOR_DOWN] = nullptr;
		}
		if (neighbors[NEIGHBOR_LEFT] != nullptr) {
			neighbors[NEIGHBOR_LEFT]->neighbors[NEIGHBOR_RIGHT] = nullptr;
		}
		if (neighbors[NEIGHBOR_RIGHT] != nullptr) {
			neighbors[NEIGHBOR_RIGHT]->neighbors[NEIGHBOR_LEFT] = nullptr;
		}

		// free(this);
	}

	int isSolid(Block block) {
		if (block == BlockType::AIR || block == BlockType::HERB) {
			return 0;
		}
		else return 1;
	}

	int blockMesh(Block block) {
		if (block == BlockType::HERB) {
			return 0;
		}
		else return 1;
	}

private:
	Block blockData[CHUNK_SIZE][HEIGHT_LIMIT][CHUNK_SIZE];

};

#endif /* _CHUNK_H_ */