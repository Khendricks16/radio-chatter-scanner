/**
 * @file radio.c
 * @author Keith Hendricks
 *
 * Implementation file for radio component that contains logic for how scanning frequencies and analyzing for voice
 * detection is done.
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtl-sdr.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "../include/input.h"
#include "../include/radio.h"

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
    fprintf(stdout, "%s\n", rtlsdr_get_device_name(0));
    char manufacturer[DEV_INFO_STR_LEN];
    char product_name[DEV_INFO_STR_LEN];
    rtlsdr_get_usb_strings(radio_dev, manufacturer, product_name, NULL);
    fprintf(stdout, "Device Manufacturer: %s\n", manufacturer);
    fprintf(stdout, "Device Product Name: %s\n", product_name);

    // Close the device so the later rtl_fm program can have control over it
    rtlsdr_close(radio_dev);
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

    // Get the string representation of the given timeout duration
    uint32_t whole_num_part = (uint32_t) timeout;
    unsigned short whole_num_len = 0;
    while (whole_num_part > 0){
        whole_num_part /= 10;
        whole_num_len++;
    }

    // 1 (.) + 2 (two points of precision) + 1 (null terminator)
    char *timeout_str = (char *) malloc((sizeof(char) * whole_num_len) + 1 + 2 + 1);
    sprintf(timeout_str, "%.2f", timeout);


    // Array containing str representations of each demod mode in order
    char *modes[] = {"fm", "wbfm", "am"};


    // Spawn and run a child process of the rtl_fm program to collect the demodulated radio data on
    // the given frequency
    pid_t pid;
    if ((pid = fork()) < 0){
        fprintf(stderr, "Failed to spawn needed process\n");
        exit(RTL_FM_FAILED);
    }

    if (pid == (pid_t) 0) {
        // We are within the child process

        // Hide all informational output that will come from rtl_fm
        freopen("/dev/null", "w", stderr);

        // Invoke rtl_fm
        char *args[] = {"timeout", timeout_str, "rtl_fm", "-f", freq_str, "-M", modes[demod_mode], "-s", "170k", "-r", "16k", "out.pcm", NULL};
        execvp(args[0], args);
        exit(1);
    } else if (pid > (pid_t) 0){
        // We are in the parent process, so wait for the child to finish
        wait(NULL);
    }


    // Spawn and run a child process of the ffmpeg to convert the captured PCM data into
    // a wav file
    if ((pid = fork()) < 0){
        fprintf(stderr, "Failed to spawn needed process\n");
        exit(RTL_FM_FAILED);
    }

    if (pid == (pid_t) 0) {
        // We are within the child process

        // Hide all informational output that will come from rtl_fm
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        // Invoke rtl_fm
        char *args[] = {"ffmpeg", "-f", "s16le", "-ar", "16k", "-ac", "1", "-i", "out.pcm", "out.wav", NULL};
        execvp(args[0], args);
        exit(1);
    } else if (pid > (pid_t) 0){
        // We are in the parent process, so wait for the child to finish
        wait(NULL);
    }


    // Make a decision on if human voice is detected in what was recorded
    //is_active_freq = determine_vad();
    // main's report_chatter


    // Close resources
    free(freq_str);
    free(timeout_str);
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

