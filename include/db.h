#ifndef CHESTQL_DB
#define CHESTQL_DB

#include "arena.h"
#include "parser.h"

// forward declare sqlite3 struct for signatures
typedef struct sqlite3 sqlite3;

#define DB_NAME "chestql_db"

sqlite3 *init_db();

void reset_db_table(sqlite3 *db);

void load_csv_into_db(sqlite3 *db, struct chestql_csv* csv);

#endif
