#include "vector.h"

void *vector_realloc(void *vec, size_t type_size, size_t chunk_size)
{
	uint32_t  size	   = VECTOR_SIZE(vec);
	uint32_t  length   = VECTOR_LENGTH(vec);
	uint32_t  new_size = size == 0 ? chunk_size : size * 2;
	uint32_t *header   = vec ? _VECTOR_HEADER(vec) : 0;

	uint32_t *result = (uint32_t *)realloc(header, (new_size * type_size) + (2 * sizeof(uint32_t)));

	ASSERT(result);

	result[0] = new_size;
	result[1] = length;

	return (result + 2);
}

void vector_remove(void *vec, size_t type_size, uint32_t index)
{
	uint32_t  size	 = VECTOR_SIZE(vec);
	uint32_t  length = VECTOR_LENGTH(vec);
	uint32_t *header = vec ? _VECTOR_HEADER(vec) : 0;

	if (size == 0 || length == 0)
	{
		return;
	}

	if (length > 1 && index < (length - 1))
	{
		size_t offset	   = (index * type_size);
		size_t offset_last = ((length - 1) * type_size);
		memmove(vec + offset, vec + offset_last, type_size);
	}

	header[1]--;
}