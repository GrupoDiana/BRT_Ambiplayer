/**
 * \class CLoudSpeakerArrayController
 *
 * \brief Controls the configuration and spatial arrangement of loudspeakers in a 3D array.
 *
 * \details This class is responsible for creating and managing a set of omnidirectional
 * sound sources (loudspeakers) placed at the vertices of Platonic solids.
 * It enables consistent loudspeaker placement for Ambisonic decoding and
 * allows rotation of the entire loudspeaker array using quaternion-based transformations.
 *
 * The class supports multiple Platonic solid configurations:
 * - Octahedron (6 speakers)
 * - Cube (8 speakers)
 * - Icosahedron (12 speakers)
 * - Dodecahedron (20 speakers)
 *
 * Each loudspeaker is created as a `CSourceOmnidirectionalModel` through the
 * BRT framework (`ofxBRTLibrary`) and positioned in 3D space according to
 * the selected geometry.
 *
 * \date November 2025
 *
 * \author R. Liceras-Ramírez  
 * \b Tutor: A. Reyes-Lecuona (University of Málaga)  
 * \b Institution: Universidad de Málaga
 *
 * \b Contact: areyes@uma.es
 *
 * \b Copyright:
 * © 2025 University of Málaga. All rights reserved.
 *
 * \b Context:
 * Developed as part of the Bachelor's Thesis (TFG) of R. Liceras-Ramírez
 * under the supervision of Prof. A. Reyes-Lecuona at the University of Málaga.
 * This component belongs to the CAMBISONIC framework for Ambisonic processing and decoding.
 *
 * \b Remarks:
 * - Integrates with the `BRTBase::CBRTManager` and `BRTSourceModel` libraries.
 * - Supports quaternion-based rotation of the loudspeaker array.
 * - Provides easy access to individual speakers and their transforms.
 * - Geometric arrangements are normalized for unit sphere representation.
 *
 * \b Licence:
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 */


#ifndef _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_
#define _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_

#include "AmbisonicSoundField.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"

#include <cmath>
#include <iostream>
#include <vector>

const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;

enum PlatonicSolid {
	octahedron,
	cube,
	icosahedron,
	dodecahedron
};

class CLoudSpeakerArrayController {
public:
	CLoudSpeakerArrayController() { }

	void SetUp(BRTBase::CBRTManager & brtManager, PlatonicSolid solid) {
		std::vector<Common::CVector3> vertices;

		switch (solid) {
		case PlatonicSolid::octahedron:
			vertices = {
				{ 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 }
			};
			break;

		case PlatonicSolid::cube: {
			float norm = 1.0f / std::sqrt(3.0f);
			vertices = {
				{ 1 * norm, 1 * norm, 1 * norm }, { -1 * norm, -1 * norm, 1 * norm }, { -1 * norm, 1 * norm, -1 * norm }, { 1 * norm, -1 * norm, -1 * norm },
				{ -1 * norm, 1 * norm, 1 * norm }, { 1 * norm, -1 * norm, 1 * norm }, { 1 * norm, 1 * norm, -1 * norm }, { -1 * norm, -1 * norm, -1 * norm }
			};
			break;
		}

		case PlatonicSolid::icosahedron: {
			float norm = 1.0f / std::sqrt(1 + phi * phi);
			vertices = {
				{ 0, 1 * norm, phi * norm }, { 0, 1 * norm, -phi * norm }, { 0, -1 * norm, phi * norm }, { 0, -1 * norm, -phi * norm },
				{ 1 * norm, phi * norm, 0 }, { 1 * norm, -phi * norm, 0 }, { -1 * norm, phi * norm, 0 }, { -1 * norm, -phi * norm, 0 },
				{ phi * norm, 0, 1 * norm }, { phi * norm, 0, -1 * norm }, { -phi * norm, 0, 1 * norm }, { -phi * norm, 0, -1 * norm }
			};
			break;
		}

		case PlatonicSolid::dodecahedron: {
			float norm = 1.0f / std::sqrt(3.0f);
			float normPhi = 1.0f / std::sqrt(1 + phi * phi);
			vertices = {
				{ 1 * norm, 1 * norm, 1 * norm }, { -1 * norm, 1 * norm, 1 * norm },
				{ 1 * norm, -1 * norm, 1 * norm }, { -1 * norm, -1 * norm, 1 * norm },
				{ 1 * norm, 1 * norm, -1 * norm }, { -1 * norm, 1 * norm, -1 * norm },
				{ 1 * norm, -1 * norm, -1 * norm }, { -1 * norm, -1 * norm, -1 * norm },

				{ 0, 1 * normPhi / phi, phi * normPhi }, { 0, -1 * normPhi / phi, phi * normPhi },
				{ 0, 1 * normPhi / phi, -phi * normPhi }, { 0, -1 * normPhi / phi, -phi * normPhi },

				{ 1 * normPhi / phi, phi * normPhi, 0 }, { -1 * normPhi / phi, phi * normPhi, 0 },
				{ 1 * normPhi / phi, -phi * normPhi, 0 }, { -1 * normPhi / phi, -phi * normPhi, 0 },

				{ phi * normPhi, 0, 1 * normPhi / phi }, { -phi * normPhi, 0, 1 * normPhi / phi },
				{ phi * normPhi, 0, -1 * normPhi / phi }, { -phi * normPhi, 0, -1 * normPhi / phi }
			};
			break;
		}
		}

		speakers.resize(vertices.size());
		for (size_t i = 0; i < vertices.size(); ++i) {
			speakers[i] = CreateOmnidirectionalSoundSource(brtManager, "Speaker" + std::to_string(i));
			speakers[i]->SetSourceTransform(Common::CTransform(vertices[i]));
		}

		setupDone = true;
	}

