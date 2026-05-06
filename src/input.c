/**
 * @file input.c
 * @author Keith Hendricks
 *
 * Implementation file that handles parsing and error checking on userse inputs to the program.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#include "getopt/getopt.h"
#include "../include/input.h"


/** argc should at least be equal to this in order for it to be a valid 'scan' subcommand */
#define MIN_CMD_ARGS_RCS_SCAN 4

/** The length of the largest demodulation mode that is supported 'wbfm' */
#define LARGEST_DEMOD_MODE_LEN 4

/** exit code for when no valid subcommand was given */
#define NO_VALID_SUBCOMMAND 5
/** exit code for when the required arguments for any flags given for a subcommand are invalid */
#define INVALID_SUBCOMMAND_USAGE 6



void display_help()
{
    printf("%s\n", 
            "Usage: rcs [subcommand] [<args>]\n\n"
            "The following rcs commands and arguments are:\n"
            "   help        Show this description on using rcs\n\n"
            "   scan [range] [mode]    Actively scan a range of frequencies with voice detection\n"
            "       ex. rcs scan 101.5-109.5 (in Mhz)\n"
            "       -v, --verbose\n"
            "       -t, --timeout (in sec.)\n"
            "       -h --hold-time (in sec.)\n"
            "       -c --cycles (integer)\n"
    );
}



/**
 * Function that will loop through and condense the expanded version of flags like '--verbose' into '-v'.
 * This function will not exit even if invalid flags exist in the given argv.
 *
 * @param argc int for how many command line arguments were given
 * @param argv array of strings, each one being one of the command line arguments given
 * @return a new modified argv that will contain the shorthand notation for each long flag name given.
 *  This array will have the same length of argc
 */
static char ** condense_flags(int argc, char const *argv[])
{
    char **margv = (char **) malloc(sizeof(char *) * argc);

    // Go through each of the command line arguments
    for (int i = 0; i < argc; i++){
        // Is this command line argument one of the expanded versions of a flag value?
        if (strcmp(argv[i], "--verbose") == 0){
            margv[i] = (char *) malloc(sizeof(char) * 3);
            strcpy(margv[i], "-v");
        } else if (strcmp(argv[i], "--timeout") == 0){
            margv[i] = (char *) malloc(sizeof(char) * 3);
            strcpy(margv[i], "-t");
        } else if (strcmp(argv[i], "--hold-time") == 0){
            margv[i] = (char *) malloc(sizeof(char) * 3);
            strcpy(margv[i], "-h");
        } else if (strcmp(argv[i], "--cycles") == 0){
            margv[i] = (char *) malloc(sizeof(char) * 3);
            strcpy(margv[i], "-c");
        } else {
            margv[i] = (char *) malloc(sizeof(char) * strlen(argv[i]) + 1);
            strcpy(margv[i], argv[i]);
        }
    }

    return margv;
}



/**
 * Frees up all memory that is contained within the modified argv that the condense_flags
 * function returns.
 * @param margv the array of strings that should be freed
 */
static void free_modified_argv(int argc, char **margv)
{
    for (int i = 0; i < argc; i++){
        free(margv[i]);
    }
    free(margv);
}




/**
 * Function used to check if the 'rcs help' command was provided or other conditions where this operation should
 * be ran.
 *
 * After calling this function its safe to assume there is at least one more word of input other than
 * just the program name in argv.
 *
 * @param argc int for how many command line arguments were given
 * @param argv array of strings, each one being one of the command line arguments given
 * @param input a struct that will hold parsed out data relating to how the operation should behave
 * @return a boolean value on if the scan subcommand was given
 */
static bool check_help_command(int argc, char const *argv[], program_input_s *input)
{
    // Was just '$ rcs' given? 
    if (argc == 1){
        input->subcommand = help;
        return true;
    }

    // Was just '$ rcs help' given?
    if (argc - 1 == 1 && strcmp(argv[1], "help") == 0){
        input->subcommand = help;
        return true;
    } else if (strcmp(argv[1], "help") == 0){
        // '$ rcs help' some other additional data provided
        fprintf(stderr, "Invalid usage: Try 'rcs help'\n");
        exit(INVALID_SUBCOMMAND_USAGE);
    } else {
        // Another subcommand may have been utilized
        return false;
    }
}



/**
 * Function used to check if the 'rcs scan' command was provided and parse out/validate data provided for this
 * operation.
 *
 * @param argc int for how many command line arguments were given
 * @param argv array of strings, each one being one of the command line arguments given
 * @param input a struct that will hold parsed out data relating to how the operation should behave
 * @return a boolean value on if the scan subcommand was given
 */
