/**
 * @file silero.hpp
 * @author NathanJHLee
 * @author Keith Hendricks
 * Created On  : 2025-11-10
 * Description : silero 6.2 system for onnx-runtime(c++)
 * Version     : 1.3
 */

#ifndef SILERO_H
#define SILERO_H

#include "onnxruntime_cxx_api.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <cstring>

namespace silero
{
	struct Interval
	{
		float start;
		float end;
		int numberOfSubseg;
		void initialize()
		{
			start = 0;
			end = 0;
			numberOfSubseg = 0;
		}
	};


	class VadIterator
	{
		public:
			VadIterator(const std::string &model_path,
						float threshold = 0.5,
						int sample_rate = 16000,
						int window_size_ms = 32,
						int speech_pad_ms = 30,
						int min_silence_duration_ms = 100,
						int min_speech_duration_ms = 250,
						int max_duration_merge_ms = 300,
						bool print_as_samples = false);
			~VadIterator();
			// Batch (non-streaming) interface (for backward compatibility)
			void SpeechProbs(std::vector<float> &input_wav);
			std::vector<Interval> GetSpeechTimestamps();
			void SetVariables();
			// Public parameters (can be modified by user)
			float threshold;
			int sample_rate;
			int window_size_ms;
			int min_speech_duration_ms;
			int max_duration_merge_ms;
			bool print_as_samples;

		private:
			Ort::Env env;								// Environment object
			Ort::SessionOptions session_options;		// Session options
			std::shared_ptr<Ort::Session> session;		// ONNX session
			Ort::AllocatorWithDefaultOptions allocator; // Default allocator
			Ort::MemoryInfo memory_info;				// Memory info (CPU)
			void init_onnx_model(const std::string &model_path);
			float predict(const std::vector<float> &data_chunk);
			// const int context_samples;                       // e.g. 64 samples
			int context_samples;		 // e.g. 64 samples
			std::vector<float> _context; // All initial values are 0
			int effective_window_size;
			// ONNX input/output related buffers and node names
			std::vector<Ort::Value> ort_inputs;
			std::vector<const char *> input_node_names;
			std::vector<float> input;
			unsigned int size_state; // Fixed value: 2*1*128
			std::vector<float> _state;
			std::vector<int64_t> sr;
			int64_t input_node_dims[2];		  // [1, effective_window_size]
			const int64_t state_node_dims[3]; // [ 2, 1, 128 ]
			const int64_t sr_node_dims[1];	  // [ 1 ]
			std::vector<Ort::Value> ort_outputs;
			std::vector<const char *> output_node_names; // Default: [ "output", "stateN" ]
			std::vector<float> outputs_prob;			 // used in batch mode
			int min_silence_samples;
			int min_speech_samples;
			int speech_pad_samples;
			int window_size_samples;
			int duration_merge_samples;
			int current_sample = 0;
			int total_sample_size = 0;
			int min_silence_duration_ms;
			int speech_pad_ms;
			bool triggered = false;
			int temp_end = 0;
			int global_end = 0;
			int erase_tail_count = 0;
			void init_engine(int window_size_ms);
			void reset_states();
			std::vector<Interval> DoVad();
	};

} // namespace silero

#endif // SILERO_H