	void UpdateSpeakerLayout(BRTBase::CBRTManager & brtManager, PlatonicSolid solid) {
		// --- Generar los vértices del nuevo sólido ---
		std::vector<Common::CVector3> vertices;
		switch (solid) {
		case PlatonicSolid::octahedron:
			vertices = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
			break;
		case PlatonicSolid::cube: {
			float norm = 1.0f / std::sqrt(3.0f);
			vertices = { { norm, norm, norm }, { -norm, -norm, norm }, { -norm, norm, -norm }, { norm, -norm, -norm },
				{ -norm, norm, norm }, { norm, -norm, norm }, { norm, norm, -norm }, { -norm, -norm, -norm } };
			break;
		}
		case PlatonicSolid::icosahedron: {
			float norm = 1.0f / std::sqrt(1 + phi * phi);
			vertices = { { 0, norm, phi * norm }, { 0, norm, -phi * norm }, { 0, -norm, phi * norm }, { 0, -norm, -phi * norm },
				{ norm * 1, phi * norm, 0 }, { norm * 1, -phi * norm, 0 }, { -norm * 1, phi * norm, 0 }, { -norm * 1, -phi * norm, 0 },
				{ phi * norm, 0, norm }, { phi * norm, 0, -norm }, { -phi * norm, 0, norm }, { -phi * norm, 0, -norm } };
			break;
		}
		case PlatonicSolid::dodecahedron: {
			float norm = 1.0f / std::sqrt(3.0f);
			float normPhi = 1.0f / std::sqrt(1 + phi * phi);
			vertices = { { norm, norm, norm }, { -norm, norm, norm }, { norm, -norm, norm }, { -norm, -norm, norm },
				{ norm, norm, -norm }, { -norm, norm, -norm }, { norm, -norm, -norm }, { -norm, -norm, -norm },
				{ 0, normPhi / phi, phi * normPhi }, { 0, -normPhi / phi, phi * normPhi }, { 0, normPhi / phi, -phi * normPhi }, { 0, -normPhi / phi, -phi * normPhi },
				{ normPhi / phi, phi * normPhi, 0 }, { -normPhi / phi, phi * normPhi, 0 }, { normPhi / phi, -phi * normPhi, 0 }, { -normPhi / phi, -phi * normPhi, 0 },
				{ phi * normPhi, 0, normPhi / phi }, { -phi * normPhi, 0, normPhi / phi }, { phi * normPhi, 0, -normPhi / phi }, { -phi * normPhi, 0, -normPhi / phi } };
			break;
		}
		}

		size_t newNum = vertices.size();
		size_t currentNum = speakers.size();

		for (size_t i = currentNum; i < newNum; ++i) {
			auto sp = CreateOmnidirectionalSoundSource(brtManager, "Speaker" + std::to_string(i));
			speakers.push_back(sp);
		}

		for (size_t i = currentNum; i > newNum; --i) {
			brtManager.RemoveSoundSource("Speaker" + std::to_string(i - 1));
			speakers.pop_back();
		}

		for (size_t i = 0; i < newNum; ++i) {
			speakers[i]->SetSourceTransform(Common::CTransform(vertices[i]));
		}

		setupDone = true;
	}


	std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel> GetSpeaker(size_t index) const {
		if (!setupDone) return nullptr;
		if (index >= speakers.size()) throw std::out_of_range("Speaker index out of range");
		return speakers[index];
	}

