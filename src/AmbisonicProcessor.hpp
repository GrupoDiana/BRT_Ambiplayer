/**
 * \class CAmbisonicProcessor
 *
 * \brief Handles Ambisonic field transformations and decoding into loudspeaker arrays.
 * 
 * \details This class performs high-level Ambisonic processing tasks such as:
 *  - Applying quaternion-based rotations to the Ambisonic sound field.
 *  - Decoding Ambisonic signals into loudspeaker arrays using FuMa channel ordering.
 *  - Managing the internal Ambisonic sound field buffer for processing.
 * 
 * It supports decoding up to **third-order Ambisonics** and includes
 * basic decoding.  
 * The decoding process considers loudspeaker positions to spatialize the field.
 * 
 * \note The implementation assumes **ACN ordering** for Ambisonic channels
 * (W, Y, Z, X, V, T, R, S, U, ...), ensuring compatibility with the rest of
 * the CAMBISONIC framework.
 * 
 * \date November 2025
 *
 * \author R. Liceras-Ramírez  
 * \b Tutor: A. Reyes-Lecuona and D. Gonzalez-Toledo  
 * \b Institution: Universidad de Málaga
 *
 * \b Contact: areyes@uma.es
 *
 * \b Copyright:
 * © 2025 University of Málaga. All rights reserved.
 * 
 * \b Remarks:
 * - All Ambisonic operations follow ACN channel ordering.
 * - Rotation uses quaternion-based orientation transformations.
 * - Supported decoding types: `"basic"`.
 * - Designed for integration with `CAmbisonicSoundField` and `CLoudspeakerArrayController`.
 * 
 * \b Licence:
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 */


#ifndef _CAMBISONIC_PROCESSOR_HPP_
#define _CAMBISONIC_PROCESSOR_HPP_

#include "AmbisonicSoundField.hpp"
#include "LoudSpeakerArrayController.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"
#include <CommonDefinitions.hpp>

#include <Common/Buffer.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>


using namespace CommonDefinitions;


class CAmbisonicProcessor {
public:
	CAmbisonicProcessor(){}

	// Rotate the field depending on listener rotation
	void CalculateRotationMatrices(Common::CQuaternion q, CAmbisonicSoundField & OutBuffer) {
		OutBuffer.CalculateRotationMatrices(q);	
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

				if (order >= 1 && channels.size() >= 4) {
					//const float W = channels[0][n];
					//const float Y = channels[1][n];
					//const float Z = channels[2][n];
					//const float X = channels[3][n];

					sample += channels[0][n] + pos.x * channels[3][n] + pos.y * channels[1][n] + pos.z * channels[2][n];
				}

				if (order >= 2 && channels.size() > 8) {
					//const float V = channels[4][n];
					//const float T = channels[5][n];
					//const float R = channels[6][n];
					//const float S = channels[7][n];
					//const float U = channels[8][n];

					sample += SQRT_3 * pos.x * pos.y * channels[4][n] + SQRT_3 * pos.y * pos.z * channels[5][n] + 0.5f * ((3.f * pos.z * pos.z) - 1.f) * channels[6][n] + SQRT_3 * pos.x * pos.z * channels[7][n] + SQRT_3_DIV_4 * (pos.x * pos.x - pos.y * pos.y) * channels[8][n];
				}

				if (order >= 3 && channels.size() > 15) {
					//const float Q = channels[9][n];
					//const float O = channels[10][n];
					//const float M = channels[11][n];
					//const float K = channels[12][n];
					//const float L = channels[13][n];
					//const float N = channels[14][n];
					//const float P = channels[15][n];

					sample += SQRT_5_DIV_8 * pos.y * (3.f * pos.x * pos.x - pos.y * pos.y) * channels[9][n] + SQRT_15 * pos.x * pos.y * pos.z * channels[10][n] + SQRT_3_DIV_8 * pos.y * (5.f * pos.z * pos.z - 1.f) * channels[11][n] + 0.5f * pos.z * (5.f * pos.z * pos.z - 3.f) * channels[12][n] + SQRT_3_DIV_8 * pos.x * (5.f * pos.z * pos.z - 1.f) * channels[13][n] + SQRT_15_DIV_4 * pos.z * (pos.x * pos.x - pos.y * pos.y) * channels[14][n] + SQRT_5_DIV_8 * pos.x * (pos.x * pos.x - 3.f * pos.y * pos.y) * channels[15][n];
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
