#include "sparse_set.h"

sparse_set_t sparse_set_new(uint32_t chunk_size)
{
	sparse_set_t sparse_set;

	sparse_set.chunk_size  = chunk_size;
	sparse_set.dense	   = 0;
	sparse_set.sparse	   = (uint32_t *)malloc(chunk_size * sizeof(uint32_t));
	sparse_set.sparse_size = chunk_size;

	ASSERT(sparse_set.sparse);

	return sparse_set;
}

void sparse_set_dispose(sparse_set_t *sparse_set)
{
	VECTOR_DISPOSE(sparse_set->dense);
	free(sparse_set->sparse);
	sparse_set->sparse = NULL;
}

void sparse_set_add(sparse_set_t *sparse_set, uint32_t id)
{
	if (id >= sparse_set->sparse_size)
	{
		uint32_t size_needed = id - sparse_set->sparse_size + 1;

		if (size_needed < sparse_set->chunk_size)
		{
			size_needed = sparse_set->chunk_size;
		}

		size_needed += sparse_set->sparse_size;
		uint32_t *temp = (uint32_t *)realloc(sparse_set->sparse, sizeof(uint32_t) * size_needed);

		ASSERT(temp);

		sparse_set->sparse		= temp;
		sparse_set->sparse_size = size_needed;
	}

	if (!SPARSE_SET_CONTAINS((*sparse_set), id))
	{
		VECTOR_PUSH(sparse_set->dense, id);
		sparse_set->sparse[id] = VECTOR_LENGTH(sparse_set->dense) - 1;
	}
}

void sparse_set_remove(sparse_set_t *sparse_set, uint32_t id)
{
	if (!SPARSE_SET_CONTAINS((*sparse_set), id))
	{
		return;
	}

	uint32_t index	= sparse_set->sparse[id];
	uint32_t length = VECTOR_LENGTH(sparse_set->dense);

	if (length > 1 && index < (length - 1))
	{
		uint32_t last_value			   = sparse_set->dense[length - 1];
		sparse_set->sparse[last_value] = index;
	}

	sparse_set->sparse[id] = 0;
	VECTOR_REMOVE(sparse_set->dense, index);
}

uint32_t sparse_set_pop(sparse_set_t *sparse_set)
{
	uint32_t length = VECTOR_LENGTH(sparse_set->dense);

	if (length == 0)
	{
		return 0;
	}

	uint32_t result = sparse_set->dense[length - 1];
	sparse_set_remove(sparse_set, result);

	return result;
}

void sparse_set_clear(sparse_set_t *sparse_set)
{
	VECTOR_CLEAR(sparse_set->dense);
	memset(sparse_set->sparse, 0, sparse_set->sparse_size * sizeof(uint32_t));
}
