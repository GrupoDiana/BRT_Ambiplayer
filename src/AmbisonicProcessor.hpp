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
	void FOARotateField(float yaw, float pitch, float roll,CAmbisonicSoundField & OutBuffer) {
		float A[3][3];
		A[0][0] = cos(yaw) * cos(pitch);
		A[0][1] = -sin(yaw) * cos(pitch);
		A[0][2] = sin(pitch);

		A[1][0] = sin(roll) * sin(pitch) * cos(yaw) + cos(roll) * sin(yaw);
		A[1][1] = -sin(roll) * sin(pitch) * sin(yaw) + cos(roll) * cos(yaw);
		A[1][2] = -sin(roll) * cos(pitch);

		A[2][0] = -cos(roll) * sin(pitch) * cos(yaw) + sin(roll) * sin(yaw);
		A[2][1] = cos(roll) * sin(pitch) * sin(yaw) + sin(roll) * cos(yaw);
		A[2][2] = cos(roll) * cos(pitch);
		                                                                                                                                                                                          
		int N = OutBuffer.GetBufferSize();
		int numChannels = OutBuffer.GetSoundFieldArray().size();
		vector<CMonoBuffer<float>> buffer(numChannels, CMonoBuffer<float>(N, 0.0f));
		buffer[0] = OutBuffer.GetSoundFieldArray()[0];

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				for (int k = 0; k < OutBuffer.GetBufferSize(); k++) {
					buffer[i + 1][k] += A[i][j] * OutBuffer.GetSoundFieldArray()[j + 1][k];
				}
			}
		}

		for (int i = 4; i < OutBuffer.GetSoundFieldArray().size()-1; i++) {
			buffer[i] = OutBuffer.GetSoundFieldArray()[i];
		}
		OutBuffer.GetSoundFieldArray() = buffer;
	}

	void FOADecodeToSpeakerArray(CLoudSpeakerArrayController & speakerArray,
		const CAmbisonicSoundField & OutBuffer, std::string decoding) {
	
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

		float kWgain;
		float gainXYZ;

		if (decoding == "basic") {
			kWgain = std::sqrt(2) / speakers.size();
			gainXYZ = 3.0f / speakers.size();
		} else if (decoding == "maxrE") {
			kWgain = 1.0f / std::sqrt(speakers.size());
			gainXYZ = std::sqrt(3.0f/2.0f) / std::sqrt(speakers.size());
		} 
		 
		for (size_t i = 0; i < speakers.size(); ++i) {
			Common::CVector3 pos = speakers[i]->GetSourceTransform().GetPosition();

			CMonoBuffer<float> buffer;
			buffer.resize(numSamples); 

			for (size_t n = 0; n < numSamples; ++n) {
				buffer[n] = kWgain * W[n] + gainXYZ*(pos.x * X[n] + pos.y * Y[n] + pos.z * Z[n]);
			}

			speakers[i]->SetBuffer(buffer);
		}
	}

	void SADecodeToSpeakerArray(CLoudSpeakerArrayController & speakerArray,
		const CAmbisonicSoundField & OutBuffer, std::string decoding) {

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

		float Gain=std::sqrt(4*M_PI/speakers.size());

		for (size_t i = 0; i < speakers.size(); ++i) {
			Common::CVector3 pos = speakers[i]->GetSourceTransform().GetPosition();

			CMonoBuffer<float> buffer;
			buffer.resize(numSamples);

			for (size_t n = 0; n < numSamples; ++n) {
				buffer[n] = Gain * (W[n] + pos.x * X[n] + pos.y * Y[n] + pos.z * Z[n]);
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

private:
	/* size_t ambisonicOrder;
	size_t sampleRate;
	int blockSize;
	int cont = 0;*/

	CAmbisonicSoundField ambisonicProcessor;

};

#endif // _CAMBISONIC_PROCESSOR_HPP_
