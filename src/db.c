#include "db.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

sqlite3 *init_db()
{
  sqlite3 *db = NULL;

  // open the db
  if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
    fprintf(stderr, "db open: %s\n", sqlite3_errmsg(db));
    exit(1);
  }

  printf("db: succesfully opened.\n");

  return db;
}

void reset_db_table(sqlite3 *db)
{
  char *err_msg = NULL;

  const char *conditional_drop_table_sql =
      // sql
      "DROP TABLE IF EXISTS inventory"
      ";";
  if (sqlite3_exec(db, conditional_drop_table_sql, NULL, NULL, &err_msg) !=
      SQLITE_OK) {
    fprintf(stderr, "db reset: %s\n", sqlite3_errmsg(db));
    sqlite3_free(err_msg);
    exit(1); // should probably just return this and let caller handle the error
  }

  const char *create_table_sql =
      // sql
      "CREATE TABLE IF NOT EXISTS inventory ("
      "  slot INTEGER PRIMARY KEY,"
      "  name TEXT NOT NULL,"
      "  count INTEGER NOT NULL"
      ");";

  if (sqlite3_exec(db, create_table_sql, NULL, NULL, &err_msg) != SQLITE_OK) {
    fprintf(stderr, "db create: %s\n", sqlite3_errmsg(db));
    sqlite3_free(err_msg);
    exit(1); // should probably just return this and let caller handle the error
  }
}

void load_csv_into_db(sqlite3 *db, struct chestql_csv *csv)
{
  sqlite3_stmt *stmt;
  const char *insert_row_sql =
      // sql
      "INSERT INTO inventory (slot, name, count)"
      "VALUES (?, ?, ?)"
      ";";

  if (sqlite3_prepare_v2(db, insert_row_sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "db load (prepare): %s\n", sqlite3_errmsg(db));
    return;
  }

  // do insert operations in bulk
  sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

  for (uint64_t i = 0; i < csv->row_count; i++) {
    // we lose some precision here because sqlite3 represents INTEGER as signed
    // though we won't ever hit the upper bounds for that to matter
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)csv->rows[i]->slot);

    // last argument is SQLITE_STATIC because memory is handled by our arena
    sqlite3_bind_text(stmt, 2, csv->rows[i]->name->value,
                      csv->rows[i]->name->length, SQLITE_STATIC);

    // same caveat as the slot insert
    sqlite3_bind_int64(stmt, 3, (sqlite3_int64)csv->rows[i]->count);

    // execute bound statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      fprintf(stderr, "db load (insert on index %ld): %s\n", i,
              sqlite3_errmsg(db));
    }

    // reset the statement binds for next iteration
    sqlite3_reset(stmt);
  }

  // close bulk operation and run
  sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);

  // free stmt
  sqlite3_finalize(stmt);
}
