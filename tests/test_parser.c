#include "arena.h"
#include "parser.h"
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdint.h>
#include <string.h>

#define ARENA_SIZE 1024

struct arena *allocator = NULL;

void setup(void)
{
  allocator = arena_make(ARENA_SIZE);

  if (allocator == NULL) {
    exit(1);
  }
}

void teardown(void) { arena_free(allocator); }

Test(parser, to_string_valid, .init = setup, .fini = teardown)
{
  string *s = NULL;

  const char *content = "hello";
  uint64_t len = strlen(content);

  s = to_string(allocator, content, len);

  cr_assert(s != NULL, "String not allocated");

  cr_assert(eq(str, s->value, (char *)content), "Expected: %s | Actual: %s",
            content, s->value);
  cr_assert(eq(int, s->length, len), "Expected: %ld | Actual %ld", len,
            s->length);
}
