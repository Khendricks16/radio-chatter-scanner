/**
 * @file vad.h
 * @author Keith Hendricks
 * 
 * Header file that will act as the C wrapper for C++ logic contained within vad.cc
 * (Functions that are invoked from other C code)
 */
#ifndef RCS_VAD_H
#define RCS_VAD_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * C wrapper to make an instance of the RcsVad class defined in vad.cc
     * @param sample_rate the sample rate the vad model should use (1600Hz OR 8000Hz)
     * @param threshold the threshold for the underlying vad model
     * @return a void pointer to the created object instance */
    void *rcs_vad_create(float sample_rate, float threshold);
    /**
     * C wrapper to destroy a given instance of the RcsVad class defined in vad.cc
     * @param rcs_vad void pointer to the created RcsVad object instance
     */
    void rcs_vad_destroy(void *rcs_vad);
    /**
     * C wrapper to invoke RcsVad.has_voice_activity() on a given RcsVad instance.
     * @param rcs_vad void pointer to the created RcsVad object instance
     * @param data the array of signed 16 bit signed ints produced by the rtl_fm process
     * @param data_len the length of the data array
     * @return a true or false value if voice activity is detected in the captured output
     *
     */
    bool rcs_vad_determine_voice_activity(void *rcs_vad, int16_t *data, int dataLen);

#ifdef __cplusplus
}
#endif

#endif