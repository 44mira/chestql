#include "parser.h"
#include "arena.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

string *to_string(struct arena *allocator, const char *content,
                  uint64_t contentlen)
{
  string *res = (string *)arena_alloc(allocator, sizeof *res);
  if (res == NULL) {
    return NULL;
  }

  // +1 for the null terminator
  res->value = (char *)arena_alloc(allocator, contentlen+1);

  // when we can't allocate for the value, deallocate the string struct first
  // before returning NULL to avoid memory leak (though that would be the least
  // of our concern here)
  if (res->value == NULL) {
    // if we can't pop, something has gone horribly wrong
    if (arena_pop(allocator, sizeof *res) == -1) {
      fprintf(stderr, "allocator: deallocation error");
      exit(1);
    }

    return NULL;
  }

  strncpy(res->value, content, contentlen);
  res->value[contentlen] = '\0';
  res->length = contentlen;

  return res;
}

int deserialize_csv_row(struct arena *allocator, struct chestql_row **row,
                        const char *content)
{
  // TODO:
  return -1;
}

int deserialize_csv(struct arena *allocator, struct chestql_csv **csv,
                    const char *content, uint64_t contentlen)
{
  // TODO:
  return -1;
}

int parse_http_request(struct arena *allocator, const char *buf, uint64_t bytes,
                       struct chestql_csv **result)
{
  // TODO:
  return -1;
}
