#ifndef _CAMBISONIC_PROCESSOR_HPP_
#define _CAMBISONIC_PROCESSOR_HPP_

#include "LoudspeakerArrayController.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"

#include <Common/Buffer.hpp>
#include <cmath>
#include <string>
#include <vector>

class CAmbisonicProcessor {
public:
	// Constructor: initialize soundFieldArray structure based on order and channels
	CAmbisonicProcessor(int numChannels, std::string normalization) {
	}

	// Rotate the field depending on how coordinates have changed
	//To implement
	void RotateField(float yaw, float pitch, float roll);

	// Convert the field to speaker array
	//To implement
	void DecodeToSpeakerArray(const CLoudSpeakerArrayController & array) const;

	// Convert from a lower-order field to a higher-order field
	//To implement
	void UpMixing(const CAmbisonicProcessor & other);

	//Prototype
	CAmbisonicProcessor & operator+=(const CAmbisonicProcessor & other) {
		if (numChannels != other.numChannels || blockSize != other.blockSize) {
			throw std::runtime_error("Cannot sum ambisonic blocks with different dimensions");
		}
		for (size_t ch = 0; ch < numChannels; ++ch) {
			soundFieldSampleArray[ch] += other.soundFieldSampleArray[ch];
		}
		return *this;
	}

	CAmbisonicProcessor operator+(const CAmbisonicProcessor & other) const {
		CAmbisonicProcessor result = *this;
		result += other;
		return result;
	}

private:

	size_t ambisonicOrder;
	size_t numChannels;
	size_t sampleRate;
	int blockSize;

	std::vector<CMonoBuffer<float>> soundFieldSampleArray;
};

#endif // _CAMBISONIC_PROCESSOR_HPP_
