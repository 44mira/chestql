#include "parser.h"
#include "arena.h"
#include <ctype.h>
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
      fprintf(stderr, "allocator: deallocation error\n");
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
        fprintf(stderr, "allocator: deallocation error\n");
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
        fprintf(stderr, "allocator: deallocation error\n");
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
      fprintf(stderr, "allocator: deallocation error\n");
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
  if (contentlen == 0)
    return -1;

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
  if (content[contentlen - 1] != '\n') {
    row_count++;
  }

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
  for (uint64_t i = 0; row_idx < row_count && i < contentlen + 1; i++) {
    if (content[i] == '\n' || i == contentlen) {
      tmp[i] = '\0';

      struct chestql_row *r = NULL;
      if (deserialize_csv_row(allocator, &r, tmp + bp) != 0) {
        if (arena_pop(allocator, allocated_bytes) != 0) {
          fprintf(stderr, "allocator: deallocation error\n");
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

enum MOVEEOL move_to_eol(const char **ptr)
{
  while (**ptr != '\n' && **ptr != '\0')
    (*ptr)++;

  if (**ptr == '\n')
    return EOL_NEWLN;
  return EOL_EOF;
}

void parse_http_header(const char *ptr, int *is_type_csv, uint64_t *contentlen)
{
  uint64_t line_length = 0;
  char header[TMPBUFSIZ] = {0};

  // find line length
  // don't think I need to check for '\0' here
  while (*(ptr + line_length) != '\n') {
    line_length++;
  }
  strncpy(header, ptr, line_length + 1);
  header[line_length] = '\0';

  // lowercase string
  for (uint64_t i = 0; i < line_length; i++) {
    header[i] = tolower(header[i]);
  }

  // set header values
  if (strncmp(header, "content-length: ", 16) == 0) {
    *contentlen = strtoul(header + 16, NULL, 10);
  } else if (strncmp(header, "content-type: ", 14) == 0) {
    *is_type_csv = strncmp(header + 14, "text/csv", 8) ? 0 : 1;
  }

  return;
}

int parse_http_request(struct arena *allocator, const char *buf, uint64_t bytes,
                       struct chestql_csv **result)
{
  int is_type_csv = 0;
  uint64_t contentlen = 0;
  const char *ptr = buf; // make a copy of the buf pointer

  // Expect HTTP request to be a POST ------------------------------------
  if (strncmp(ptr, "POST", 4) != 0) {
    fprintf(stderr, "parse_http_request: expected POST request\n");
    return -1;
  }

  if (move_to_eol(&ptr) == EOL_EOF) {
    fprintf(stderr, "parse_http_request: early EOF\n");
    return -1;
  }
  ptr++;

  // loop over headers, making sure we don't go over the buffer size
  while (ptr < buf + bytes) {
    parse_http_header(ptr, &is_type_csv, &contentlen);
    if (move_to_eol(&ptr) == EOL_EOF) {
      fprintf(stderr, "parse_http_request: early EOF\n");
      return -1;
    }

    // if body starts, exit header loop
    if (strncmp(ptr - 1, "\r\n\r\n", 4) == 0) {
      ptr++;
      move_to_eol(&ptr);
      break;
    }

    // move to start of next line
    ptr++;
  }
  ptr++; // move to start of body
  
  if (!is_type_csv) {
    fprintf(stderr, "parse_http_request: expected Content-Type: text/csv\n");
    return -1;
  }

  // Ensure ptr didn't overshoot the buffer
  if (ptr > buf + bytes) {
    fprintf(stderr, "parse_http_request: overshot buffer\n");
    return -1;
  }

  // Ensure actual remaining bytes and contentlen match
  uint64_t remaining_bytes = (uint64_t)((buf + bytes) - ptr);
  if (contentlen > remaining_bytes) {
    fprintf(stderr,
            "parse_http_request: contentlen and remaining bytes mismatch\n\n "
            "contentlen: %ld\nremaining_bytes: %ld\n",
            contentlen, remaining_bytes);
    return -1;
  }

  return deserialize_csv(allocator, result, ptr, contentlen);
}
