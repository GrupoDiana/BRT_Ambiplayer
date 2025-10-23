#ifndef _CAMBISONIC_SOUND_FIELD_HPP_
#define _CAMBISONIC_SOUND_FIELD_HPP_

#include "LoudspeakerArrayController.hpp"
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


class CAmbisonicSoundField {
public:
	CAmbisonicSoundField(): sampleRate(-1), ambisonicOrder(-1), bufferSize(-1), ordering(""), setupDone {false} {}

	void SetUp(int ambisonicOrder_, size_t sampleRate_, size_t bufferSize_, std::string ordering_){
		ambisonicOrder = ambisonicOrder_;
		sampleRate = sampleRate_;
		bufferSize = bufferSize_;
		ordering = ordering_;

		// Initialize buffers for each channel
		int calculatedChannels = pow(ambisonicOrder + 1, 2);
		soundFieldArray = std::vector<CMonoBuffer<float>>(calculatedChannels, CMonoBuffer<float>(bufferSize, 0.0f));

		if (ordering == "FuMa") {
			ConvertFuMatoACN();
		}
		
		setupDone = true;
		std::cout << "CAmbisonicSoundField created with "
				  << calculatedChannels << " channels, order " << ambisonicOrder
				  << ", sampleRate " << sampleRate
				  << ", buffer size " << bufferSize << std::endl;
	}


	// Returns the field order
	int GetOrder() const {
		if (!setupDone) return -1;
		return ambisonicOrder;
	}

	void SetOrder(int order) {
		if (!setupDone) return;
		ambisonicOrder=order;
	}

	int GetBufferSize() const {
		if (!setupDone) return -1;
		return bufferSize;
	}

	std::vector<CMonoBuffer<float>> & GetSoundFieldArray() {
		if (!setupDone) return std::vector<CMonoBuffer<float>>();
		return soundFieldArray;
	}

	const std::vector<CMonoBuffer<float>> & GetSoundFieldArray() const {
		if (!setupDone) return std::vector<CMonoBuffer<float>>();
		return soundFieldArray;
	}

	void SetSoundField(const CAmbisonicSoundField & other) {
		if (!setupDone) return;
			sampleRate = other.sampleRate;
			soundFieldArray = other.soundFieldArray;
	}


	/// Fill multi-channel soundFieldArray from multi-channel samples
	void GetSegment(unsigned int & position,
		unsigned int & endFrame,
		CAmbisonicSoundField & source,
		size_t bufferSize) {
		if (!setupDone) return;

		const auto & sourceBuffers = source.GetSoundFieldArray();
		size_t numChannels = sourceBuffers.size();
		size_t sourceSize = sourceBuffers[0].size();

		for (size_t i = 0; i < numChannels; i++) {
			if (soundFieldArray[i].size() != bufferSize)
				soundFieldArray[i].resize(bufferSize, 0.0f);
		}

		for (size_t ch = 0; ch < numChannels; ++ch) {
			const auto & src = sourceBuffers[ch];
			auto & dest = soundFieldArray[ch];

			for (size_t i = 0; i < bufferSize; ++i) {
				size_t idx = (position + i) % sourceSize;
				dest[i] = src[idx];
			}
		}

		endFrame = (position + bufferSize - 1) % sourceSize;
		position = (position + bufferSize) % sourceSize;
	}


	void ConvertACNtoFuMa() {
		if (!setupDone) return;

		std::vector<CMonoBuffer<float>> aux = soundFieldArray;
		int numChannels = pow(ambisonicOrder + 1, 2);
		numChannels = std::min(numChannels, static_cast<int>(ACN_to_FuMa_order_0_3.size()));

		for (int i = 0; i < numChannels; i++) {
			soundFieldArray[i] = aux[ACN_to_FuMa_order_0_3[i]];
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

		std::vector<CMonoBuffer<float>> aux = soundFieldArray;
		int numChannels = pow(ambisonicOrder + 1, 2);
		numChannels = std::min(numChannels, static_cast<int>(FuMa_to_ACN_order3.size()));

		for (int i = 0; i < numChannels; i++) {
			soundFieldArray[i] = aux[FuMa_to_ACN_order3[i]];
		}
	}


	// Rotate the field depending on how coordinates have changed
	//To implement
	void RotateField(float yaw, float pitch, float roll);

	void DownMixing(int order) {
		soundFieldArray.resize(pow(order + 1, 2));
		ambisonicOrder = order;
	}

	//TODO
	CAmbisonicSoundField & operator+=(const CAmbisonicSoundField & other) {
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
	bool setupDone;
	int ambisonicOrder;
	int sampleRate;
	int bufferSize;
	std::string ordering;

	std::vector<CMonoBuffer<float>> soundFieldArray;
};

#endif // _CAMBISONIC_SOUND_FIELD_HPP_
