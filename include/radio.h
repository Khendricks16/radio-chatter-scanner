/**
 * @file radio.h
 * @author Keith Hendricks
 *
 * Header file for radio component that will expose functions used to setup and collect data from the
 * radio device.
 */

#ifndef radio
#define radio



/**
 * Performs setup and information annoucement of radio device
 * @param radioDev the radio device that will be used for the program
 */
void radioDeviceSetup(rtlsdr_dev_t **radioDev);


/**
 * Collects ~40 MiB worth of data from the radio device on the given frequency.
 * Data collected will be stored in a file named "raw_radio_data.iq"
 * @param radioDev the radio device that will collect data
 * @param freq the frequency for the data collection to take place on
 */
void radioScanFreq(rtlsdr_dev_t *radioDev, uint32_t freq);


#endif
