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
 * \author R. Liceras-Ramírez under the supervision of A. Reyes-Lecuona and D. Gonzalez-Toledo (3DI-DIANA Research Group)
 * \b Institution: Universidad de Málaga
 *
 * \b Context:
 * Developed as part of a Bachelor's Thesis (TFG), this
 * class defines a sound field controller.
 *
  * \b License:
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * \b Copyright:
 * This library includes pieces of code from the BRT Library, shared under the GPLv3 license and copyrighted (c) by the University of Málaga
 *
 * \b Acknowledgement:
 * This project has also been partially funded by the Spanish Ministerio de Ciencia e Innovación under the SONIX project (PID2023-152547NB-I00). https://www.diana.uma.es/?page_id=203
 */



#ifndef _CAMBISONIC_SOUNDFIELD_CONTROLLER_HPP_
#define _CAMBISONIC_SOUNDFIELD_CONTROLLER_HPP_

#include "LoudspeakerArrayController.hpp"
#include "AmbisonicSoundField.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"
#include "CommonDefinitions.hpp"

#include <filesystem>
#include <Common/Buffer.hpp>
#include <cmath>
#include <string>
#include <vector>

using namespace CommonDefinitions;

std::vector<int> ACN_to_FuMa_order_0_3 = {
	0, // W
	3, 1, 2, // Y,Z,X
	6, 7, 5, 8, 4, // V,T,R,S,U
	12, 13, 11, 14, 10, 15, 9 // Q,O,M,K,L,N,P
};

struct Rotation {
	float yaw = 0.0f;
	float pitch = 0.0f;
	float roll = 0.0f;
};

class CAmbisonicSoundFieldController {
public:
	CAmbisonicSoundFieldController()
		: soundFieldController()
		, setupDone { false }
		, ordering { "" } { }

