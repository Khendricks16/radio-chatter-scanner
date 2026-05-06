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
#include "../include/input.h"



/**
static void report_found_chatter(uint32_t freq)
{
    printf("Found chatter on frequency: %d", freq);
}
*/



/**
 * The main entry point to the program.
 *
 * @param argc int for how many command line arguments were given
 * @param argv array of strings, each one being one of the command line given arguments
 * @return the exit status for the program
 */
int main(int argc, char const *argv[])
{
    // Open up db connection instance for program wide use
    sqlite3 *rcs_db = NULL;
    rcs_open_db(&rcs_db);

    // Parse out the input that was given to the program
    program_input_s *input = parse_program_input(argc, argv);
    
    // Based on which command should be ran, run it
    switch(input->subcommand){
        case help:
            display_help();
            break;
        case scan:
            radio_device_check();

            if (input->freq_start == input->freq_end){
                radio_scan_single_freq(input->freq_start, input->hold_time, input->demod_mode);
            } else {
                // scan_range_linear(input->freq_start, input->freq_end, input->hold_time, input->cycles);
            }
            break;
    }


    // Free up resources used
    free(input);

    // Make sure that the program db is closed
    rcs_close_db(&rcs_db);

    // Program is ready to successfully terminate
    return EXIT_SUCCESS;
}

