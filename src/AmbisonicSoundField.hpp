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
	CAmbisonicSoundField()
		: numChannels(4)
		, sampleRate(48000)
		, ambisonicOrder(1) // default to first-order ambisonic
	{
		soundFieldArray.resize(numChannels);
		for (int ch = 0; ch < numChannels; ++ch) {
			soundFieldArray[ch].resize(512, 0.0f); // default buffer size
		}

		std::cout << "CAmbisonicSoundField default created with "
				  << numChannels << " channels, order " << ambisonicOrder
				  << ", sampleRate " << sampleRate
				  << ", buffer size 512" << std::endl;
	}

	CAmbisonicSoundField(int numChannels, size_t sampleRate = 48000, size_t initialBufferSize = 512)
		: numChannels(numChannels)
		, sampleRate(sampleRate) {
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
			soundFieldArray[ch].resize(initialBufferSize, 0.0f); // allocate initial size with zeros
		}

		std::cout << "CAmbisonicSoundField created with "
				  << numChannels << " channels, order " << ambisonicOrder
				  << ", sampleRate " << sampleRate
				  << ", buffer size " << initialBufferSize << std::endl;
	}


	// Returns the field order
	int GetOrder() const {
		return ambisonicOrder;
	}

	// Returns the number of channels of the field
	int GetNumChannels() const {
		return numChannels;
	}

	std::vector<CMonoBuffer<float>> & GetSoundFieldArray() {
		return soundFieldArray;
	}

	const std::vector<CMonoBuffer<float>> & GetSoundFieldArray() const {
		return soundFieldArray;
	}

	/// Function to fill buffer with audio from the wav file
	void FillBuffer(CMonoBuffer<float> & output, unsigned int & position, unsigned int & endFrame, std::vector<float> & samplesVector) {
		position = endFrame + 1; // Set starting point as next sample of the end of last frame
		if (position >= samplesVector.size()) // If the end of the audio is met, the position variable must return to the beginning
			position = 0;

		endFrame = position + output.size() - 1; // Set ending point as starting point plus frame size
		for (int i = 0; i < output.size(); i++) {
			if ((position + i) < samplesVector.size())
				output[i] = (samplesVector[position + i]); // Fill with audio
			else
				output[i] = 0.0f; // Fill with zeros if the end of the audio is met
		}
	}

	// Save a sound field to a WAV file
	//Prototype
	bool SaveToWav(const std::string & filePath) {
		if (soundFieldArray.empty()) {
			std::cout << "No audio data to save." << std::endl;
			return false;
		}

		int totalChannels = static_cast<int>(soundFieldArray.size());
		size_t numSamples = soundFieldArray[0].size();

		// Verify consistent lengths
		for (int ch = 0; ch < totalChannels; ++ch) {
			if (soundFieldArray[ch].size() != numSamples) {
				std::cout << "Inconsistent channel lengths." << std::endl;
				return false;
			}
		}

		// Interleave samples (consecutive channels per frame)
		std::vector<float> interleavedSamples;
		interleavedSamples.reserve(numSamples * totalChannels);

		for (size_t i = 0; i < numSamples; ++i) {
			for (int ch = 0; ch < totalChannels; ++ch) {
				interleavedSamples.push_back(soundFieldArray[ch][i]);
			}
		}

		// Open file to write in binary
		std::ofstream outFile(filePath, std::ios::binary);
		if (!outFile) {
			std::cout << "Failed to open file for writing: " << filePath << std::endl;
			return false;
		}

		// --- WAV header ---
		uint16_t bitsPerSample = 32;
		uint16_t audioFormat = 3; // IEEE float
		uint16_t wavNumChannels = static_cast<uint16_t>(totalChannels);
		uint32_t sampleRate_ = sampleRate;
		uint32_t blockAlign = wavNumChannels * sizeof(float);
		uint32_t byteRate = sampleRate_ * blockAlign;
		uint32_t dataSize = interleavedSamples.size() * sizeof(float);
		uint32_t chunkSize = 36 + dataSize;
		uint32_t subchunk1Size = 16;

		// RIFF
		outFile.write("RIFF", 4);
		outFile.write(reinterpret_cast<const char *>(&chunkSize), 4);
		outFile.write("WAVE", 4);

		// fmt
		outFile.write("fmt ", 4);
		outFile.write(reinterpret_cast<const char *>(&subchunk1Size), 4);
		outFile.write(reinterpret_cast<const char *>(&audioFormat), 2);
		outFile.write(reinterpret_cast<const char *>(&wavNumChannels), 2);
		outFile.write(reinterpret_cast<const char *>(&sampleRate_), 4);
		outFile.write(reinterpret_cast<const char *>(&byteRate), 4);
		outFile.write(reinterpret_cast<const char *>(&blockAlign), 2);
		outFile.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

		// data
		outFile.write("data", 4);
		outFile.write(reinterpret_cast<const char *>(&dataSize), 4);
		outFile.write(reinterpret_cast<const char *>(interleavedSamples.data()), dataSize);

		outFile.close();

		std::cout << "Successfully saved WAV file: " << filePath << std::endl;
		return true;
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

private:
	size_t ambisonicOrder;
	size_t numChannels;
	size_t sampleRate;

	std::vector<CMonoBuffer<float>> soundFieldArray;
};

#endif // _CAMBISONIC_SOUND_FIELD_HPP_
