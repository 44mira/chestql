#include "arena.h"
#include "parser.h"
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdint.h>
#include <string.h>

#define ARENA_SIZE 1024
#define expect_msg(fmtsp) "Expected: " fmtsp " | Actual: " fmtsp

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

  cr_assert(eq(str, s->value, (char *)content), expect_msg("%s"), content,
            s->value);
  cr_assert(eq(int, s->length, len), expect_msg("%ld"), len, s->length);
}

Test(parser, deserialize_csv_row_valid, .init = setup, .fini = teardown)
{
  struct chestql_row *r = NULL;
  int retcode;

  const char *content = "1,minecraft:stone,24";

  retcode = deserialize_csv_row(allocator, &r, content);
  cr_assert(retcode == 0, "Deserialization return code is not 0");
  cr_assert(r != NULL, "CSV Row not allocated");

  cr_assert(r->slot == 1, expect_msg("%ld"), 1, r->slot);
  cr_assert(r->count == 24, expect_msg("%ld"), 24, r->count);
  cr_assert(r->name != NULL, "Name not allocated");

  char *stored_name = r->name->value;
  cr_assert(eq(str, stored_name, "minecraft:stone"), expect_msg("%s"),
            "minecraft:stone", stored_name);
}

Test(parser, deserialize_csv_row_invalid, .init = setup, .fini = teardown)
{
  struct chestql_row *r = NULL;
  int retcode;

  const char *content = "1minecraft:stone24";

  retcode = deserialize_csv_row(allocator, &r, content);
  cr_assert(retcode == -1, "Deserialization error should have retcode -1");
}
