/**
 * @file db.c
 * @author Keith Hendricks
 *
 * Header file for db component that contains all operations realting to interacting with the program db.
 */

#ifndef RCS_DB_H
#define RCS_DB_H

#include <sqlite3.h>
#include <stdbool.h>


/**
 * Function which opens up the db holding the radio chatter data for the program. If the .db file of the name 
 * "rcs_captured.db" does not exist, then one will be created and have the desired schema added to it. If any error occurs
 * where the db can not be opened, then the program will exit fatally.
 * @param rcs_db the program db holding radio data
 */
void rcs_open_db(sqlite3 **rcs_db);


/**
 * Function which opens will close the sqlite db used by the program. If the db fails to close then the program
 * will exit fatally.
 * @param rcs_db the program db holding radio data
 */
void rcs_close_db(sqlite3 **rcs_db);


/**
 * Takes in recorded 16 signed bit integer representation of PCM data and stores it within the chatter table of the rcs
 * db.
 *
 * @param rcs_db the program db holding radio data
 * @param data array of data representing the PCM data of captured audio
 * @param data_len how large is the given data array
 * @param freq on what frequency was this data captured on
 * @return a boolean value on if the sql transaction successfull?
 */
bool store_radio_data(sqlite3 **rcs_db, int16_t *data, int data_len, uint32_t freq);


#endif
