#ifndef _CAMBISONIC_PROCESSOR_HPP_
#define _CAMBISONIC_PROCESSOR_HPP_

#include "AmbisonicSoundField.hpp"
#include "LoudSpeakerArrayController.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"

#include <Common/Buffer.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

class CAmbisonicProcessor {
public:
	CAmbisonicProcessor(): CAmbisonicProcessor(4, 48000, 512) {}

	CAmbisonicProcessor(size_t numChannels_, unsigned int sampleRate_, size_t blockSize_)
		: numChannels(numChannels_)
		, sampleRate(sampleRate_)
		, blockSize(blockSize_) {
		if (numChannels <= 0) {
			throw std::invalid_argument("numChannels must be > 0");
		}

		ambisonicOrder = static_cast<size_t>(std::sqrt(numChannels) - 1);
	}


	// Rotate the field depending on listener rotation
	void RotateField(float yaw, float pitch, float roll) {
		// TODO: implement ambisonic rotation
	}

void DecodeToSpeakerArray(CLoudSpeakerArrayController & speakerArray,
		const CAmbisonicSoundField & OutBuffer) {
	
		const auto & field = OutBuffer.GetSoundFieldArray();
		const CMonoBuffer<float> & Wbuf = field[0];
		const CMonoBuffer<float> & Xbuf = field[3];
		const CMonoBuffer<float> & Ybuf = field[1];
		const CMonoBuffer<float> & Zbuf = field[2];

		const float * W = Wbuf.data();
		const float * X = Xbuf.data();
		const float * Y = Ybuf.data();
		const float * Z = Zbuf.data();

		size_t numSamples = OutBuffer.GetBufferSize();
		auto & speakers = speakerArray.GetSpeakers();

		const float kWgain = 1.0f/ std::sqrt(2.0f);
		float gainZ = 0.0f; // aumentar su contribución
		float gainXY = 1.0f; // reducir lateral

		for (size_t i = 0; i < speakers.size(); ++i) {
			Common::CVector3 pos = speakers[i]->GetSourceTransform().GetPosition();

			float norm = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
			if (norm > 0.0f) {
				pos.x /= norm;
				pos.y /= norm;
				pos.z /= norm;
			}

			CMonoBuffer<float> buffer;
			buffer.resize(numSamples); 

			for (size_t n = 0; n < numSamples; ++n) {
				buffer[n] = kWgain * W[n] + gainXY*(pos.x * X[n] + pos.y * Y[n]) + gainZ*pos.z * Z[n];
			}

			speakers[i]->SetBuffer(buffer);
		}
	}

	// Accessor for the internal ambisonic sound field
	CAmbisonicSoundField & GetSoundFieldProcessor() { return ambisonicProcessor; }

	const CAmbisonicSoundField & GetSoundFieldProcessor() const { return ambisonicProcessor; }

	// Setter para el ambisonicProcessor
	void SetSoundFieldProcessor(const CAmbisonicSoundField & sf) {
		ambisonicProcessor = sf;
	}

	// Sum ambisonic fields
	CAmbisonicProcessor & operator+=(const CAmbisonicProcessor & other) {
		if (numChannels != other.numChannels || blockSize != other.blockSize) {
			throw std::runtime_error("Cannot sum ambisonic blocks with different dimensions");
		}

		auto & arr = ambisonicProcessor.GetSoundFieldArray();
		const auto & otherArray = other.ambisonicProcessor.GetSoundFieldArray();

		for (size_t ch = 0; ch < numChannels; ++ch) {
			arr[ch] += otherArray[ch];
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
	int cont = 0;

	CAmbisonicSoundField ambisonicProcessor;

};

#endif // _CAMBISONIC_PROCESSOR_HPP_
