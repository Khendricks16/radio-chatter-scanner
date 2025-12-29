/**
 * @file db.c
 * @author Keith Hendricks
 *
 * Implementation file for db component that contains all logic relating to handling db operations.
 */

#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/db.h"


/** Exit code for database related errors, which put the program in a state where it cannot continue */
#define FATAL_DB_ERROR 3


// TODO: Utilize ISO-8601 Text for storing data in strings in "time" field 

/**
 * Private helper function for rcs_open_db in the event that the db file did not already exist. The program may also fatally
 * exit here if the correct schema can not be loaded into the new db.
 * @param rcsDB the program db holding radio data
 */
static void load_db_schema(sqlite3 *rcsDB)
{
    // Make sure that the new db has the correct expected schema
    FILE *fp = fopen("db_schema.sql", "r");

    // Was the sql file for the db schema not able to be opened?
    if (fp == NULL){
        fprintf(stderr, "Error: db schema for new db could not be opened");
        exit(FATAL_DB_ERROR);
    }

    // Will hold how many chars exist in the sql command to load in the db schema
    unsigned short sqlCommandLen = 0;
    
    // Get the length of the sql commands for the schema
    int currChar = fgetc(fp);
    while (currChar != EOF){
        sqlCommandLen++;
        currChar = fgetc(fp);
    }

    // Go back to the beginning of the file
    fseek(fp, 0, SEEK_SET);

    // Get a string containing all of the schema sql commands
    char *schemaCommands = (char *) malloc(sizeof(char) * sqlCommandLen + 1);
    for (int i = 0; i < sqlCommandLen; i++){
        schemaCommands[i] = fgetc(fp);
    }
    schemaCommands[sqlCommandLen] = '\0';

    // Close the opened file
    fclose(fp);
    

    // Execute the commands in schemaCommands
    sqlite3_stmt *schemaStmt;
    sqlite3_prepare_v2(rcsDB, schemaCommands, sqlCommandLen + 1, &schemaStmt, NULL);

    int stmtResult = sqlite3_step(schemaStmt);
    if (stmtResult == SQLITE_DONE){
        sqlite3_finalize(schemaStmt);
        free(schemaCommands);
        return;
    }

    fprintf(stderr, "Error: DB Schema statement failed to be called");
    exit(FATAL_DB_ERROR);
}


void rcs_open_db(sqlite3 *rcsDB)
{
    // Does a db file for the program not already exist?
    bool dbExisted = true;
    struct stat statbuf;
    if (stat("rcs_captured.db", &statbuf) == -1 && errno == ENOENT){
        dbExisted = false;
    }

    // Try and open the program db
    int dbOpenStatus = sqlite3_open("rcs_captured.db", &rcsDB);


    // Did something go wrong?
    if (dbOpenStatus != SQLITE_OK){
        // Some other error occured that is fatal as to why the db could not be opened
        fprintf(stderr, sqlite3_errmsg(rcsDB));
        fprintf(stderr, "Error: sqlite db could not be opened");
        exit(FATAL_DB_ERROR);
    }

    // When the db was opened, was it newely created?
    if (!dbExisted){
        load_db_schema(rcsDB);
    }
}


void rcs_close_db(sqlite3 *rcsDB)
{
    // Close the programs db connection
    if (sqlite3_close(rcsDB) != SQLITE_OK){
        // Something wen't wrong as the db couldn't be closed
        // TODO: Log The error that occured 
        fprintf(stderr, sqlite3_errmsg(rcsDB));
        exit(FATAL_DB_ERROR);
    }
}


/*
void store_radio_data(sqlite3 *rcsDB, void *data)
{
    return;
}

*/