	void SetUp(size_t ambisonicOrder_, unsigned int sampleRate_, size_t blockSize_, std::string ordering_) {
		soundFieldController.SetUp(ambisonicOrder_, sampleRate_, blockSize_);
		ordering = ordering_;
		sampleRate = sampleRate_;
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

	unsigned int & GetWavSamplePositionSource() {
		return wavSamplePositionSource;
	}

	unsigned int & GetPositionEndFrameSource() {
		return positionEndFrameSource;
	}

	std::string & GetFilePathName() {
		return filePathName;
	}

	CQuaternion & GetQ() {
		return q;
	}

	Rotation & GetRotation() {
		return rotation;
	}

	// Load WAV file and fill soundFieldArray with samples mapped by ambisonic order and channel
	bool LoadWav(string filePath, const std::string & ordering_, int order) {
		if (!setupDone) return false;

		ofxAudioFile audioFile;
		audioFile.load(filePath);

		int numChannels = audioFile.channels();
		size_t numFrames = audioFile.length();
		int fileSampleRate = audioFile.samplerate();

		if (!audioFile.loaded() || numChannels != pow(order + 1, 2)) {
			std::cerr << "Error loading WAV: " << filePath << std::endl;
			return false;
		}

		filePathName = std::filesystem::path(filePath).filename().string() + "-" + ordering_;

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

		if (ordering_ == "FuMa") {
			ConvertFuMatoACN();
			ordering = "ACN";
		}

		std::cout << "Loaded WAV: " << filePath
				  << " | Channels: " << numChannels
				  << " | Frames: " << numFrames
				  << " | SampleRate: " << fileSampleRate
				  << " | Ordering: " << ordering_ << std::endl;
		return true;
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

		int order = 1;
		int numChannels = (order + 1) * (order + 1);
		soundFieldController.SetOrder(order);

		soundFieldController.GetSoundFieldArray().resize(numChannels);
		for (int ch = 0; ch < numChannels; ++ch) {
			soundFieldController.GetSoundFieldArray()[ch].resize(numFrames, 0.0f);
		}

		int channel = 0;

		// --- Gaussian noise ---
		static std::mt19937 rng(std::random_device {}());
		static std::normal_distribution<float> gaussian(0.0f, 1.0f); // media=0, sigma=1

		for (size_t i = 0; i < numFrames; ++i) {
			float sample = gaussian(rng); 
			soundFieldController.GetSoundFieldArray()[channel][i] = sample;
		}

		std::cout << "Generated white noise in Ambisonic field" << std::endl;
	}

	bool GeneratePuntualSourceField(int order, const char * filePath, Common::CVector3 pos) {
		if (!setupDone) {
			std::cerr << "Error: setup not done before generating synthetic field." << std::endl;
			return false;
		}

		ofxAudioFile audioFile;
		audioFile.load(filePath);

		if (!audioFile.loaded()) {
			std::cerr << "Error loading WAV: " << filePath << std::endl;
			return false;
		}

		float norm = 1.0f / std::sqrt(3.0f);
		int numChannels = (order + 1) * (order + 1);
		size_t numFrames = audioFile.length();
		int fileSampleRate = audioFile.samplerate();

		filePathName = std::filesystem::path(filePath).filename().string();

		soundFieldController.GetSoundFieldArray().resize(numChannels);
		soundFieldController.SetOrder(order);

		for (int ch = 0; ch < numChannels; ++ch) {
			soundFieldController.GetSoundFieldArray()[ch].resize(numFrames, 0.0f);
		}

		auto source = std::make_shared<BRTSourceModel::CSourceOmnidirectionalModel>("GNoise");
		if (!source) {
			std::cerr << "Error: could not create BRT source." << std::endl;
			return false;
		}

		CMonoBuffer<float> buffer(numFrames);
		for (size_t i = 0; i < numFrames; ++i) {
			buffer[i] = audioFile.sample(i, 0);
		}

		source->SetBuffer(buffer);

		Common::CVector3 sourcePos = pos;
		Common::CVector3 sourcePosNorm = Normalize(sourcePos);

		EncodePuntualSourceToAmbisonic(*source, soundFieldController, sourcePosNorm);
		return true;
	}

	void Play(bool m) { isPlaying = m; }
	void Stop() { isPlaying = false; }
	void Mute(bool m) { isMuted = m; }
	void Solo(bool m) { isSolo = m; }
	void Loop(bool m) {
		isLoop = m;
	}

	bool IsPlaying() const { return isPlaying; }
	bool & GetPlaying() { return isPlaying; }
	bool IsMuted() const { return isMuted; }
	bool IsSolo() const { return isSolo; }
	bool IsLoop() const { return isLoop; }

	float GetPlaybackProgress() const {
		const auto & arr = soundFieldController.GetSoundFieldArray();
		if (arr.empty() || arr[0].empty())
			return 0.0f;

		return static_cast<float>(wavSamplePositionSource) / static_cast<float>(arr[0].size());
	}

	void SetPlaybackProgress(float norm) {
		const auto & arr = soundFieldController.GetSoundFieldArray();
		if (arr.empty() || arr[0].empty())
			return;

		norm = std::clamp(norm, 0.0f, 1.0f);
		wavSamplePositionSource = static_cast<unsigned int>(norm * arr[0].size());

		if (!isPlaying)
			isPlaying = true;
	}

	float GetTotalTimeSeconds() const {
		const auto & arr = soundFieldController.GetSoundFieldArray();
		if (arr.empty() || arr[0].empty() || sampleRate == 0)
			return 0.0f;

		return static_cast<float>(arr[0].size()) / static_cast<float>(sampleRate);
	}

	float GetCurrentTimeSeconds() const {
		if (sampleRate == 0)
			return 0.0f;

		return static_cast<float>(wavSamplePositionSource) / static_cast<float>(sampleRate);
	}

	void SetPlaybackVolume(float norm) {
		norm = std::clamp(norm, 0.0f, 1.0f);
		volume = norm * 1.25f;
	}

	float GetPlaybackVolume() {
		return volume;
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
	int sampleRate;

	bool setupDone;
	CAmbisonicSoundField soundFieldController;
	CQuaternion q;
	unsigned int wavSamplePositionSource, positionEndFrameSource;
	std::string filePathName;

	bool isPlaying=false;
	bool isMuted=false;
	bool isSolo = false;
	bool isLoop = false;

	float volume = 1.0f;

	Rotation rotation;

	Common::CVector3 Normalize(const Common::CVector3 & v) {
		float mag = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

		if (mag < 1e-6f)
			return Common::CVector3(0, 0, 0); 

		return Common::CVector3(v.x / mag, v.y / mag, v.z / mag);
	}


	void EncodePuntualSourceToAmbisonic(
		BRTSourceModel::CSourceOmnidirectionalModel & source,
		CAmbisonicSoundField & ambiField,
		const Common::CVector3 & sourcePos) {
		int order = ambiField.GetOrder();

		const auto & buffer = source.GetBuffer(); // std::vector<float>
		size_t numFrames = buffer.size();

		int numChannels = (order + 1) * (order + 1);

		ambiField.GetSoundFieldArray().resize(numChannels);
		for (int ch = 0; ch < numChannels; ++ch) {
			ambiField.GetSoundFieldArray()[ch].resize(numFrames, 0.0f);
		}

		for (size_t n = 0; n < numFrames; ++n) {
			float sample = buffer[n];

			if (order >= 1) {
				float W = sample;
				float Y = sample * sourcePos.y;
				float Z = sample * sourcePos.z;
				float X = sample * sourcePos.x;

				ambiField.GetSoundFieldArray()[0][n] = W;
				ambiField.GetSoundFieldArray()[1][n] = Y;
				ambiField.GetSoundFieldArray()[2][n] = Z;
				ambiField.GetSoundFieldArray()[3][n] = X;
			}

			if (order >= 2 ) {
				float V = sample * sourcePos.x * sourcePos.y * SQRT_3;
				float T = sample * sourcePos.y * sourcePos.z * SQRT_3;
				float R = sample * (0.5f * (3.f * sourcePos.z * sourcePos.z - 1.f));
				float S = sample * SQRT_3 * sourcePos.x * sourcePos.z;
				float U = sample * SQRT_3_DIV_4 * (sourcePos.x * sourcePos.x - sourcePos.y * sourcePos.y);

				ambiField.GetSoundFieldArray()[4][n] = V;
				ambiField.GetSoundFieldArray()[5][n] = T;
				ambiField.GetSoundFieldArray()[6][n] = R;
				ambiField.GetSoundFieldArray()[7][n] = S;
				ambiField.GetSoundFieldArray()[8][n] = U;
			}

			if (order >= 3 ) {
				float Q = sample * SQRT_5_DIV_8 * sourcePos.y * (3.f * sourcePos.x * sourcePos.x - sourcePos.y * sourcePos.y);
				float O = sample * SQRT_15 * sourcePos.x * sourcePos.y * sourcePos.z;
				float M = sample * SQRT_3_DIV_8 * sourcePos.y * (5.f * sourcePos.z * sourcePos.z - 1.f);
				float K = sample * 0.5f * sourcePos.z * (5.f * sourcePos.z * sourcePos.z - 3.f);
				float L = sample * SQRT_3_DIV_8 * sourcePos.x * (5.f * sourcePos.z * sourcePos.z - 1.f);
				float N = sample * SQRT_15_DIV_4 * sourcePos.z * (sourcePos.x * sourcePos.x - sourcePos.y * sourcePos.y);
				float P = sample * SQRT_5_DIV_8 * sourcePos.x * (sourcePos.x * sourcePos.x - 3.f * sourcePos.y * sourcePos.y);

				ambiField.GetSoundFieldArray()[9][n] = Q;
				ambiField.GetSoundFieldArray()[10][n] = O;
				ambiField.GetSoundFieldArray()[11][n] = M;
				ambiField.GetSoundFieldArray()[12][n] = K;
				ambiField.GetSoundFieldArray()[13][n] = L;
				ambiField.GetSoundFieldArray()[14][n] = N;
				ambiField.GetSoundFieldArray()[15][n] = P;
			}
		}
	}
};

#endif // _CAMBISONIC_SOUNDFIELD_CONTROLLER_HPP_
