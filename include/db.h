/**
 * @file db.c
 * @author Keith Hendricks
 *
 * Header file for db component that contains all operations realting to interacting with the program db.
 */

#ifndef db
#define db

#include <sqlite3.h>


/**
 * Function which opens up the db holding the radio chatter data for the program. If the .db file of the name 
 * "rcs_captured.db" does not exist, then one will be created and have the desired schema added to it. If any error occurs
 * where the db can not be opened, then the program will exit fatally.
 * @param rcsDB the program db holding radio data
 */
void rcs_open_db(sqlite3 *rcsDB);


/**
 * Function which opens will close the sqlite db used by the program. If the db fails to close then the program
 * will exit fatally.
 * @param rcsDB the program db holding radio data
 */
void rcs_close_db(sqlite3 *rcsDB);


//void store_radio_data(sqlite3 *rcsDB, void *data);


#endif
