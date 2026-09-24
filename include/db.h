#ifndef CHESTQL_DB
#define CHESTQL_DB

// forward declare sqlite3 struct for signatures
typedef struct sqlite3 sqlite3;

#define DB_NAME "chestql_db"

sqlite3 *init_db();

#endif
