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
	CAmbisonicProcessor(){}

	// Rotate the field depending on listener rotation
	void RotateField(Common::CQuaternion q,CAmbisonicSoundField & OutBuffer) {
		OutBuffer.RotateField(q);
	}

	void DecodeToSpeakerArray(CLoudSpeakerArrayController & speakerArray,const CAmbisonicSoundField & OutBuffer, std::string decoding) {
		const auto & field = OutBuffer.GetSoundFieldArray();
		size_t order = OutBuffer.GetOrder();
		if (field.empty()) return;

		size_t numSamples = OutBuffer.GetBufferSize();
		if (numSamples == 0) return;

		float Wgain = 0.0f, XYZgain = 0.0f, VTRSUgain = 0.0f;

		if (decoding == "basic") {
			Wgain = 1 / std::sqrt(speakerArray.GetNumSpeakers());
			XYZgain = 0.932f / std::sqrt(speakerArray.GetNumSpeakers());
			VTRSUgain = 0.8029f / std::sqrt(speakerArray.GetNumSpeakers());
		} else if (decoding == "maxrE") {
			Wgain = 1.0f / std::sqrt(speakerArray.GetNumSpeakers());
			XYZgain = std::sqrt(3.0f / 2.0f) / std::sqrt(speakerArray.GetNumSpeakers());
		}

		std::vector<const float *> channels;
		channels.reserve(field.size());
		for (const auto & f : field)
			channels.push_back(f.data());

		for (size_t i = 0; i < speakerArray.GetNumSpeakers(); ++i) {
			Common::CVector3 pos = speakerArray.GetSpeaker(i)->GetSourceTransform().GetPosition();

			CMonoBuffer<float> buffer;
			buffer.resize(numSamples, 0.0f);

			for (size_t n = 0; n < numSamples; ++n) {
				float sample = 0.0f;

				if (order >= 1) {
					const float W = channels[0][n];
					const float X = channels[1][n];
					const float Y = channels[2][n];
					const float Z = channels[3][n];

					sample += Wgain * W + XYZgain * (pos.x * X + pos.y * Y + pos.z * Z);
				}

				if (order >= 2 && channels.size() > 8) {
					const float R = channels[4][n];
					const float S = channels[5][n];
					const float T = channels[6][n];
					const float U = channels[7][n];
					const float V = channels[8][n];

					sample += VTRSUgain * (sqrtf(3.f) * pos.x * pos.y * V + sqrtf(3.f) * pos.y * pos.z * T + 0.5f * ((3.f * pos.z * pos.z) - 1.f) * R + sqrtf(3.f) * pos.x * pos.z * S + sqrtf(3.f / 4.f) * (pos.x * pos.x - pos.y * pos.y) * U);
				}

				if (order >= 3 && channels.size() > 15) {
					const float K = channels[9][n];
					const float L = channels[10][n];
					const float M = channels[11][n];
					const float N = channels[12][n];
					const float O = channels[13][n];
					const float P = channels[14][n];
					const float Q = channels[15][n];

					sample += XYZgain * (sqrtf(5.f / 8.f) * pos.y * (3.f * pos.x * pos.x - pos.y * pos.y) * Q + sqrtf(15.f) * pos.x * pos.y * pos.z * O + sqrtf(3.f / 8.f) * pos.y * (5.f * pos.z * pos.z - 1.f) * M + 0.5f * pos.z * (5.f * pos.z * pos.z - 3.f) * K + sqrtf(3.f / 8.f) * pos.x * (5.f * pos.z * pos.z - 1.f) * L + sqrtf(15.f / 4.f) * pos.z * (pos.x * pos.x - pos.y * pos.y) * N + sqrtf(5.f / 8.f) * pos.x * (pos.x * pos.x - 3.f * pos.y * pos.y) * P);
				}
				buffer[n] = sample;
			}
			speakerArray.GetSpeaker(i)->SetBuffer(buffer);
		}
	}

	// Accessor for the internal ambisonic sound field
	CAmbisonicSoundField & GetSoundFieldProcessor() { return ambisonicProcessor; }

	const CAmbisonicSoundField & GetSoundFieldProcessor() const { return ambisonicProcessor; }

	// Setter para el ambisonicProcessor
	void SetSoundFieldProcessor(const CAmbisonicSoundField & sf) {
		ambisonicProcessor = sf;
	}

private:

	CAmbisonicSoundField ambisonicProcessor;

};

#endif // _CAMBISONIC_PROCESSOR_HPP_
