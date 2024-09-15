#ifndef _WORLD_H_
#define _WORLD_H_

#include "shader.h"
#include "chunkmanager.h"

#include <map>
#include <random>
#include <chrono>

#include <FastNoise/FastNoise.h>

#define SUN_MOON_DISTANCE 150
#define MAX_TIME 3600
#define TIME_SPEED 250
#define DO_WORLD_PASS_TIME false

#define PI_6 (3.14 / 6) // pi / 6

// a block that is in a chunk that has not been generated yet
struct CachedBlock {
	BlockType type;
	int x;
	int y;
	int z;
};

class World {

public:

	ChunkManager chunkManager;

	// uses a hashcode for int, and vector of cached blocks for each chunk
	std::map<int, std::vector<CachedBlock>> cachedBlocks;

	float time; // world time (between 0 and 3600)
	float timeSpeed; // speed to update time



	World() {}

	void initNoise() {
		simplex = FastNoise::New<FastNoise::Simplex>();
		fractal = FastNoise::New<FastNoise::FractalFBm>();
		scale = FastNoise::New<FastNoise::DomainScale>();

		fractal->SetSource(simplex);
		fractal->SetOctaveCount(4);
		fractal->SetLacunarity(1.3f);
		fractal->SetGain(0.9f);
		fractal->SetWeightedStrength(0.5f);

		scale->SetSource(fractal);
		scale->SetScale(0.8f);
	}

	void init() {
		chunkManager.init();
		chunkManager.world = this;

		seed = getRandom(0, 3500);

		std::cout << "seed = " << seed << "\n";

		time = 0; // sunrise
		timeSpeed = TIME_SPEED;

		initNoise();
	}

	void worldUpdate(Camera *camera, float deltaTime, int playerMoved) {

		chunkManager.update(camera, playerMoved);

		// update world time
		if (DO_WORLD_PASS_TIME) {
			time += timeSpeed * deltaTime;
			if (time > MAX_TIME) {
				time = time - MAX_TIME;
			}
		}
		else {
			time = 1400;
		}
	}

	template <typename T>
	T interpolate(T a, T b, float t, float max) {
		return a * (t / max) + b * ((max - t) / t);
	}

	float calculateSunlight(float time) {
		//std::cout << "time:" << time << "\n";
		float light = 0;
		if (time < 600) {
			light = (time / 600) * 0.7f;
		}
		else if (time < 1800) {
			light = 0.7f; // day
		}
		else if (time < 2400) { // 2400 too
			light = ((600 - (time - 1800)) / 600) * 0.7f;
		}
		else if (time < 3600) {
			light = 0;
		}

		return 0.3f + light;
	}

	glm::vec3 calculateSkyColor(float time) {
		glm::vec3 day{ 0.46f, 0.89f, 1.0f };
		glm::vec3 night{ 8.0f / 255.0f, 7.0f / 255.0f, 41.0f / 255.0f };
		glm::vec3 orange{ 227.0f / 255.0f, 128.0f / 255.0f, 16.0f / 255.0f };

		if (time < 300) {
			return interpolate(orange, night, time, 300);
		}
		if (time < 600) {
			return day * ((time - 300) / 300) + orange * ((300 - (time - 300)) / 300);
		}
		else if (time < 1800) {
			return day;
		}
		else if (time < 2100) {
			return orange * ((time - 1800) / 300) + day * ((300 - (time - 1800)) / 300);
		}
		else if (time < 2400) {
			return night * ((time - 2100) / 300) + orange * ((300 - (time - 2100)) / 300);
		}
		else if (time < 3600) {
			return night;
		}
	}

