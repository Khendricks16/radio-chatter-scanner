/**
 * @file input.c
 * @author Keith Hendricks
 *
 * Implementation file that handles parsing and error checking on users inputs to the program.
 */
#include "getopt/getopt.h"
#include "input.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/** argc should at least be equal to this in order for it to be a valid 'scan' subcommand */
#define MIN_CMD_ARGS_RCS_SCAN 4
/** The length of the largest demodulation mode that is supported 'wbfm' */
#define LARGEST_DEMOD_MODE_LEN 4


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
 * Note: The 'rcs help' command should take the form of:
 *  $ rcs help
 *
 * @param argc int for how many command line arguments were given
 * @param argv array of strings, each one being one of the command line arguments given
 * @param input a struct that will hold parsed out data relating to how the operation should behave
 * @return a boolean value on if the help subcommand was given
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
    }

    // Another subcommand may have been utilized or there was extra data after the 'help'
    return false;
}



void display_help()
{
    printf("%s\n", 
            "Usage: rcs [subcommand] [<args>]\n\n"
            "The following rcs commands and arguments are:\n"
            "   help        Show this description on using rcs\n\n"
            "   scan [range] [mode]    Actively scan a range of frequencies with voice detection\n"
            "       ex. rcs scan 101.5M-109.5M wbfm\n"
            "       -v, --verbose\n"
            "       -t, --timeout (in sec.)\n"
            "       -h --hold-time (in sec.)\n"
            "       -c --cycles (integer)\n"
    );
}



/**
 * Private helper routine for sanitize_frequency_range that takes a value (112.8) along with a pointer to a potential
 * ending frequency suffix (M/k) and applying the increase on the given val.
 * 
 * ex. 112.8M | (val = 112.8) (end_ptr -> M) | val becomes 11280000
 * 
 * Note: If a shorthand suffix is present (i.e M/k) then end_ptr will be moved over by one (end_ptr++) after
 * processing the character
 * 
 * Note: If the given float val points to is 0, then the program will exit with a failure status
 *  (An invalid frequency was provided).
 * 
 * @param val pointer to the float that will be modified
 * @param end_ptr pointer to the char pointer which is one character after the frequency that was read
 */
static bool apply_frequency_suffix(float *val, char **end_ptr)
{
    // Is the given val valid?
    if (*val == 0){
        return false;
    }

    // Did the user use a M/k suffix as a shorthand for a larger number?
    switch (**end_ptr){
        case 'k':
            *val *= 1e3;
            (*end_ptr)++;
            break;
        case 'M':
            *val *= 1e6;
            (*end_ptr)++;
            break;
    }

    // All done
    return true;
}



/**
 * Private helper function to validate the frequency range given by the user in commands
 * like scan and capture
 * 
 * ex. 112.8M-118.9M
 * 
 * @param freq_range_str the user input which contains the frequency range (can take multiple forms)
 * @param input the program input struct that will be modified if the user input is valid
 * @param argv array of strings, each one being one of the command line given arguments
 * @param argvPos index for which element in argv is the frequency range
 * @return a boolean value on if it was valid or not
 */
static bool sanitize_frequency_range(const char *freq_range_str, program_input_s *input)
{
    char *end_ptr = NULL;

    // Try and get at least one value for the frequency range
    float start_val = strtof(freq_range_str, &end_ptr);

    // Make sure the given value is not negative
    if (start_val < 0){
        return false;
    }
    
    // Attempt to apply the frequency suffix to the starting frequency, or fail if it was invalid
    bool freq_suffix_result = apply_frequency_suffix(&start_val, &end_ptr);
    if (!freq_suffix_result){
        return false;
    }

    // Are we done? (No ending frequency given)
    if (*end_ptr == '\0'){
        input->freq_start = start_val;
        input->freq_end = start_val;
        return true;
    }

    // If there is going to be an ending frequency, then the char tha end_ptr is pointing to must be '-'
    if (*end_ptr != '-'){
        return false;
    }

    // Move past the valid '-' that was present
    end_ptr++;

    // Attempt to get the ending frequency
    float end_val = strtof(end_ptr, &end_ptr);

    // Make sure the given value is not negative
    if (end_val < 0){
        return false;
    }

    // Attempt to apply the frequency suffix to the ending frequency, or fail if it was invalid
    freq_suffix_result = (&end_val, &end_ptr);
    if (!freq_suffix_result){
        return false;
    }

    // Make sure that there is no other characters provided after processing the ending frequency
    if (*end_ptr != '\0'){
        return false;
    }

    // All done, so update the provided program input struct
    input->freq_start = start_val;
    input->freq_end = end_val;
    return true;
}



/**
 * Function used to check if the 'rcs scan' command was provided and parse out/validate data provided for this
 * operation.
 * 
 * Note: The 'rcs scan' command should take the form of:
 *  $ rcs scan [range] [mode]
 *  ex. rcs scan 108.9M wbfm
 *  ex. rcs scan 112.6M-136.5M wbfm
 *  ex. rcs scan 25k am
 *  ex. rcs scan 112900 wbfm
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
        return false;
    }

    // Holds the position value for the current command line argument we are on
    unsigned short curr_arg_pos = 2; // starts at 2 as argv[0] is program name and argv[1] is 'scan'

    // Process what should be the range of provided frequencies and exit if its invalid
    bool range_result = sanitize_frequency_range(argv[curr_arg_pos], input);
    if (!range_result){
        return false;
    }

    // Move on to validating the demodulation mode
    curr_arg_pos++;

    // should only be at biggest 'wbfm'
    char *demod_mode = (char *) malloc(sizeof(char) * (LARGEST_DEMOD_MODE_LEN + 1));
    strncpy(demod_mode, argv[curr_arg_pos], LARGEST_DEMOD_MODE_LEN + 1);
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
        return false;
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
                    return false;
                }
                break;
            case 'h':
                // Set hold-time duration
                if ((input->hold_time = atoi(optarg)) == 0){
                    return false;
                }
                break;
            case 'c':
                // Set num of cycles through the freq range that will be done during scan
                if ((input->cycles = atoi(optarg)) == 0){
                    return false;
                }
                break;
            case '?':
                return false;
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
    return NULL;

}