	const std::vector<std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel>> & GetSpeakers() const {
		if (!setupDone) return std::vector<std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel>>();
		return speakers;
	}

	int GetNumSpeakers() const {
		if (!setupDone) return -1;
		return speakers.size();
	}

	void RotateLoudspeakerArray(Common::CQuaternion & q) {
		if (!setupDone) return;
		CMatrix<double> R(3,3,0.0);
		R = q.R_1();
		for (auto & speaker : speakers) {
			Common::CVector3 pos = speaker->GetSourceTransform().GetPosition();
			Common::CVector3 rotatedPos = R * pos;
			speaker->SetSourceTransform(Common::CTransform(rotatedPos));
		}
	}

std::vector<float> ComputeDirectionalEnergyMap(
		const CAmbisonicSoundField & field,
		int numAz,
		int numEl) {
		std::vector<float> energies(numAz * numEl, 0.0f);

		const auto & soundField = field.GetSoundFieldArray();
		if (soundField.empty()) return energies;

		size_t order = field.GetOrder();
		size_t numChannels = soundField.size();
		size_t numSamples = field.GetBufferSize();
		if (numSamples == 0) return energies;

		std::vector<const float *> channels;
		channels.reserve(numChannels);
		for (const auto & ch : soundField)
			channels.push_back(ch.data());

		for (int ia = 0; ia < numAz; ++ia) {
			float az = (2.0f * M_PI * ia) / numAz;
			for (int ie = 0; ie < numEl; ++ie) {
				float el = ((float)ie / (float)(numEl - 1)) * M_PI; 
				float sinEl = std::sin(el);
				float cosEl = std::cos(el);

				Common::CVector3 pos(
					sinEl * std::cos(az), // x
					sinEl * std::sin(az), // y
					cosEl // z
				);


				double energySum = 0.0;
				for (size_t n = 0; n < numSamples; ++n) {
					float sample = 0.0f;

					if (order >= 1 && numChannels >= 4) {
						const float W = channels[0][n];
						const float Y = channels[1][n];
						const float Z = channels[2][n];
						const float X = channels[3][n];

						sample += W + pos.x * X + pos.y * Y + pos.z * Z;
					}

					if (order >= 2 && numChannels > 8) {
						const float V = channels[4][n];
						const float T = channels[5][n];
						const float R = channels[6][n];
						const float S = channels[7][n];
						const float U = channels[8][n];

						sample += sqrtf(3.f) * pos.x * pos.y * V + sqrtf(3.f) * pos.y * pos.z * T + 0.5f * ((3.f * pos.z * pos.z) - 1.f) * R + sqrtf(3.f) * pos.x * pos.z * S + sqrtf(3.f / 4.f) * (pos.x * pos.x - pos.y * pos.y) * U;
					}

					if (order >= 3 && numChannels > 15) {
						const float Q = channels[9][n];
						const float O = channels[10][n];
						const float M = channels[11][n];
						const float K = channels[12][n];
						const float L = channels[13][n];
						const float N = channels[14][n];
						const float P = channels[15][n];

						sample += sqrtf(5.f / 8.f) * pos.y * (3.f * pos.x * pos.x - pos.y * pos.y) * Q + sqrtf(15.f) * pos.x * pos.y * pos.z * O + sqrtf(3.f / 8.f) * pos.y * (5.f * pos.z * pos.z - 1.f) * M + 0.5f * pos.z * (5.f * pos.z * pos.z - 3.f) * K + sqrtf(3.f / 8.f) * pos.x * (5.f * pos.z * pos.z - 1.f) * L + sqrtf(15.f / 4.f) * pos.z * (pos.x * pos.x - pos.y * pos.y) * N + sqrtf(5.f / 8.f) * pos.x * (pos.x * pos.x - 3.f * pos.y * pos.y) * P;
					}

					energySum += sample * sample;
				}

				float energyAvg = static_cast<float>(energySum / (double)numSamples);
				energies[ia * numEl + ie] = energyAvg;
			}
		}

		return energies;
	}


private:
	bool setupDone;

	using SourceModelPtr = std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel>;
	std::vector<SourceModelPtr> speakers;

	std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel> CreateOmnidirectionalSoundSource(
		BRTBase::CBRTManager & brtManager, const std::string & _soundSourceID) {
		brtManager.BeginSetup();
		auto _brtSoundSource = brtManager.CreateSoundSource<BRTSourceModel::CSourceOmnidirectionalModel>(_soundSourceID);
		brtManager.EndSetup();
		if (!_brtSoundSource) std::cout << "Error creating sound source" << std::endl;
		return _brtSoundSource;
	}
};


#endif // _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_
