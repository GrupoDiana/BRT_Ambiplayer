#ifndef _CAMBISONIC_SOUNDFIELD_CONTROLLER_HPP_
#define _CAMBISONIC_SOUNDFIELD_CONTROLLER_HPP_

#include "LoudspeakerArrayController.hpp"
#include "AmbisonicSoundField.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"

#include <Common/Buffer.hpp>
#include <cmath>
#include <string>
#include <vector>

class CAmbisonicSoundFieldController {
public:
	CAmbisonicSoundFieldController(): soundFieldController(), setupDone { false } {}

	void SetUp(size_t ambisonicOrder_, unsigned int sampleRate_, size_t blockSize_){
		// Inicializa el controlador interno de campo ambisónico
		soundFieldController.SetUp(ambisonicOrder_, sampleRate_, blockSize_, "ACN");
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

		soundFieldController.GetSoundFieldArray().resize(numChannels);
		soundFieldController.SetOrder(static_cast<int>(sqrt(numChannels)) - 1);

		for (int ch = 0; ch < numChannels; ++ch) {
			soundFieldController.GetSoundFieldArray()[ch].resize(numFrames);
		}

		for (size_t i = 0; i < numFrames; ++i) {
			for (int ch = 0; ch < numChannels; ++ch) {
				soundFieldController.GetSoundFieldArray()[ch][i] = audioFile.sample(i, ch);
			}
		}

		if (ordering == "FuMa") {
			soundFieldController.ConvertFuMatoACN();
		}

		std::cout << "Loaded WAV: " << filePath
				  << " | Channels: " << numChannels
				  << " | Frames: " << numFrames
				  << " | SampleRate: " << fileSampleRate
				  << " | Ordering: " << ordering << std::endl;
	}

	
	// Save a sound field to a WAV file
	//Prototype
	/* bool SaveToWav(const std::string & filePath) {
		auto & in = soundFieldController.GetSoundFieldArray();
		size_t numChannels = in.size();

		if (numChannels == 0) {
			std::cout << "Error: no audio data available." << std::endl;
			return false;
		}

		size_t numSamples = in[0].size();
		// Verificar longitudes consistentes
		for (size_t ch = 0; ch < numChannels; ++ch) {
			if (in[ch].size() != numSamples) {
				std::cout << "Inconsistent channel lengths." << std::endl;
				return false;
			}
		}

		soundFieldController.ConvertACNtoFuMa();

		auto & audioArray = soundFieldController.GetSoundFieldArray();

		// --- Interleave ---
		std::vector<float> interleavedSamples;
		interleavedSamples.reserve(numSamples * numChannels);
		for (size_t i = 0; i < numSamples; ++i) {
			for (size_t ch = 0; ch < numChannels; ++ch) {
				interleavedSamples.push_back(audioArray[ch][i]);
			}
		}

		// --- WAV Header ---
		uint16_t bitsPerSample = 32;
		uint16_t audioFormat = 3; // IEEE float
		uint16_t wavNumChannels = static_cast<uint16_t>(numChannels);
		uint32_t blockAlign = wavNumChannels * sizeof(float);
		uint32_t byteRate = sampleRate * blockAlign;
		uint32_t dataSize = static_cast<uint32_t>(interleavedSamples.size() * sizeof(float));
		uint32_t chunkSize = 36 + dataSize;
		uint32_t subchunk1Size = 16;

		std::ofstream outFile(filePath, std::ios::binary);
		if (!outFile) {
			std::cout << "Failed to open file: " << filePath << std::endl;
			return false;
		}

		// RIFF
		outFile.write("RIFF", 4);
		outFile.write(reinterpret_cast<const char *>(&chunkSize), 4);
		outFile.write("WAVE", 4);

		// fmt
		outFile.write("fmt ", 4);
		outFile.write(reinterpret_cast<const char *>(&subchunk1Size), 4);
		outFile.write(reinterpret_cast<const char *>(&audioFormat), 2);
		outFile.write(reinterpret_cast<const char *>(&wavNumChannels), 2);
		outFile.write(reinterpret_cast<const char *>(&sampleRate), 4);
		outFile.write(reinterpret_cast<const char *>(&byteRate), 4);
		outFile.write(reinterpret_cast<const char *>(&blockAlign), 2);
		outFile.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

		// data
		outFile.write("data", 4);
		outFile.write(reinterpret_cast<const char *>(&dataSize), 4);
		outFile.write(reinterpret_cast<const char *>(interleavedSamples.data()), dataSize);

		outFile.close();
		std::cout << "Successfully saved WAV: " << filePath << std::endl;
		return true;
	}*/


	//Prototype
	CAmbisonicSoundFieldController & operator+=(const CAmbisonicSoundFieldController & other) {
		const auto & otherArray = other.soundFieldController.GetSoundFieldArray();
		auto & arr = soundFieldController.GetSoundFieldArray();
		for (size_t ch = 0; ch < pow(soundFieldController.GetOrder()+1,2); ++ch) {
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
	/* size_t ambisonicOrder;
	size_t sampleRate;
	int blockSize;*/

	bool setupDone;
	CAmbisonicSoundField soundFieldController;
};

#endif // _CAMBISONIC_SOUNDFIELD_CONTROLLER_HPP_