	void renderWorld(Shader *chunkShader, Shader *blockShader, BlockModel *blockModel, Camera *camera) {

		// render sun and moon first (so they appear behind)
		blockModel->renderBlock(blockShader,
			camera->Position + glm::vec3(cos((time / MAX_TIME) * 6.38f - PI_6) * SUN_MOON_DISTANCE,
				sin((time / MAX_TIME) * 6.38f - PI_6) * SUN_MOON_DISTANCE, 0),
			glm::vec3(0, 0, sin((time / MAX_TIME) * 6.38f - PI_6)),
			glm::vec3(20),
			BlockType::SUN);

		blockModel->renderBlock(blockShader,
			camera->Position + glm::vec3(cos((time / MAX_TIME) * 6.38f - PI_6 + 3.14f) * SUN_MOON_DISTANCE,
				sin((time / MAX_TIME) * 6.38f - PI_6 + 3.14f) * SUN_MOON_DISTANCE, 0),
			glm::vec3(0, 0, sin((time / MAX_TIME) * 6.38f - PI_6 + 3.14f)),
			glm::vec3(20),
			BlockType::MOON);

		// set sunLight uniform
		float sunLight = calculateSunlight(time);
		chunkShader->use();
		chunkShader->setFloat("sunLight", sunLight);

		// render chunks
		chunkManager.renderChunks(chunkShader);
	}

	void requestNoiseGen(std::vector<float> *vect, int xStart, int yStart, int xSize, int ySize, float frequency, float noise_scale, int noise_seed) {
		if (noise_seed == -1) {
			noise_seed = seed; // use world seed if not specified
		}
		else {
			noise_seed += seed;
		}
		scale->SetScale(noise_scale);
		scale->GenUniformGrid2D(vect->data(), xStart, yStart, xSize, ySize, frequency, noise_seed);
	}

	int getChunkPosHash(int x, int z) {
		// return (x << 16 + y);
		return (x * 1000 + z);
	}

	// add a cached block in chunk of pos (xChunk, zChunk) at pos (x, y, z)
	void addCachedBlock(BlockType type, int x, int y, int z, int xChunk, int zChunk) {
		int hash = getChunkPosHash(xChunk, zChunk);
		cachedBlocks[hash].push_back(CachedBlock{ type, x, y, z });
	}

	void placeStructure(Chunk *chunk, Structure s, int x, int y, int z) {

		x += s.offset.x;
		y += s.offset.y;
		z += s.offset.z;

		// iterate over the structure's data
		for (int y_s = 0; y_s < s.dim.y; y_s++) {
			int index = 0;
			for (int z_s = 0; z_s < s.dim.z; z_s++) {
				for (int x_s = 0; x_s < s.dim.x; x_s++) {

					BlockType block = static_cast<BlockType>(s.blocks[y_s][index]);

					if (block != BlockType::AIR) {

						// block position
						int xb = x + x_s - s.dim.x / 2;
						int yb = y + y_s;
						int zb = z + z_s - s.dim.z / 2;

						// start with xChunk and zChunk = 0 (no neighbour offset)
						int xChunk = 0;
						int zChunk = 0;
						int set = chunk->setBlockWithNeighbors(xb, yb, zb, block, &xChunk, &zChunk);

						// block = BlockType::DEBUG_X;
						// if we had to move chunks to place the block
						if (set == BLOCK_NOT_PLACED) {
							addCachedBlock(block,
								abs(xb - 16 * xChunk) % 16,
								yb,
								abs(zb - 16 * zChunk) % 16,
								chunk->position.x + xChunk,
								chunk->position.y + zChunk);
						}
					}
					index++;
				}
			}
		}
	}

	int fitContinentalness(float x) {
		if (x < 0.3) {
			return 7.692*x + 17.69;
		}
		if (x < 0.4) {
			return 300*x - 70;
		}
		if (x < 1.0) {
			return 8.33*x + 46.67;
		}
	}

	void placeCactus(Chunk *chunk, int x, int y, int z, int height) {
		for (int i = 0; i < height; i++) {
			chunk->setBlock(x, y + i, z, BlockType::CACTUS);
		}
	}

