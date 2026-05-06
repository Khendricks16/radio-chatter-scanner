/**
 * @file input.h
 * @author Keith Hendricks
 *
 * Header file for input component for the program which handles all things relating to program input.
 * An important struct definition for holding the configuration settings for the program is defined here
 * along with exposing a function to handle the argc and argv provided to the program accordingly.
 */

#ifndef RCS_INPUT_H
#define RCS_INPUT_H

#include <stdbool.h>

/** How many total demodulation modes does this program support? */
#define SUPPORTED_DEMOD_MODES_AMT 3
/** How many total subcommands does this program support? */
#define SUPPORTED_SUBCOMMANDS_AMT 2

/** struct which will hold possible input values given to the program */
struct program_input_struct {
    enum subcommands {help, scan} subcommand;
    enum demod_modes {fm, wbfm, am} demod_mode;
    bool verbose;
    int timeout; // Timeout for program scanning (In seconds)
    int hold_time; // How long should each pause occur on frequency changing (In seconds)
    int cycles;
    uint32_t freq_start;
    uint32_t freq_end;
    int recency; // How recent should desired data be shown/deleted (In seconds)
};

typedef struct program_input_struct program_input_s;




/**
 * Function that will take in argc and argv for the program and parse out and fill up possible input
 * for the program. If any of the given flags or subcommands are invalid, the program will terminate with
 * a specific exit code.
 * @param argc int for how many command line arguments were given
 * @param argv array of strings, each one being one of the command line given arguments
 * @return a filled program input struct with the users data
 */
program_input_s * parse_program_input(int argc, char const *argv[]);


/**
 * Prints to stdout a quick help reference for how to use the program with its subcommands and options
 * available to the user.
 */
void display_help();


#endif

