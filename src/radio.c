/**
 * @file radio.c
 * @author Keith Hendricks
 *
 * Implementation file for radio component that contains logic for how scanning frequencies and analyzing for voice
 * detection is done.
 * */

#include "input.h"
#include "radio.h"
#include "vad.h"

#include <rtl-sdr.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>


/** The amount of space that should be provided for USB device strings */
#define DEV_INFO_STR_LEN 256

/** What is the largest size that a captured clip can be? */
#define LARGEST_CLIP_SIZE 30


/** exit code for if no radio device is plugged into the computer */
#define NO_RADIO_DEVICE_FOUND 4
/** exit code for if the underlying rtl_fm program could not be invoked */
#define RTL_FM_FAILED 7



void radio_device_check()
{
    // Make sure that at least one rtlsdr device can be found
    if (rtlsdr_get_device_count() == 0){
        fprintf(stderr, "No rtlsdr device could be found, please connect and retry\n");
        exit(NO_RADIO_DEVICE_FOUND);
    }

    // Open device
    rtlsdr_dev_t *radio_dev = NULL;
    rtlsdr_open(&radio_dev, 0);

    // Announce the device that was opened up
    char manufacturer[DEV_INFO_STR_LEN];
    char product_name[DEV_INFO_STR_LEN];
    rtlsdr_get_usb_strings(radio_dev, manufacturer, product_name, NULL);
    fprintf(stdout, "Found available device: %s, %s, %s\n\n", manufacturer, product_name, rtlsdr_get_device_name(0));

    // Close the device so the later rtl_fm program can have control over it
    rtlsdr_close(radio_dev);
}

/**
 * This function will take in as input the read end of a POSIX pipe and continually read in the signed 16 bit ints
 * produced by rtl_fm from the other side and store them in a heap allocated array. A pointer
 * to the start of this array will be returned.
 * 
 * Note: It is the clients responsibilty to free the allocated array returned
 * 
 */
int consume_radio_output(int fd, float timeout, int16_t **radio_data){
    // Contains the start time of this function so we can only run the logic for the desired amount of time
    time_t start = time(NULL);

    int dataCap = 1000;
    *radio_data = (int16_t *) malloc(sizeof(int16_t) * dataCap);
    int data_len = 0;

    while ((time(NULL) - start) < timeout){
        // Do we need to resize our data array?
        if (data_len + 1 >= dataCap){
            dataCap *= 2; // double for optimal runtime efficently
            *radio_data = realloc(*radio_data, sizeof(int16_t) * dataCap);
        }

        // Get the next 
        int readResult = read(fd, &((*radio_data)[data_len]), sizeof(int16_t));
        if (readResult <= 0){
            // Something went wrong when trying to read from our pipe so return with what we got
            perror("Error: ");
            return data_len;
        }

        // Make sure that we increase the length since we just added to our array
        data_len++;
    }

    // We are all done
    return data_len;
}

void radio_scan_single_freq(uint32_t freq, float timeout, int demod_mode)
{
    // Get the string representation of the given freq
    uint32_t tmp_freq = freq;
    unsigned short freq_len = 0;
    while (tmp_freq > 0){
        tmp_freq /= 10;
        freq_len++;
    }

    char *freq_str = (char *) malloc((sizeof(char) * freq_len) + 1);
    sprintf(freq_str, "%d", freq);

    // Array containing str representations of each demod mode in order
    char *modes[] = {"fm", "wbfm", "am"};

    // Create a POSIX pipe that the child rtl_fm process will use to spit out radio data
    int radioPipe[2];
    if (pipe(radioPipe) != 0){
        fprintf(stderr, "Failed to create a needed pipe");
        exit(1);
    }

    // Spawn and run a child process of the rtl_fm program to collect the demodulated radio data on
    // the given frequency
    pid_t pid;
    if ((pid = fork()) < 0){
        fprintf(stderr, "Failed to spawn needed process\n");
        exit(RTL_FM_FAILED);
    }

    // Are we within the child process?
    if (pid == (pid_t) 0) {
        // Hide all informational output that will come from rtl_fm
        freopen("/dev/null", "w", stderr);

        // Close the side of the pipe that will be unused
        close(radioPipe[0]);

        // Make sure that the radio data produced by rtl_fm goes to the radioPipe
        dup2(radioPipe[1], STDOUT_FILENO);

        // Invoke rtl_fm
        char *args[] = {"rtl_fm", "-f", freq_str, "-M", modes[demod_mode], "-s", "170k", "-r", "16k", NULL};
        execvp(args[0], args);
        exit(EXIT_FAILURE);
    }
    // (We are the parent process)
    // Close the side of the pipe that will be unused
    close(radioPipe[1]);

    // Consume radio output for the desired amount of time
    int16_t *radio_data;
    int radio_data_len = consume_radio_output(radioPipe[0], timeout, &radio_data);
    
    // Does this radio data contain voice activity?
    void *rcs_vad = rcs_vad_create(16000, 0.5);
    bool has_voice_activity = rcs_vad_determine_voice_activity(rcs_vad, radio_data, radio_data_len);
    if (has_voice_activity){
        fprintf(stdout, "Voice activity detected on provided frequency!\n");
    } else {
        fprintf(stdout, "No voice activity detected :(\n");
    }

    // We are all done with the spawned rtl_fm child process, so kill it
    kill(pid, SIGTERM);

    // Close resources
    rcs_vad_destroy(rcs_vad);
    close(radioPipe[0]);
    free(freq_str);
    free(radio_data);
}



/**
 * Potential callback function for each timestamp (return of r_scan_single_freq) as a param to this function.
void scan_range_linear(uint32_t start, uint32_t end, int hold_time, int cycles, int demod_mode)
{

    for (int i = 0; i < cycles; i++) {
        for (int j = start; j < end; j += 5e5) { // Intervals of 0.5 MHz
            radio_scan_single_freq(j, hold_time, demod_mode);
        }
    }
}
*/

