/**
 * @file main.c
 * @author Keith Hendricks
 *
 * The main starting point for the program.
 */

#include <sqlite3.h>
#include <rtl-sdr.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/db.h"
#include "../include/radio.h"
//#include "../include/input.h"



/**
 * The main entry point to the program.
 *
 * @param argc int for how many command line arguments were given
 * @param argv array of strings, each one being one of the program given arguments
 * @return the exit status for the program
 */
int main(int argc, char *argv[])
{
    // Open up db connection instance for program wide use
    sqlite3 *rcsDB = NULL;
    rcs_open_db(rcsDB);

    // Open up connection to radio device
    rtlsdr_dev_t *radioDev = NULL;
    radioDeviceSetup(&radioDev);

    // struct programInput *input = defaultProgramInput;

    // Just try and scan for now
    radioScanFreq(radioDev, 1.015e8);

    // Make sure the radio device is closed
    rtlsdr_close(radioDev);

    // Make sure that the program db is closed
    rcs_close_db(rcsDB);

    // Program is ready to successfully terminate
    return EXIT_SUCCESS;
}

