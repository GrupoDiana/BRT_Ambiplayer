/**
 * \class CAmbisonicSoundFieldController
 *
 * \brief Controller class for managing and processing Ambisonic sound fields.
 * 
 * \details This class handles the loading, conversion, and manipulation
 * of Ambisonic sound fields, including ACN–FuMa format conversions and
 * combination of multichannel ambisonic data. It is designed to integrate
 * with the LoudspeakerArrayController and AmbisonicSoundField components.
 *
 * \date November 2025
 *
 * \author R. Liceras-Ramírez  
 * \b Tutor: A. Reyes-Lecuona (University of Málaga)  
 * \b Institution: Universidad de Málaga
 *
 * \b Contact: areyes@uma.es
 *
 * \b Copyright: © 2025 University of Málaga. All rights reserved.
 *
 * \b Note: This class is part of the Bachelor's Thesis of R. Liceras-Ramírez
 * developed at the University of Málaga under the supervision of
 * A. Reyes-Lecuona.
 *
 * \b Licence: This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or 
 * (at your option) any later version.
 */



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

std::vector<int> ACN_to_FuMa_order_0_3 = {
	0, // W
	3, 1, 2, // Y,Z,X
	6, 7, 5, 8, 4, // V,T,R,S,U
	12, 13, 11, 14, 10, 15, 9 // Q,O,M,K,L,N,P
};

class CAmbisonicSoundFieldController {
public:
	CAmbisonicSoundFieldController(): soundFieldController(), setupDone { false }, ordering { "" } { }

	void SetUp(size_t ambisonicOrder_, unsigned int sampleRate_, size_t blockSize_, std::string ordering_){
		// Inicializa el controlador interno de campo ambisónico
		soundFieldController.SetUp(ambisonicOrder_, sampleRate_, blockSize_);
		ordering = ordering_;

		setupDone = true;
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
	void LoadWav(const char * filePath, const std::string & ordering_) {
		if (!setupDone) return;

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

		for (int ch = 0; ch < numChannels; ++ch) {
			CMonoBuffer<float> channelData(numFrames);

			for (size_t i = 0; i < numFrames; ++i) {
				channelData[i] = audioFile.sample(i, ch);
			}

			soundFieldController.FillChannel(ch, channelData);

		}

		if (ordering_ == "ACN") {
			ConvertACNtoFuMa();
			ordering = "FuMa";
		}

		std::cout << "Loaded WAV: " << filePath
				  << " | Channels: " << numChannels
				  << " | Frames: " << numFrames
				  << " | SampleRate: " << fileSampleRate
				  << " | Ordering: " << ordering << std::endl;
	}

	
	// Save a sound field to a WAV file
	//Prototype
	 bool SaveToWav(const std::string & filePath) {
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
		uint32_t byteRate = 48000 * blockAlign;
		uint32_t dataSize = static_cast<uint32_t>(interleavedSamples.size() * sizeof(float));
		uint32_t chunkSize = 36 + dataSize;
		uint32_t subchunk1Size = 16;
		uint32_t sampleRate = 48000;

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
	}

	void ConvertACNtoFuMa() {
		if (!setupDone) return;

		std::vector<CMonoBuffer<float>> aux = soundFieldController.GetSoundFieldArray();
		int numChannels = pow(soundFieldController.GetOrder() + 1, 2);
		numChannels = std::min(numChannels, static_cast<int>(ACN_to_FuMa_order_0_3.size()));

		for (int i = 0; i < numChannels; i++) {
			soundFieldController.GetSoundFieldArray()[i] = aux[ACN_to_FuMa_order_0_3[i]];
		}
	}

	void ConvertFuMatoACN() {
		if (!setupDone) return;

		static std::vector<int> FuMa_to_ACN_order3 = []() {
			std::vector<int> inv(ACN_to_FuMa_order_0_3.size());
			for (size_t i = 0; i < ACN_to_FuMa_order_0_3.size(); ++i)
				inv[ACN_to_FuMa_order_0_3[i]] = i;
			return inv;
		}();

		std::vector<CMonoBuffer<float>> aux = soundFieldController.GetSoundFieldArray();
		int numChannels = pow(soundFieldController.GetOrder() + 1, 2);
		numChannels = std::min(numChannels, static_cast<int>(FuMa_to_ACN_order3.size()));

		for (int i = 0; i < numChannels; i++) {
			soundFieldController.GetSoundFieldArray()[i] = aux[FuMa_to_ACN_order3[i]];
		}
	}

void GenerateSyntheticYField(size_t numFrames = 1024) {
		if (!setupDone) {
			std::cerr << "Error: setup not done before generating synthetic field." << std::endl;
			return;
		}

		int order = 2;
		int numChannels = (order + 1) * (order + 1);
		soundFieldController.SetOrder(order);

		soundFieldController.GetSoundFieldArray().resize(numChannels);
		for (int ch = 0; ch < numChannels; ++ch) {
			soundFieldController.GetSoundFieldArray()[ch].resize(numFrames, 0.0f);
		}

		int channel = 4;

		// --- Generador de ruido gaussiano ---
		static std::mt19937 rng(std::random_device {}());
		static std::normal_distribution<float> gaussian(0.0f, 1.0f); // media=0, sigma=1

		for (size_t i = 0; i < numFrames; ++i) {
			float sample = gaussian(rng); // ruido blanco
			soundFieldController.GetSoundFieldArray()[channel][i] = sample;
		}

		std::cout << "Generated white noise in Ambisonic field" << std::endl;
	}


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
	std::string ordering;

	bool setupDone;
	CAmbisonicSoundField soundFieldController;
};

#endif // _CAMBISONIC_SOUNDFIELD_CONTROLLER_HPP_
