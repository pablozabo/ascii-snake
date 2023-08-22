#ifndef VECTOR_H
#define VECTOR_H

#include "../common.h"
#include "../defs.h"

#define VECTOR_CHUNK_SIZE 2048

void *vector_realloc(void *vec, size_t type_size, size_t chunk_size);
void  vector_remove(void *vec, size_t type_size, uint32_t index);

#define VECTOR_PUSH(vec, val) (_VECTOR_CHECK(vec) ? ((vec)[_VECTOR_HEADER(vec)[1]++] = (val)), 1 : 0)
#define VECTOR_REMOVE(vec, index) ((vec) ? vector_remove(vec, sizeof(*vec), index), 1 : 0)
#define VECTOR_SIZE(vec) ((vec) ? _VECTOR_HEADER(vec)[0] : 0)
#define VECTOR_LENGTH(vec) ((vec) ? _VECTOR_HEADER(vec)[1] : 0)
#define VECTOR_CHECK(vec, index) (((int32_t)index < 0 || index >= VECTOR_LENGTH(vec)) ? false : true)
#define VECTOR_CLEAR(vec) ((vec) ? _VECTOR_HEADER(vec)[1] = 0, 1 : 0)
#define VECTOR_DISPOSE(vec) (_VECTOR_DISPOSE(vec) ? (vec) = 0, 1 : 0)

#define _VECTOR_HEADER(_vec) ((uint32_t *)(_vec)-2)
#define _VECTOR_CHECK(_vec) (_vec == 0 || VECTOR_LENGTH(_vec) >= VECTOR_SIZE(_vec) ? (*((void **)&(_vec)) = vector_realloc(_vec, sizeof(*_vec), VECTOR_CHUNK_SIZE)), 1 : 1)
#define _VECTOR_DISPOSE(vec) ((vec) ? free(_VECTOR_HEADER(vec)), 1 : 0)

#endif
