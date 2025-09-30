#ifndef _CAMBISONIC_SOUND_FIELD_CONTROLLER_HPP_
#define _CAMBISONIC_SOUND_FIELD_CONTROLLER_HPP_

#include "LoudspeakerArrayController.hpp"
#include "ofxAudioFile.h"
#include "ofxBRT.h"

#include <BRTLibrary.h>
#include <Common/Buffer.hpp>
#include <cmath>
#include <string>
#include <vector>

static const std::vector<std::vector<double>> NORM_MATRIX_O1 = {
	{ 1.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0 / std::sqrt(3.0), 0.0 },
	{ 0.0, 0.0, 0.0, 1.0 / std::sqrt(3.0) },
	{ 0.0, 1.0 / std::sqrt(3.0), 0.0, 0.0 }
};
static const std::vector<std::vector<double>> NORM_MATRIX_O2 = {
	{ std::sqrt(2.0), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, std::sqrt(3.0) / 2.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, std::sqrt(3.0) / 2.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, std::sqrt(3.0) / 2.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, std::sqrt(3.0) / 2.0, 0.0 }
};
static const std::vector<std::vector<double>> NORM_MATRIX_O3 = {
	{ 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0 / std::sqrt(3.0), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 1.0 / std::sqrt(3.0), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 1.0 / std::sqrt(3.0), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, std::sqrt(2.0) / 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, std::sqrt(2.0) / 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / std::sqrt(5.0), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / std::sqrt(5.0), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / std::sqrt(5.0), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / std::sqrt(6.0), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / std::sqrt(6.0), 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / std::sqrt(6.0), 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / std::sqrt(6.0), 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / std::sqrt(6.0), 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / std::sqrt(6.0), 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 / std::sqrt(6.0) }
};

class CAmbisonicSoundFieldController {
public:
	// Constructor: initialize soundFieldArray structure based on order and channels
	CAmbisonicSoundFieldController(int numChannels, std::string normalization)
		: numChannels(numChannels)
		, normalization(normalization) {
		// Resize for channels
		soundFieldArray.resize(numChannels);

		//calculate order
		int calculatedOrder = static_cast<int>(std::sqrt(numChannels)) - 1;
		if (calculatedOrder < 0 || std::pow(calculatedOrder + 1, 2) != numChannels) {
			std::cout << "Number of channels does not correspond to a valid ambisonic order." << std::endl;
		}
		ambisonicOrder = calculatedOrder;
	}

	// Load WAV file and fill soundFieldArray with samples mapped by ambisonic order and channel
	//Prototype
	bool LoadWav(const std::string & filePath, std::string normalization) {
		ofxAudioFile audioFile;
		audioFile.load(filePath);

		if (!audioFile.loaded()) {
			std::cout << "Error loading WAV file: " << filePath << std::endl;
			return false;
		}

		int fileChannels = audioFile.channels();
		if (fileChannels <= 0) {
			std::cout << "Invalid number of channels in WAV file." << std::endl;
			return false;
		}

		// Calculate ambisonic order
		int calculatedOrder = static_cast<int>(std::sqrt(fileChannels)) - 1;
		if (calculatedOrder < 0 || std::pow(calculatedOrder + 1, 2) != fileChannels) {
			std::cout << "Number of channels does not correspond to a valid ambisonic order." << std::endl;
			return false;
		}

		size_t numSamples = audioFile.length();

		// Resize soundFieldArray with correct number of channels
		soundFieldArray.resize(fileChannels);

		// Reserve memory for each channel
		for (int ch = 0; ch < fileChannels; ch++) {
			soundFieldArray[ch].reserve(numSamples);
		}

		// Fill with samples
		for (size_t i = 0; i < numSamples; ++i) {
			for (int ch = 0; ch < fileChannels; ch++) {
				float sample = audioFile.sample(i, ch);
				soundFieldArray[ch].push_back(sample);
			}
		}

		// Update class data
		numChannels = fileChannels;
		ambisonicOrder = calculatedOrder;
		sampleRate = audioFile.samplerate();

		//Normalization update missing
		if (normalization == "FuMa_B") {
			ApplySN3DNormalization(ambisonicOrder);
		}
		return true;
	}

	// Returns the field order
	int GetOrder() const {
		return ambisonicOrder;
	}

	// Returns the number of channels of the field
	int GetNumChannels() const {
		return numChannels;
	}

	// Normalize to SN3D
	void ApplySN3DNormalization(int ambisonicOrder) {
		const std::vector<std::vector<double>> * NORM_MATRIX = nullptr;

		if (ambisonicOrder == 1) {
			NORM_MATRIX = &NORM_MATRIX_O1;
		} else if (ambisonicOrder == 2) {
			NORM_MATRIX = &NORM_MATRIX_O2;
		} else if (ambisonicOrder == 3) {
			NORM_MATRIX = &NORM_MATRIX_O3;
		} else {
			throw std::runtime_error("Unsupported ambisonic order");
		}

		int numChannels = soundFieldArray.size();
		std::vector<CMonoBuffer<float>> newArray(numChannels);

		for (int i = 0; i < numChannels; i++) {
			newArray[i].resize(soundFieldArray[i].size(), 0.0f);
		}

		for (int i = 0; i < numChannels; i++) {
			for (int j = 0; j < numChannels; j++) {
				float gain = static_cast<float>((*NORM_MATRIX)[i][j]);
				for (size_t k = 0; k < soundFieldArray[j].size(); k++) {
					newArray[i][k] += soundFieldArray[j][k] * gain;
				}
			}
		}

		soundFieldArray = std::move(newArray);
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

	//Prototype
	CAmbisonicSoundFieldController & operator+=(const CAmbisonicSoundFieldController & other) {
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

	CAmbisonicSoundFieldController operator+(const CAmbisonicSoundFieldController & other) const {
		CAmbisonicSoundFieldController result = *this;
		result += other;
		return result;
	}

private:
	size_t ambisonicOrder;
	size_t numChannels;
	size_t sampleRate;
	std::string normalization;

	std::vector<CMonoBuffer<float>> soundFieldArray;
};

#endif // _CAMBISONIC_SOUND_FIELD_CONTROLLER_HPP_
