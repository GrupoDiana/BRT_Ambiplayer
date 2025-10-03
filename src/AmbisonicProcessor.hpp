#ifndef _CAMBISONIC_PROCESSOR_HPP_
#define _CAMBISONIC_PROCESSOR_HPP_

#include "AmbisonicSoundField.hpp"
#include "LoudSpeakerArrayController.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"

#include <Common/Buffer.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

class CAmbisonicProcessor {
public:
	// Constructor: initialize soundFieldArray structure based on number of channels
	CAmbisonicProcessor()
		: numChannels(4)
		, sampleRate(48000)
		, blockSize(512)
		, ambisonicProcessor(numChannels) {
		if (numChannels <= 0) {
			throw std::invalid_argument("numChannels must be > 0");
		}
		ambisonicOrder = static_cast<size_t>(std::sqrt(numChannels) - 1);
		std::cout << "CAmbisonicProcessor created with " << numChannels
				  << " channels, order " << ambisonicOrder
				  <<std::endl;
	}

	// Rotate the field depending on listener rotation
	void RotateField(float yaw, float pitch, float roll) {
		// TODO: implement ambisonic rotation
	}

	// Decode ambisonic field to a speaker array
	void DecodeToSpeakerArray(CLoudSpeakerArrayController & speakerArray) const {
		auto & speakers = speakerArray.GetSpeakers();
		size_t numSpeakers = speakers.size();
		size_t numChannels = ambisonicProcessor.GetNumChannels();

		if (numChannels == 0 || numSpeakers == 0) {
			std::cout << "No channels or no speakers available for decoding." << std::endl;
			return;
		}

		// Loop over each speaker
		for (size_t sp = 0; sp < numSpeakers; ++sp) {
			CMonoBuffer<float> speakerBuffer;
			speakerBuffer.resize(ambisonicProcessor.GetSoundFieldArray()[0].size(), 0.0f);

			// Simple ACN/N3D decoding: equal contribution from each ambisonic channel
			for (size_t ch = 0; ch < numChannels; ++ch) {
				for (size_t i = 0; i < speakerBuffer.size(); ++i) {
					speakerBuffer[i] += ambisonicProcessor.GetSoundFieldArray()[ch][i] / static_cast<float>(numChannels);
				}
			}

			// Assign the buffer to the speaker
			speakers[sp]->SetBuffer(speakerBuffer);
		}
	}

	// Sum ambisonic fields
	CAmbisonicProcessor & operator+=(const CAmbisonicProcessor & other) {
		if (numChannels != other.numChannels || blockSize != other.blockSize) {
			throw std::runtime_error("Cannot sum ambisonic blocks with different dimensions");
		}

		auto & arr = ambisonicProcessor.GetSoundFieldArray();
		const auto & otherArray = other.ambisonicProcessor.GetSoundFieldArray();

		for (size_t ch = 0; ch < numChannels; ++ch) {
			arr[ch] += otherArray[ch];
		}

		return *this;
	}

	CAmbisonicProcessor operator+(const CAmbisonicProcessor & other) const {
		CAmbisonicProcessor result = *this;
		result += other;
		return result;
	}

	// Accessor for the internal ambisonic sound field
	CAmbisonicSoundField & GetSoundFieldController() { return ambisonicProcessor; }
	const CAmbisonicSoundField & GetSoundFieldController() const { return ambisonicProcessor; }

private:
	size_t ambisonicOrder;
	size_t numChannels;
	size_t sampleRate;
	int blockSize;

	CAmbisonicSoundField ambisonicProcessor;
};

#endif // _CAMBISONIC_PROCESSOR_HPP_
