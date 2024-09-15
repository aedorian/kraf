#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"


#define N_FACE_DATA 30 // number of floats in a single face data

using Block = unsigned char;

struct BlockFace {
	int x;
	int y;
};

enum BlockType {
	AIR,		// 0
	STONE,
	DIRT,
	GRASS,
	SAND,
	DEBUG,
	LOG,
	LEAVES,
	DEBUG_X,
	DEBUG_Y,
	DEBUG_Z,
	COBBLE,
	PLANKS,
	HERB,
	CACTUS,
	SUN,
	MOON
};

struct Structure {
	int blocks[20][8 * 8]; // y, xz (from bottom to top)
	glm::ivec3 dim; // dimensions of the structure
	glm::ivec3 offset; // offset to the center block it has been placed in (ex: tower is in the ground a bit)
};





/* GAME DATA */

static float blockVertices[] = {
	// Back face
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	// Front face
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
	// Left face
	-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right 1.0f, 1.0f
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
	-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
	-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
	// Right face
	 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left     
	// Bottom face
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
	// Top face
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left        
};

static const float faceData[6][N_FACE_DATA] = {
	// Back face
	{
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	},
	// Front face
	{
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
	},
	// Left face
	{
	-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right 1.0f, 1.0f
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
	-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
	-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
	},
	// Right face
	{
	 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left  
	},
	// Bottom face
	{
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
	},
	// Top face
	{
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left   
	}
};

static const float crossFaceData[2][N_FACE_DATA] = {
	// Back face
	{
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
	 0.5f,  0.5f, 0.5f,  1.0f, 1.0f, // top-right
	 0.5f, -0.5f, 0.5f,  1.0f, 0.0f, // bottom-right         
	 0.5f,  0.5f, 0.5f,  1.0f, 1.0f, // top-right
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
	},
	// Front face
	{
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
	 0.5f, -0.5f,  -0.5f,  1.0f, 0.0f, // bottom-right
	 0.5f,  0.5f,  -0.5f,  1.0f, 1.0f, // top-right
	 0.5f,  0.5f,  -0.5f,  1.0f, 1.0f, // top-right
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
	}
};

/* define indices for each */
static BlockFace air = { 0, 0 };
static BlockFace stone = { 0, 5 };
static BlockFace dirt = { 1, 5 };
static BlockFace grass_side = { 2, 5 };
static BlockFace grass = { 3, 5 };
static BlockFace sand = { 4, 5 };
static BlockFace trunk = { 0, 4 };
static BlockFace trunk_middle = { 0, 3 };
static BlockFace leaves = { 1, 4 };
static BlockFace cobble = { 5, 5 };
static BlockFace planks = { 5, 4 };
static BlockFace herb = { 0, 2 };
static BlockFace cactus = { 1, 3 };

static BlockFace sun = { 5, 0 };
static BlockFace moon = { 4, 0 };

static BlockFace debug_x = { 2, 4 };
static BlockFace debug_y = { 3, 4 };
static BlockFace debug_z = { 4, 4 };

/* order: back, front, left, right, bottom, top */
static BlockFace faceTexture[][6]{
	{ air, air, air, air, air, air },										 // AIR 0
	{ stone, stone, stone, stone, stone, stone },							 // STONE 1
	{ dirt, dirt, dirt, dirt, dirt, dirt },									 // DIRT 2
	{ grass_side, grass_side, grass_side, grass_side, dirt, grass },		 // GRASS 3
	{ sand, sand, sand, sand, sand, sand },									 // SAND 4
	{ debug_x, debug_x, debug_z, debug_z, debug_y, debug_y },				 // DEBUG 5
	{ trunk, trunk, trunk, trunk, trunk_middle, trunk_middle },				 // LOG 6
	{ leaves, leaves, leaves, leaves, leaves, leaves },						 // LEAVES 7
	{ debug_x, debug_x, debug_x, debug_x, debug_x, debug_x },
	{ debug_y, debug_y, debug_y, debug_y, debug_y, debug_y },
	{ debug_z, debug_z, debug_z, debug_z, debug_z, debug_z },
	{ cobble, cobble, cobble, cobble, cobble, cobble },						 // COBBLE 11
	{ planks, planks, planks, planks, planks, planks },						 // PLANKS 12
	{ herb, herb },															 // HERB 13
	{ cactus, cactus, cactus, cactus, cactus, cactus },						 // CACTUS 14
	{ sun, sun, sun, sun, sun, sun },										 // SUN 15
	{ moon, moon, moon, moon, moon, moon }									 // MOON 16
};

// structures

static Structure tree = {
	{
		{0,0,0,0,0,
		0,0,0,0,0,
		0,0,6,0,0,
		0,0,0,0,0,
		0,0,0,0,0},

		{0,0,0,0,0,
		0,0,0,0,0,
		0,0,6,0,0,
		0,0,0,0,0,
		0,0,0,0,0},

		{0,0,0,0,0,
		0,0,0,0,0,
		0,0,6,0,0,
		0,0,0,0,0,
		0,0,0,0,0},

		{7,7,7,7,7,
		7,7,7,7,7,
		7,7,6,7,7,
		7,7,7,7,7,
		7,7,7,7,7},

		{7,7,7,7,0,
		7,7,7,7,7,
		7,7,6,7,7,
		7,7,7,7,7,
		0,7,7,7,7},

		{0,0,0,0,0,
		0,7,7,7,0,
		0,7,7,7,0,
		0,7,7,7,0,
		0,0,0,0,0},
	},
	{5, 6, 5},
	{0, 0, 0}
};

static Structure tower = {
	{
		{0,11,11,11,11,11,0,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		0,11,11,11,11,11,0},

		{0,11,11,11,11,11,0,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		0,11,11,11,11,11,0},

		{0,11,11,11,11,11,0,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		0,11,11,11,11,11,0},

		{0,11,11,11,11,11,0,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		0,11,11,11,11,11,0},

		{0,11,11,11,11,11,0,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		0,11,11,11,11,11,0},

		{0,11,11,11,11,11,0,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		0,11,11,11,11,11,0},

		{0,11,11,11,11,11,0,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		0,11,11,11,11,11,0},

		{0,11,11,11,11,11,0,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		0,11,11,11,11,11,0},

		{0,11,11,11,11,11,0,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		0,11,11,11,11,11,0},

		{0,11,11,11,11,11,0,
		11,12,12,12,12,12,11,
		11,12,12,12,12,12,11,
		11,12,12,12,12,12,11,
		11,12,12,12,12,12,11,
		11,12,12,12,12,12,11,
		0,11,11,11,11,11,0},

		{0,11,11,11,11,11,0,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		11,0,0,0,0,0,11,
		0,11,11,11,11,11,0},

		{0,11,0,11,0,11,0,
		11,0,0,0,0,0,11,
		0,0,0,0,0,0,0,
		11,0,0,0,0,0,11,
		0,0,0,0,0,0,0,
		11,0,0,0,0,0,11,
		0,11,0,11,0,11,0},
	},
	{7, 12, 7},
	{0, -3, 0}
};

static Structure rock = {
	{
		{1,1,1,0,
		1,1,1,1,
		1,1,1,1,
		0,1,1,1},

		{1,1,0,0,
		1,1,1,0,
		0,1,1,1,
		0,1,1,1},

		{0,1,0,0,
		1,1,1,0,
		0,1,1,0,
		0,1,0,0},

		{0,0,0,0,
		0,1,0,0,
		0,1,0,0,
		0,0,0,0}
	},
	{4, 4, 4},
	{0, 0, 0}
};

#endif /* _BLOCK_H_ */