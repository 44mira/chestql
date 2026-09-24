#include "db.h"
#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>

sqlite3 *init_db()
{
  sqlite3 *db = NULL;

  if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
    fprintf(stderr, "db: %s\n", sqlite3_errmsg(db));
    exit(1);
  }
  
  printf("db: succesfully opened.\n");

  return db;
}
