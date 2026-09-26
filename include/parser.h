#ifndef CHESTQL_PARSER
#define CHESTQL_PARSER

#include "arena.h"
#include <stdint.h>

typedef struct {
  char *value;
  uint64_t length;
} string;

struct chestql_row {
  uint64_t slot;
  uint64_t count;
  string *name;
};

struct chestql_csv {
  struct chestql_row **rows;
  uint64_t row_count;
};

/**
 * Converts a char* into a string struct that holds both the content and its
 * length.
 *
 * @param allocator arena to be used for allocation.
 * @param content character array to be converted.
 * @param contentlen length of the string, excluding the null terminator.
 * @return pointer to the allocated struct, NULL on unsuccessful allocation.
 */
string *to_string(struct arena *allocator, const char *content,
                  uint64_t contentlen);

/**
 * Parses a csv row according to the spec.
 *
 * @param allocator arena to be used for allocation.
 * @param row pointer to a pointer that will be used to store the
 * resulting struct.
 * @param content raw csv row text to be deserialized.
 * @return 0 on success, -1 on error.
 */
int deserialize_csv_row(struct arena *allocator, struct chestql_row **row,
                        const char *content);

/**
 * Parses a csv according to the spec. Makes calls to `deserialize_csv_row` for
 * every newline-delimited row in the csv.
 *
 * @param allocator arena to be used for allocation.
 * @param csv pointer to a pointer that will be used to store the resulting
 * struct.
 * @param content raw csv text to be deserialized.
 * @param contentlen size of content.
 * @return 0 on success, -1 on error, permeates error from internal
 * `deserialize_csv_row` calls.
 */
int deserialize_csv(struct arena *allocator, struct chestql_csv **csv,
                    const char *content, uint64_t contentlen);

/**
 * Parses an HTTP request, expecting a `text/csv` that matches the spec.
 *
 * @param allocator arena to be used for allocation.
 * @param buf buffer that contains the raw HTTP request.
 * @param bytes length of the request in bytes.
 * @param result pointer to a pointer that will hold the resulting csv struct.
 * @return 0 on success, -1 on error, permeates from internal errors.
 */
int parse_http_request(struct arena *allocator, const char *buf, uint64_t bytes,
                       struct chestql_csv **result);

#endif
