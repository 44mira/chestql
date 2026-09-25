#ifndef CHESTQL_ARENA
#define CHESTQL_ARENA

#include <stdint.h>

// by default allot 1 MB for arena operations
// (should be enough for handling csvs representing a vanilla chest)
#define DEFAULT_ARENA_SIZE 1024 * 1024

struct arena;

/**
 * Instantiate an arena for linear allocation.
 *
 * @return The pointer the the arena, else NULL when allocation is unsuccessful.
 */
struct arena *arena_make(uint64_t size);

/**
 * Allocate a specified amount of bytes on an arena.
 *
 * @param allocator the arena to be used for the allocation
 * @param bytes the number of bytes to allocate on the arena
 * @return The pointer to the allocated memory on the arena. NULL when the
 * allocation is unsuccessful (not enough memory).
 */
void *arena_alloc(struct arena *allocator, uint64_t bytes);

/**
 * Frees an arena.
 *
 * @param allocator The arena to be freed.
 */
void arena_free(struct arena *allocator);

#endif