	void generateChunk(Chunk *chunk, bool generateTrees) {

		


		std::vector<float> noise(16 * 16);

		std::vector<float> temperature(16 * 16);
		std::vector<float> humidity(16 * 16);

		std::vector<float> continentalness(16 * 16);
		std::vector<float> erosion(16 * 16);

		requestNoiseGen(&noise, chunk->position.x * CHUNK_SIZE, chunk->position.y * CHUNK_SIZE, 16, 16, 0.02f, 0.8f, -1);
		requestNoiseGen(&temperature, chunk->position.x * CHUNK_SIZE, chunk->position.y * CHUNK_SIZE, 16, 16, 0.005f, 0.3f, 1820);
		requestNoiseGen(&humidity, chunk->position.x * CHUNK_SIZE, chunk->position.y * CHUNK_SIZE, 16, 16, 0.02f, 0.3f, 2067);
		requestNoiseGen(&continentalness, chunk->position.x * CHUNK_SIZE, chunk->position.y * CHUNK_SIZE, 16, 16, 0.01f, 0.5f, 920);

		int index = 0;

		int hasTower = 0;

		// APPLY NOISE
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				// int value = static_cast<int>((noise[index] * 1 + 1) * 13);
				int value = static_cast<int>((noise[index] + 1) * 13);
				value += static_cast<int>(fitContinentalness(continentalness[index]));

				BiomeType biome = chunk->getBiome(temperature[index] + 0.5f, humidity[index] + 0.5f);

				switch (biome) {
				case BiomeType::PLAINS:
					for (int y = 0; y < value - 1; y++) {
						chunk->setBlock(x, y, z, BlockType::DIRT);
					}
					chunk->setBlock(x, value - 1, z, BlockType::GRASS);

					if (!hasTower && getRandom(0, 10000) < 1) {
						placeStructure(chunk, tower, x, value, z);
						hasTower = 1;
					}

					break;
				case BiomeType::FOREST:
					for (int y = 0; y < value - 1; y++) {
						chunk->setBlock(x, y, z, BlockType::DIRT);
					}
					chunk->setBlock(x, value - 1, z, BlockType::GRASS);

					if (generateTrees) {

						// trees
						if (getRandom(0, 100) < 1) {
							placeStructure(chunk, tree, x, value, z);
						}

					}
					break;
				case BiomeType::DESERT:
					for (int y = 0; y < value - 1; y++) {
						chunk->setBlock(x, y, z, BlockType::STONE);
					}
					chunk->setBlock(x, value - 1, z, BlockType::SAND);

					break;
				case BiomeType::JUNGLE:
					for (int y = 0; y < value - 1; y++) {
						chunk->setBlock(x, y, z, BlockType::STONE);
					}
					chunk->setBlock(x, value - 1, z, BlockType::SAND);

					// cactus
					if (getRandom(0, 80) < 1) {
						placeCactus(chunk, x, value, z, getRandom(2, 5));
					}
					break;
				}

				if (value <= 0) {
					chunk->setBlock(x, 0, z, BlockType::SAND);
				}

				// surface-level layer

				// herb
				if (biome == BiomeType::PLAINS || biome == BiomeType::FOREST) {
					if (getRandom(0, 2) < 1)
						chunk->setBlockWithCheck(x, value, z, BlockType::HERB);
				}
				// rocks
				if (biome == BiomeType::DESERT || biome == BiomeType::JUNGLE
					&& continentalness[index] < 0.3f) {
					if (getRandom(0, 1000) < 1)
						placeStructure(chunk, rock, x, value, z);
				}

				index++;
			}
		}

		/*
		// add blocks that were cached (placed by other chunks before this one was loaded)
		int hash = getChunkPosHash(chunk->position.x, chunk->position.y);
		if (cachedBlocks.find(hash) != cachedBlocks.end()) { // if we found an entry for this chunk
			int size = cachedBlocks[hash].size();
			for (int i = 0; i < size; i++) {
				chunk->setBlock(cachedBlocks[hash][i].x, cachedBlocks[hash][i].y, cachedBlocks[hash][i].z, cachedBlocks[hash][i].type);
			}
			cachedBlocks.erase(hash);

			// need to regenerate chunk mesh
			chunkManager.unbuiltChunks.push_back(chunk);
		}
		*/
	}

	int getRandom(int min, int max) {
		return std::uniform_int_distribution<int>{ min, max }(mt);
	}

	// returns the chunk and sets the x, y, z parameters
	Chunk *worldPosToChunkPos(int x, int y, int z, int *xChunk, int *yChunk, int *zChunk) {

	}

private:

	// noise
	FastNoise::SmartNode<FastNoise::Simplex> simplex;
	FastNoise::SmartNode<FastNoise::FractalFBm> fractal;
	FastNoise::SmartNode<FastNoise::DomainScale> scale;
	int seed;

	// random
	std::random_device rd{};
	std::seed_seq ss{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
	std::mt19937 mt{ ss };
};

#endif /* _WORLD_H_ */