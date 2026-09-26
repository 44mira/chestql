#include "arena.h"
#include <stdint.h>
#include <stdlib.h>

struct arena {
  void *mem;
  uint64_t pos;
  uint64_t size;
};

struct arena *arena_make(uint64_t size)
{
  struct arena *arena = (struct arena *)malloc(sizeof *arena);
  if (arena == NULL) {
    return NULL;
  }

  // allocate underlying memory
  arena->mem = malloc(size);
  if (arena->mem == NULL) {
    return NULL;
  }

  arena->pos = 0;
  arena->size = size;

  return arena;
}

void *arena_alloc(struct arena *allocator, uint64_t bytes)
{
  uint64_t alloc_pos = allocator->pos;
  uint64_t bytes_free = allocator->size - allocator->pos;

  // do not allocate when there is insufficient space
  if (bytes > bytes_free) {
    return NULL;
  }

  allocator->pos += bytes;
  return allocator->mem + alloc_pos;
}

int arena_pop(struct arena *allocator, uint64_t bytes)
{
  if (bytes > allocator->pos) {
    return -1;
  }

  allocator->pos -= bytes;
  return 0;
}

void arena_free(struct arena *allocator)
{
  free(allocator->mem);
  free(allocator);
}
