/**
 * @file vad.cc
 * @author Keith Hendricks
 * 
 * Implementation file that contains the logic for determining if speech is present within our captured radio data.
 */
#include <iostream>
#include "silero.hpp"

#include <limits.h>

/**
 * Class that contains the logic for determining if speech is present within our captured radio data.
 * @author Keith Hendricks
 */
class RcsVad {
	private:
		/** Path to the underlying onnx model being used */	  
		static constexpr const char* MODEL_PATH = "model/silero_vad.onnx";
		/** The value used to convert our 16 bit signed audio data to float values silero vad can understand */
		static constexpr const float NORMALIZATION_FACTOR = INT16_MAX + 1.0f;

	 	/** The object used for gathering the speech timestamps of voice activity given data */
		silero::VadIterator vad = silero::VadIterator(MODEL_PATH);


	public:
		/**
		 * Constructor method which sets up the underlying vad object used to determine if speech is present
		 * given data.
		 * @param sample_rate the sample rate the vad model should use (1600Hz OR 8000Hz)
		 * @param threshold the threshold for the underlying vad model
		 */
		RcsVad(float sample_rate=16000, float threshold=0.5)
		{
			// More set up logic for vad after initalization
			vad.threshold = threshold;	 
			vad.sample_rate = sample_rate; 
			vad.print_as_samples = false;
			vad.SetVariables();
		}

		/**
		 * Method used to return a true or false value if the given output from a rtl_fm process (16 bit signed ints)
		 * is captured audio that has voice activity in it or not.
		 * @param data the array of signed 16 bit signed ints
		 * @param data_len the length of the data array
		 * @return a true or false value if voice activity is detected in the captured output
		 */
		bool has_voice_activity(int16_t *data, int data_len){
			// Convert the given signed 16 bit radio data to something our vad model can process
			std::vector<float> input_data;
			input_data.reserve(data_len);
			for (int i = 0; i < data_len; i++){
				input_data.push_back(data[i] / NORMALIZATION_FACTOR);
			}

			// Get the speech timestamps if there are any present
			vad.SpeechProbs(input_data);
			std::vector<silero::Interval> speeches = vad.GetSpeechTimestamps();

			// Was there any speech detected?
			return speeches.size() > 0;
		}
};


// =====================================
// Pure C API function implementation
// =====================================
#ifdef __cplusplus
extern "C" {
#endif

void *rcs_vad_create(float sample_rate, float threshold)
{
	// Instantiate the vad and return a pointer to the memory address of it
	RcsVad *vad = new RcsVad(sample_rate, threshold);
	return (void *)vad;
}

void rcs_vad_destroy(void *rcs_vad)
{
	RcsVad *vad = (RcsVad *)rcs_vad;
	delete vad;
}

bool rcs_vad_determine_voice_activity(void *rcs_vad, int16_t *data, int data_len)
{
	RcsVad *vad = (RcsVad *) rcs_vad;
	return vad->has_voice_activity(data, data_len);
}

#ifdef __cplusplus
}
#endif
