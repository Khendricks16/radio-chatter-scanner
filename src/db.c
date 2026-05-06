/**
 * @file db.c
 * @author Keith Hendricks
 *
 * Implementation file for db component that contains all logic relating to handling db operations.
 */

#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "../include/db.h"


/** Exit code for database related errors, which put the program in a state where it cannot continue */
#define FATAL_DB_ERROR 3



/**
 * Private helper function for rcs_open_db in the event that the db file did not already exist. The program may also fatally
 * exit here if the correct schema can not be loaded into the new db.
 * @param rcsDB the program db holding radio data
 */
static void load_db_schema(sqlite3 *rcs_db)
{
    // Make sure that the new db has the correct expected schema
    FILE *fp = fopen("./assets/db_schema.sql", "r");

    // Was the sql file for the db schema not able to be opened?
    if (fp == NULL){
        fprintf(stderr, "Error: db schema for new db could not be opened");
        exit(FATAL_DB_ERROR);
    }

    // Will hold how many chars exist in the sql command to load in the db schema
    unsigned short sql_command_len = 0;
    
    // Get the length of the sql commands for the schema
    int curr_char = fgetc(fp);
    while (curr_char != EOF){
        sql_command_len++;
        curr_char = fgetc(fp);
    }

    // Go back to the beginning of the file
    fseek(fp, 0, SEEK_SET);

    // Get a string containing all of the schema sql commands
    char *schema_commands = (char *) malloc(sizeof(char) * sql_command_len + 1);
    for (int i = 0; i < sql_command_len; i++){
        schema_commands[i] = fgetc(fp);
    }
    schema_commands[sql_command_len] = '\0';

    // Close the opened file
    fclose(fp);
    

    // Execute the commands in schema_commands
    sqlite3_stmt *schema_stmt;
    sqlite3_prepare_v2(rcs_db, schema_commands, sql_command_len + 1, &schema_stmt, NULL);

    int stmt_result = sqlite3_step(schema_stmt);
    if (stmt_result == SQLITE_DONE){
        sqlite3_finalize(schema_stmt);
        free(schema_commands);
        return;
    }

    fprintf(stderr, "Error: DB Schema statement failed to be called");
    exit(FATAL_DB_ERROR);
}


void rcs_open_db(sqlite3 **rcs_db)
{
    // Does a db file for the program not already exist?
    bool db_existed = true;
    struct stat statbuf;
    if (stat("rcs_captured.db", &statbuf) == -1 && errno == ENOENT){
        db_existed = false;
    }

    // Try and open the program db
    int db_open_status = sqlite3_open("rcs_captured.db", rcs_db);


    // Did something go wrong?
    if (db_open_status != SQLITE_OK){
        // Some other error occured that is fatal as to why the db could not be opened
        fprintf(stderr, sqlite3_errmsg(*rcs_db));
        fprintf(stderr, "Error: sqlite db could not be opened");
        exit(FATAL_DB_ERROR);
    }

    // When the db was opened, was it newely created?
    if (!db_existed){
        load_db_schema(*rcs_db);
    }
}


void rcs_close_db(sqlite3 **rcs_db)
{
    // Close the programs db connection
    if (sqlite3_close(*rcs_db) != SQLITE_OK){
        // Something wen't wrong as the db couldn't be closed
        // TODO: Log The error that occured 
        fprintf(stderr, sqlite3_errmsg(*rcs_db));
        exit(FATAL_DB_ERROR);
    }
}



bool store_radio_data(sqlite3 **rcs_db, int16_t *data, int data_len, uint32_t freq)
{
    // Set up sql statement that will run against db
    char *sql = "INSERT INTO chatter (freq, time, audio, duration)" \
                 "VALUES ($freqVal, datetime('now'), $audioVal, $durationVal);";
    sqlite3_stmt *stmt;
    int a = sqlite3_prepare_v2(*rcs_db, sql, -1, &stmt, NULL);
    int b = sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "$freqVal"), freq);
    int c = sqlite3_bind_blob(stmt, sqlite3_bind_parameter_index(stmt, "$audioVal"), data, sizeof(int16_t) * data_len, SQLITE_STATIC);

    a = a + b + c;

    // Invoke the statement and destroy it afterwards
    int stmtResult = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Was this sql command successful?
    return stmtResult == SQLITE_DONE;
}

