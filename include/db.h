#ifndef CHESTQL_DB
#define CHESTQL_DB

#include "parser.h"

// forward declare sqlite3 struct for signatures
typedef struct sqlite3 sqlite3;

#define DB_NAME "chestql_db"

/**
 * Perform sqlite3_open with error handling.
 *
 * @return database handle
 */
sqlite3 *init_db();

/**
 * Wipe the `inventory` table, creating it if it doesn't exist.
 *
 * @param db database handle
 */
void reset_db_table(sqlite3 *db);

/**
 * Inserts the chestql_csv data into the database.
 *
 * @param db database handle
 * @param csv csv struct to insert
 */
void load_csv_into_db(sqlite3 *db, struct chestql_csv *csv);

#endif
