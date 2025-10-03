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
	CAmbisonicSoundFieldController(): numChannels(4), sampleRate(44100), blockSize(512)
	{
		if (numChannels <= 0) {
			throw std::invalid_argument("numChannels must be > 0");
		}

		// Ambisonic order is usually derived from number of channels: (N+1)^2 = channels
		// So N = sqrt(numChannels) - 1
		ambisonicOrder = static_cast<size_t>(std::sqrt(numChannels) - 1);

		// Inicializar el procesador interno (simbolizado aquí)
		soundFieldController = CAmbisonicSoundField(ambisonicOrder);

		// Mensaje de debug (opcional)
		std::cout << "CAmbisonicSoundFieldController creado con "
				  << numChannels << " canales, orden "
				  << ambisonicOrder << std::endl;
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
	bool LoadWav(const char * filePath, std::string ordering) {
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
		soundFieldController.GetSoundFieldArray().resize(fileChannels);

		// Reserve memory for each channel
		for (int ch = 0; ch < fileChannels; ch++) {
			soundFieldController.GetSoundFieldArray()[ch].reserve(numSamples);
		}

		// Fill with samples
		for (size_t i = 0; i < numSamples; ++i) {
			for (int ch = 0; ch < fileChannels; ch++) {
				float sample = audioFile.sample(i, ch);
				soundFieldController.GetSoundFieldArray()[ch].push_back(sample);
			}
		}

		// Update class data
		numChannels = fileChannels;
		ambisonicOrder = calculatedOrder;
		sampleRate = audioFile.samplerate();

		//Normalization update missing
		if (ordering == "FuMa_B") {
			ApplyOrdering(ambisonicOrder);
		}
		return true;
	}

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
