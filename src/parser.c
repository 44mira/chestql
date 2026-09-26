#include "parser.h"
#include "arena.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TMPBUFSIZ 1024

string *to_string(struct arena *allocator, const char *content,
                  uint64_t contentlen)
{
  string *res = (string *)arena_alloc(allocator, sizeof *res);
  if (res == NULL) {
    return NULL;
  }

  // +1 for the null terminator
  res->value = (char *)arena_alloc(allocator, contentlen + 1);

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
  uint64_t bp, i = 0;

  // allocate for the row struct
  *row = (struct chestql_row *)arena_alloc(allocator, sizeof **row);
  if (*row == NULL) {
    return -1;
  }

  char tmp[TMPBUFSIZ] = {0}; // same as memset 0 with static arrays

  /*
   * Parse CSV --------------------------------------------------------
   * Can probably shorten this with a for loop and some flags,
   * but this works and is just pedantic enough to be readable
   * so i'll keep it this way until something about it really bothers me
   *
   * Copy characters over a continuous buffer, separating entries with
   * null terminators and accessing them with pointer arithmetic (bp and i).
   */

  // read for slot
  while (content[i] != ',') {
    if (content[i] == '\0') {
      if (arena_pop(allocator, sizeof **row) == -1) {
        fprintf(stderr, "allocator: deallocation error");
        exit(1);
      }
      return -1;
    }

    tmp[i] = content[i];
    i++;
  }

  tmp[i] = '\0';

  (*row)->slot = strtoul(tmp, NULL, 10);

  i++;
  bp = i; // set a breakpoint to indicate the start of the next string

  // read for name
  while (content[i] != ',') {
    if (content[i] == '\0') {
      if (arena_pop(allocator, sizeof **row) == -1) {
        fprintf(stderr, "allocator: deallocation error");
        exit(1);
      }
      return -1;
    }

    tmp[i] = content[i];
    i++;
  }

  tmp[i] = '\0';
  (*row)->name = to_string(allocator, tmp + bp, i - bp);
  if ((*row)->name == NULL) {
    if (arena_pop(allocator, sizeof **row) == -1) {
      fprintf(stderr, "allocator: deallocation error");
      exit(1);
    }
    return -1;
  }

  i++;
  bp = i; // set a breakpoint to indicate the start of the next string

  // read for count
  while (content[i] != '\0') {
    tmp[i] = content[i];
    i++;
  }
  tmp[i] = '\0';

  // we actually lose some of the numbers from using uint64 by using atoi
  // but I don't feel like rolling up my own number parser rn
  (*row)->count = strtoul(tmp + bp, NULL, 10);

  return 0;
}

int deserialize_csv(struct arena *allocator, struct chestql_csv **csv,
                    const char *content, uint64_t contentlen)
{
  uint64_t row_count = 0, bp = 0;
  uint64_t allocated_bytes = 0;
  *csv = (struct chestql_csv *)arena_alloc(allocator, sizeof **csv);
  if (*csv == NULL) {
    return -1;
  }
  allocated_bytes += sizeof(**csv);

  // First Pass: count the rows

  for (uint64_t i = 0; i < contentlen; i++) {
    if (content[i] == '\n') {
      row_count++;
    }
  }

  // account for the last row if it doesn't end in newline
  if (content[contentlen] != '\n')
    row_count++;

  (*csv)->row_count = row_count;
  (*csv)->rows = (struct chestql_row **)arena_alloc(
      allocator, sizeof *(*csv)->rows * row_count);
  allocated_bytes += sizeof *(*csv)->rows * row_count;

  // Second pass: parse the rows

  // we allocate outside of the arena because this tmp does not
  // share lifetime with them. malloc() should be ok here since
  // the free() is pretty obvious (within the same scope).
  char *tmp = (char *)malloc(sizeof *tmp * (contentlen + 1));
  uint64_t row_idx = 0;
  for (uint64_t i = 0; i < contentlen + 1; i++) {
    if (content[i] == '\n' || i == contentlen) {
      tmp[i] = '\0';

      struct chestql_row *r = NULL;
      if (deserialize_csv_row(allocator, &r, tmp + bp) != 0) {
        if (arena_pop(allocator, allocated_bytes) != 0) {
          fprintf(stderr, "allocator: deallocation error");
          exit(1);
        }

        free(tmp);
        return -1;
      }

      (*csv)->rows[row_idx] = r;
      row_idx++;
      bp = i + 1; // move start of next string after this index

      continue;
    }

    tmp[i] = content[i];
  }
  free(tmp);

  return 0;
}

int parse_http_request(struct arena *allocator, const char *buf, uint64_t bytes,
                       struct chestql_csv **result)
{
  // TODO:
  return -1;
}
