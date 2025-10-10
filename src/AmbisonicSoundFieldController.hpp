#ifndef _CAMBISONIC_SOUNDFIELD_CONTROLLER_HPP_
#define _CAMBISONIC_SOUNDFIELD_CONTROLLER_HPP_

#include "LoudspeakerArrayController.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"

#include <Common/Buffer.hpp>
#include <cmath>
#include <string>
#include <vector>


static const std::vector<std::vector<double>> NORM_MATRIX_O1 = {
	{ 1.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0, 0.0 },
	{ 0.0, 0.0, 0.0, 1.0 },
	{ 0.0, 1.0, 0.0, 0.0 }
};
static const std::vector<std::vector<double>> NORM_MATRIX_O2 = {
	{ 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 }
};
static const std::vector<std::vector<double>> NORM_MATRIX_O3 = {
	{ 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 }
};

class CAmbisonicSoundFieldController {
public:
	CAmbisonicSoundFieldController(): numChannels(4), sampleRate(48000), blockSize(512)
	{
		if (numChannels <= 0) {
			throw std::invalid_argument("numChannels must be > 0");
		}
		// Ambisonic order is usually derived from number of channels: (N+1)^2 = channels
		// So N = sqrt(numChannels) - 1
		ambisonicOrder = static_cast<size_t>(std::sqrt(numChannels) - 1);

		soundFieldController = CAmbisonicSoundField(numChannels,sampleRate,blockSize);

	}

	// Returns a reference to the internal sound field controller
	CAmbisonicSoundField & GetSoundFieldController() {
		return soundFieldController;
	}

	// Const version for read-only access
	const CAmbisonicSoundField & GetSoundFieldController() const {
		return soundFieldController;
	}

	// Load WAV file and fill soundFieldArray with samples mapped by ambisonic order and channel
	//Prototype
	void LoadWav(const char * filePath, const std::string & ordering) {
		ofxAudioFile audioFile;
		audioFile.load(filePath);

		if (!audioFile.loaded()) {
			std::cerr << "Error loading WAV: " << filePath << std::endl;
			return;
		}

		int numChannels = audioFile.channels();
		size_t numFrames = audioFile.length(); 
		int fileSampleRate = audioFile.samplerate(); 

		auto & buffers = soundFieldController.GetSoundFieldArray();
		buffers.clear();
		buffers.resize(numChannels);

		for (int ch = 0; ch < numChannels; ++ch) {
			buffers[ch].resize(numFrames);
		}

		for (size_t i = 0; i < numFrames; ++i) {
			for (int ch = 0; ch < numChannels; ++ch) {
				buffers[ch][i] = audioFile.sample(i, ch);
			}
		}

		std::cout << "Loaded WAV: " << filePath
				  << " | Channels: " << numChannels
				  << " | Frames: " << numFrames
				  << " | SampleRate: " << fileSampleRate
				  << " | Ordering: " << ordering << std::endl;
	}

	/*
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
	}*/

	//Prototype
	CAmbisonicSoundFieldController & operator+=(const CAmbisonicSoundFieldController & other) {
		if (numChannels != other.numChannels || blockSize != other.blockSize) {
			throw std::runtime_error("Cannot sum ambisonic blocks with different dimensions");
		}

		const auto & otherArray = other.soundFieldController.GetSoundFieldArray();
		auto & arr = soundFieldController.GetSoundFieldArray();
		for (size_t ch = 0; ch < numChannels; ++ch) {
			arr[ch] += other.soundFieldController.GetSoundFieldArray()[ch];
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
	int blockSize;

	CAmbisonicSoundField soundFieldController;

	// Apply ACN ordering
	void ApplyOrdering(int ambisonicOrder) {
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

		int numChannels = soundFieldController.GetSoundFieldArray().size();
		std::vector<CMonoBuffer<float>> newArray(numChannels);

		for (int i = 0; i < numChannels; i++) {
			newArray[i].resize(soundFieldController.GetSoundFieldArray()[i].size(), 0.0f);
		}

		for (int i = 0; i < numChannels; i++) {
			for (int j = 0; j < numChannels; j++) {
				float gain = static_cast<float>((*NORM_MATRIX)[i][j]);
				for (size_t k = 0; k < soundFieldController.GetSoundFieldArray()[j].size(); k++) {
					newArray[i][k] += soundFieldController.GetSoundFieldArray()[j][k] * gain;
				}
			}
		}

		soundFieldController.GetSoundFieldArray() = std::move(newArray);
	}
};

#endif // _CAMBISONIC_SOUNDFIELD_CONTROLLER_HPP_
