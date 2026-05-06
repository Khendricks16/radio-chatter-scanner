/**
 * @file radio.h
 * @author Keith Hendricks
 *
 * Header file for radio component that will expose functions used to setup and collect data from the
 * radio device.
 */

#ifndef RCS_RADIO_H
#define RCS_RADIO_H



/**
 * Performs a check that a radio device is available for the program to use and prints out the information
 * of the device that will be used (the first one available). 
 */
void radio_device_check();


/**
 * Collects ~40 MiB worth of data from the radio device on the given frequency.
 * Data collected will be stored in a file named "raw_radio_data.iq"
 * @param freq the current frequency that should be scanned for voice detection
 * @param timeout how long this frequency should be scanned for
 * @param demod_mode the enum value for the demod mode as seen in program_input_s
 */
void radio_scan_single_freq(uint32_t freq, float timeout, int demod_mode);


/**
 *
 *
 */
//void scan_range_linear(uint32_t start, uint32_t end, int hold_time, int cycles); 


#endif

