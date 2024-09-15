#ifndef _UTILS_H_
#define _UTILS_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "chunk.h"

glm::ivec2 getChunkPosition(glm::vec3 *position);

#endif /* _UTILS_H_ */
