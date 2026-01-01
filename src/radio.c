/**
 * @file radio.c
 * @author Keith Hendricks
 *
 */

#include <rtl-sdr.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/radio.h"

#define NO_RADIO_DEVICE_FOUND 4

#define DEV_INFO_STR_LEN 256


void radioDeviceSetup(rtlsdr_dev_t **radioDev)
{
    // Make sure that at least one rtlsdr device can be found
    if (rtlsdr_get_device_count() == 0){
        fprintf(stderr, "No rtlsdr device could be found, please connect and retry\n");
        exit(NO_RADIO_DEVICE_FOUND);
    }

    // Open device
    rtlsdr_open(radioDev, 0);

    // Announce the device that was opened up
    fprintf(stdout, "%s\n", rtlsdr_get_device_name(0));
    char manufacturer[DEV_INFO_STR_LEN];
    char productName[DEV_INFO_STR_LEN];
    rtlsdr_get_usb_strings(*radioDev, manufacturer, productName, NULL);
    fprintf(stdout, "Device Manufacturer: %s\n", manufacturer);
    fprintf(stdout, "Device Product Name: %s\n", productName);
}


struct contextData {
    FILE *fp;
    unsigned short n;
    rtlsdr_dev_t *radioDev;
};


static void recordRadioData(unsigned char *buf, uint32_t len, void *ctx)
{
    struct contextData *ctxData = (struct contextData *)ctx;
    ctxData->n++;

    if (len > 0){
        printf("Read in %d bytes of radio data\n", len);
        fwrite(buf, sizeof(unsigned char), len, ctxData->fp);
    }

    if (ctxData->n == 80){
        rtlsdr_cancel_async(ctxData->radioDev);
    }
    
}


void radioScanFreq(rtlsdr_dev_t *radioDev, uint32_t freq)
{
    // Dev config
    rtlsdr_set_center_freq(radioDev, freq);
    rtlsdr_set_sample_rate(radioDev, 2400000); // 2.4 MSps
    rtlsdr_set_tuner_gain_mode(radioDev, 0); // Automatic gain mode
    rtlsdr_reset_buffer(radioDev);
    
    // Annouce the current frequency we are on
    printf("Tuned Frequency: %d\n", rtlsdr_get_center_freq(radioDev));

    FILE *fp = fopen("raw_radio_data.iq", "wb+");

    // Create contextData for callback function
    struct contextData *ctxData = (struct contextData *) malloc(sizeof(struct contextData));
    ctxData->fp = fp;
    ctxData->n = 0;
    ctxData->radioDev = radioDev;

    // Read in data for the current freq
    rtlsdr_read_async(radioDev, recordRadioData, ctxData, 0, 512 * 1000);

    return;
}

