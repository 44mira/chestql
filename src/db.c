#include "db.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

sqlite3 *init_db()
{
  sqlite3 *db = NULL;
  char *err_msg = NULL;

  // open the db
  if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
    fprintf(stderr, "db open: %s\n", sqlite3_errmsg(db));
    exit(1);
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
    exit(1);
  }

  printf("db: succesfully opened.\n");

  return db;
}
