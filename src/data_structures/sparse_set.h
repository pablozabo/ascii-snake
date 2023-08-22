#ifndef SPARSE_SET_H
#define SPARSE_SET_H

#include "../common.h"
#include "../defs.h"
#include "vector.h"

typedef struct
{
	uint32_t  chunk_size;
	uint32_t  sparse_size;
	uint32_t *sparse;
	uint32_t *dense;
} sparse_set_t;

#define SPARSE_SET_CHECK(sparse_length, id) (((int32_t)id < 0 || (int32_t)id > (int32_t)(sparse_length - 1)) ? false : true)

#define SPARSE_SET_CONTAINS(sparse_set, id) (                \
	SPARSE_SET_CHECK(sparse_set.sparse_size, id) &&          \
	VECTOR_CHECK(sparse_set.dense, sparse_set.sparse[id]) && \
	sparse_set.dense[sparse_set.sparse[id]] == (uint32_t)id)

#define SPARSE_SET_INDEXOF(sparse_set, id) (SPARSE_SET_CONTAINS(sparse_set, id) ? (int32_t)sparse_set.sparse[id] : -1)

sparse_set_t sparse_set_new(uint32_t chunk_size);
void		 sparse_set_dispose(sparse_set_t *sparse_set);
void		 sparse_set_add(sparse_set_t *sparse_set, uint32_t id);
void		 sparse_set_remove(sparse_set_t *sparse_set, uint32_t id);
uint32_t	 sparse_set_pop(sparse_set_t *sparse_set);
void		 sparse_set_clear(sparse_set_t *sparse_set);

#endif