static bool check_scan_command(int argc, char const *argv[], program_input_s *input)
{

    // Was '$ rcs scan' given?
    if (strcmp(argv[1], "scan") != 0){
        // Another subcommand may have been utilized
        return false;
    }
    input->subcommand = scan;


    // Make sure that the minimum amount of arguments for running this subcommand is given
    if (argc < MIN_CMD_ARGS_RCS_SCAN){
        fprintf(stderr, "Invalid usage: Try 'rcs help'\n");
        exit(INVALID_SUBCOMMAND_USAGE);
    }

    // Holds the position value for the current command line argument we are on
    unsigned short curr_arg_pos = 2; // starts at 2 as argv[0] is program name and argv[1] is 'scan'


    // Move on to validating the given frequencies
    if (atoi(argv[curr_arg_pos]) == 0){
        // A range of frequencies should be given
        char *freq_range = (char *) malloc(sizeof(char) * strlen(argv[curr_arg_pos]) + 1);
        strcpy(freq_range, argv[curr_arg_pos]);

        char *freq_start;
        char *freq_end;
        if ((freq_start = strtok(freq_range, " - ")) == NULL || (freq_end = strtok(NULL, " - ")) == NULL){
            fprintf(stderr, "Invalid usage: Try 'rcs help'\n");
            exit(INVALID_SUBCOMMAND_USAGE);
        }

        input->freq_start = atoi(freq_start);
        input->freq_end = atoi(freq_end);
        free(freq_range);
    } else {
        // Only one given frequency was provided
        input->freq_start = atoi(argv[curr_arg_pos]);
        input->freq_end = atoi(argv[curr_arg_pos]);
    }

    // Move on to validating the demodulation mode
    curr_arg_pos++;

    // should only be at biggest 'wbfm'
    char *demod_mode = (char *) malloc(sizeof(char) * LARGEST_DEMOD_MODE_LEN + 1);
    strncpy(demod_mode, argv[curr_arg_pos], LARGEST_DEMOD_MODE_LEN);
    for (int i = 0; demod_mode[i]; i++){
        demod_mode[i] = tolower(demod_mode[i]);
    }

    if (strcmp(demod_mode, "am") == 0){
        input->demod_mode = am;

    } else if (strcmp(demod_mode, "fm") == 0){
        input->demod_mode = fm;

    } else if (strcmp(demod_mode, "wbfm") == 0){
        input->demod_mode = wbfm;
    } else {
        fprintf(stderr, "Invalid usage: Try 'rcs help'\n");
        exit(INVALID_SUBCOMMAND_USAGE);
    }
    free(demod_mode);


    char **margv = condense_flags(argc, argv);

    // Handle the additional given flags for the subcommand
    int opt;
    while ((opt = getopt(argc, margv, "vt:h:c:")) != -1){
        switch(opt){
            case 'i':
                // TODO: Enable verbose mode
                break;
            case 't':
                // Set timeout duration
                if ((input->timeout = atoi(optarg)) == 0){
                    fprintf(stderr, "Invalid usage: Try 'rcs help'\n");
                    exit(INVALID_SUBCOMMAND_USAGE);
                }
                break;
            case 'h':
                // Set hold-time duration
                if ((input->hold_time = atoi(optarg)) == 0){
                    fprintf(stderr, "Invalid usage: Try 'rcs help'\n");
                    exit(INVALID_SUBCOMMAND_USAGE);
                }
                break;
            case 'c':
                // Set num of cycles through the freq range that will be done during scan
                if ((input->cycles = atoi(optarg)) == 0){
                    fprintf(stderr, "Invalid usage: Try 'rcs help'\n");
                    exit(INVALID_SUBCOMMAND_USAGE);
                }
                break;
            case '?':
                fprintf(stderr, "Invalid usage: Try 'rcs help'\n");
                exit(INVALID_SUBCOMMAND_USAGE);
                break;
        }

    }

    // Free resources that were used
    free_modified_argv(argc, margv);

    return true;
}



program_input_s * parse_program_input(int argc, char const *argv[])
{
    program_input_s default_program_input_s = {.hold_time=3, .verbose=false, .cycles=3, .recency=86400};

    // Will contain data that will be returned
    program_input_s *input = (program_input_s *) malloc(sizeof(program_input_s));
    memcpy(input, &default_program_input_s, sizeof(program_input_s));

    // Get an array of all the helper methods that check for and validate data for each subcommand
    typedef bool (*check_command_function) (int argc, char const *argv[], program_input_s *input);
    check_command_function funcs[] = {check_help_command, check_scan_command};

    
    // Determine the subcommand given for the program and validate any additional data given
    for (int i = 0; i < SUPPORTED_SUBCOMMANDS_AMT; i++){
        if (funcs[i](argc, argv, input)){
            return input;
        }
    }
    
    // No valid subcommand was able to be located within the argv
    fprintf(stderr, "No valid subcommand given: See 'rcs help'\n");
    exit(NO_VALID_SUBCOMMAND);

}

