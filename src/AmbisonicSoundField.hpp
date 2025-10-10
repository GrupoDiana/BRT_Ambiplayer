#ifndef _CAMBISONIC_SOUND_FIELD_HPP_
#define _CAMBISONIC_SOUND_FIELD_HPP_

#include "LoudspeakerArrayController.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"

#include <Common/Buffer.hpp>
#include <cmath>
#include <string>
#include <vector>


class CAmbisonicSoundField {
public:
	CAmbisonicSoundField(): CAmbisonicSoundField(4, 48000, 1, 512) {}


	CAmbisonicSoundField(size_t numChannels_, unsigned int sampleRate_,size_t ambisonicOrder_, size_t bufferSize_)
		: numChannels(numChannels_)
		, sampleRate(sampleRate_)
		, ambisonicOrder(ambisonicOrder_)
		, bufferSize(bufferSize_) {
		if (numChannels == 0) {
			throw std::invalid_argument("numChannels must be > 0");
		}

		soundFieldArray.resize(numChannels);
	}


	CAmbisonicSoundField(int numChannels, size_t sampleRate, size_t bufferSize)
		: numChannels(numChannels)
		, sampleRate(sampleRate)
		, bufferSize(bufferSize) {
		if (numChannels <= 0) {
			throw std::invalid_argument("numChannels must be > 0");
		}

		// Calculate Ambisonic order
		int calculatedOrder = static_cast<int>(std::sqrt(numChannels)) - 1;
		if (calculatedOrder < 0 || std::pow(calculatedOrder + 1, 2) != numChannels) {
			throw std::runtime_error("Number of channels does not correspond to a valid Ambisonic order.");
		}
		ambisonicOrder = calculatedOrder;

		// Initialize buffers for each channel
		soundFieldArray.resize(numChannels);
		for (int ch = 0; ch < numChannels; ++ch) {
			soundFieldArray[ch].resize(bufferSize, 0.0f);
		}

		std::cout << "CAmbisonicSoundField created with "
				  << numChannels << " channels, order " << ambisonicOrder
				  << ", sampleRate " << sampleRate
				  << ", buffer size " << bufferSize << std::endl;
	}


	// Returns the field order
	int GetOrder() const {
		return ambisonicOrder;
	}

	// Returns the number of channels of the field
	int GetNumChannels() const {
		return numChannels;
	}

	int GetBufferSize() const {
		return bufferSize;
	}

	std::vector<CMonoBuffer<float>> & GetSoundFieldArray() {
		return soundFieldArray;
	}

	const std::vector<CMonoBuffer<float>> & GetSoundFieldArray() const {
		return soundFieldArray;
	}

	void SetSoundField(const CAmbisonicSoundField & other) {
		if (numChannels != other.numChannels || bufferSize != other.bufferSize) {
			throw std::runtime_error("Cannot set sound field: mismatched dimensions");
		}
		ambisonicOrder = other.ambisonicOrder;
		sampleRate = other.sampleRate;
		soundFieldArray = other.soundFieldArray;
	}


	/// Fill multi-channel soundFieldArray from multi-channel samples
	void CAmbisonicSoundField::FillBuffer(
		unsigned int & position,
		unsigned int & endFrame,
		CAmbisonicSoundField & source,
		size_t bufferSize) {
		const auto & sourceBuffers = source.GetSoundFieldArray();
		size_t numChannels = sourceBuffers.size();
		size_t sourceSize = sourceBuffers[0].size();

		if (soundFieldArray.size() != numChannels)
			soundFieldArray.resize(numChannels);

		for (size_t ch = 0; ch < numChannels; ++ch) {
			if (soundFieldArray[ch].size() != bufferSize)
				soundFieldArray[ch].resize(bufferSize, 0.0f);
		}

		endFrame = position + bufferSize - 1;

		for (size_t ch = 0; ch < numChannels; ++ch) {
			const auto & src = sourceBuffers[ch];
			auto & dest = soundFieldArray[ch];

			for (size_t i = 0; i < bufferSize; ++i) {
				size_t idx = position + i;
				if (idx < sourceSize)
					dest[i] = src[idx];
				else
					dest[i] = 0.0f; 
			}
		}

		position += bufferSize;
		if (position >= sourceSize)
			position = 0;
	}

	// Rotate the field depending on how coordinates have changed
	//To implement
	void RotateField(float yaw, float pitch, float roll);

	//Prototype
	CAmbisonicSoundField & operator+=(const CAmbisonicSoundField & other) {
		// Basic checks
		if (sampleRate != other.sampleRate) {
			throw std::runtime_error("Ambisonic fields must have the same order and sample rate to add.");
		}

		//Adjust orders with UpMixing(const CAmbisonicSoundField & other)

		// Add to this object using += of CBuffer
		for (size_t ch = 0; ch < soundFieldArray.size(); ++ch) {
			soundFieldArray[ch] += other.soundFieldArray[ch];
		}

		return *this;
	}

	CAmbisonicSoundField operator+(const CAmbisonicSoundField & other) const {
		CAmbisonicSoundField result = *this;
		result += other;
		return result;
	}

	CAmbisonicSoundField operator-(const CAmbisonicSoundField & other) const {
		CAmbisonicSoundField result = *this;
		result = result - other;
		return result;
	}

private:
	size_t ambisonicOrder;
	size_t numChannels;
	size_t sampleRate;
	size_t bufferSize;

	std::vector<CMonoBuffer<float>> soundFieldArray;
};

#endif // _CAMBISONIC_SOUND_FIELD_HPP_
