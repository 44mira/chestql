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

Test(parser, deserialize_csv_valid, .init = setup, .fini = teardown)
{
  struct chestql_csv *c = NULL;
  struct chestql_row *row = NULL;
  int retcode;

  const char *content = "1,minecraft:stone,24\n"
                        "2,minecraft:boat,1\n"
                        "4,minecraft:copper_ingot,64";
  uint64_t len = strlen(content);

  retcode = deserialize_csv(allocator, &c, content, len);
  cr_assert(retcode == 0, "Deserialization should have retcode 0");
  cr_assert(c != NULL, "CSV not allocated");

  cr_assert(c->row_count == 3, expect_msg("%ld"), 3, c->row_count);

  row = c->rows[0];
  cr_assert(row->slot == 1, expect_msg("%ld"), 1, row->slot);
  cr_assert(row->count == 24, expect_msg("%ld"), 24, row->count);
  cr_assert(eq(str, row->name->value, "minecraft:stone"), expect_msg("%s"),
            "minecraft:stone", row->name->value);

  row = c->rows[1];
  cr_assert(row->slot == 2, expect_msg("%ld"), 2, row->slot);
  cr_assert(row->count == 1, expect_msg("%ld"), 1, row->count);
  cr_assert(eq(str, row->name->value, "minecraft:boat"), expect_msg("%s"),
            "minecraft:boat", row->name->value);

  row = c->rows[2];
  cr_assert(row->slot == 4, expect_msg("%ld"), 4, row->slot);
  cr_assert(row->count == 64, expect_msg("%ld"), 64, row->count);
  cr_assert(eq(str, row->name->value, "minecraft:copper_ingot"),
            expect_msg("%s"), "minecraft:copper_ingot", row->name->value);
}
