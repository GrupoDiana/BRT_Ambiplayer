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
#include "ConfigurationA.hpp"

#include <cmath>
#include <iostream>
#include <vector>

using namespace Common;

const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;

float normI = 1.0f / std::sqrt(1 + phi * phi);
std::vector<Common::CVector3> IVertices = {
	{ 0, 1 * normI, phi * normI }, { 0, 1 * normI, -phi * normI }, { 0, -1 * normI, phi * normI }, { 0, -1 * normI, -phi * normI },
	{ 1 * normI, phi * normI, 0 }, { 1 * normI, -phi * normI, 0 }, { -1 * normI, phi * normI, 0 }, { -1 * normI, -phi * normI, 0 },
	{ phi * normI, 0, 1 * normI }, { phi * normI, 0, -1 * normI }, { -phi * normI, 0, 1 * normI }, { -phi * normI, 0, -1 * normI }
};

std::vector<std::vector<int>> IFaces = {
	{ 0, 4, 6 }, { 0, 6, 10 }, { 0, 10, 8 }, { 0, 8, 4 },
	{ 1, 6, 4 }, { 1, 11, 6 }, { 1, 9, 11 }, { 1, 4, 9 },
	{ 2, 7, 5 }, { 2, 10, 7 }, { 2, 8, 10 }, { 2, 5, 8 },
	{ 3, 5, 7 }, { 3, 7, 11 }, { 3, 11, 9 }, { 3, 9, 5 },
	{ 4, 8, 9 }, { 5, 9, 8 }, { 6, 11, 10 }, { 7, 10, 11 }
};

struct Edge {
    int vertex1, vertex2;
	std::vector<int> points;
};

struct Face {
	int v0, v1, v2; 
};

float increment = 0.1;
float decrement = 0.01;

enum PlatonicSolid {
	octahedron,
	cube,
	icosahedron,
	dodecahedron
};

class CLoudSpeakerArrayController {
public:
	CLoudSpeakerArrayController(): setupDone(false) { }

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

	void UpdateSpeakerLayout(BRTBase::CBRTManager & brtManager, PlatonicSolid solid, CConfigurationA & ConfigurationA) {
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

		brtManager.BeginSetup();
		
		
		for (size_t i = currentNum; i < newNum; ++i) {
			auto _brtSoundSource = brtManager.CreateSoundSource<BRTSourceModel::CSourceOmnidirectionalModel>("Speaker" + std::to_string(i));
			ConfigurationA.ConnectSoundSource(&brtManager, "Speaker" + std::to_string(i));
			speakers.push_back(_brtSoundSource);
		}
		

		for (size_t i = currentNum; i > newNum; --i) {
			ConfigurationA.DisconnectSoundSource(&brtManager, "Speaker" + std::to_string(i - 1));
			brtManager.RemoveSoundSource("Speaker" + std::to_string(i - 1));
			speakers.pop_back();
		}

		brtManager.EndSetup();

		for (size_t i = 0; i < newNum; ++i) {
			speakers[i]->SetSourceTransform(Common::CTransform(vertices[i]));
		}

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
		CMatrix<float> R(3,3,0.0);
		R = q.RotationMatrix();
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

	std::vector<float> ComputeDirectionalEnergyMapGeodesic(
		const CAmbisonicSoundField & field) {
		std::vector<float> energies(verticesSubdivided.size(), 0.0f);

		const auto & soundField = field.GetSoundFieldArray();
		if (soundField.empty()) return energies;

		size_t order = field.GetOrder();
		size_t numChannels = soundField.size();
		size_t numSamples = field.GetBufferSize();
		float alpha;
		if (numSamples == 0) return energies;

		std::vector<const float *> channels;
		channels.reserve(numChannels);
		for (const auto & ch : soundField)
			channels.push_back(ch.data());

		for (size_t i = 0; i < verticesSubdivided.size(); ++i) {
			const Common::CVector3 & pos = verticesSubdivided[i];
			float sampleEnergy = 0.0f; 

			for (size_t n = 0; n < numSamples; ++n) {
				float sample = 0.0f;

				if (order >= 1 && numChannels >= 4) {
					//const float W = channels[0][n];
					//const float Y = channels[1][n];
					//const float Z = channels[2][n];
					//const float X = channels[3][n];

					sample += channels[0][n] + pos.x * channels[3][n] + pos.y * channels[1][n] + pos.z * channels[2][n];
				}

				if (order >= 2 && numChannels > 8) {
					//const float V = channels[4][n];
					//const float T = channels[5][n];
					//const float R = channels[6][n];
					//const float S = channels[7][n];
					//const float U = channels[8][n];

					sample += sqrtf(3.f) * pos.x * pos.y * channels[4][n] + sqrtf(3.f) * pos.y * pos.z * channels[5][n] + 0.5f * ((3.f * pos.z * pos.z) - 1.f) * channels[6][n] + sqrtf(3.f) * pos.x * pos.z * channels[7][n] + sqrtf(3.f / 4.f) * (pos.x * pos.x - pos.y * pos.y) * channels[8][n];
				}

				if (order >= 3 && numChannels > 15) {
					//const float Q = channels[9][n];
					//const float O = channels[10][n];
					//const float M = channels[11][n];
					//const float K = channels[12][n];
					//const float L = channels[13][n];
					//const float N = channels[14][n];
					//const float P = channels[15][n];

					sample += sqrtf(5.f / 8.f) * pos.y * (3.f * pos.x * pos.x - pos.y * pos.y) * channels[9][n] + sqrtf(15.f) * pos.x * pos.y * pos.z * channels[10][n] + sqrtf(3.f / 8.f) * pos.y * (5.f * pos.z * pos.z - 1.f) * channels[11][n] + 0.5f * pos.z * (5.f * pos.z * pos.z - 3.f) * channels[12][n] + sqrtf(3.f / 8.f) * pos.x * (5.f * pos.z * pos.z - 1.f) * channels[13][n] + sqrtf(15.f / 4.f) * pos.z * (pos.x * pos.x - pos.y * pos.y) * channels[14][n] + sqrtf(5.f / 8.f) * pos.x * (pos.x * pos.x - 3.f * pos.y * pos.y) * channels[15][n];
				}

				float absSample = std::abs(sample);
				
				if (absSample > sampleEnergy) {
					alpha = increment;
				} else {
					alpha = decrement;
				}
				sampleEnergy = (1.0f - alpha) * sampleEnergy + alpha * absSample;
			}
			energies[i] = sampleEnergy;
		}

		return energies;
	}


	std::vector<Edge> GenerateFaces(int k, std::vector<Common::CVector3> & vertices) {
		
	}

	std::vector<Common::CVector3> kDivision(int k) {
		std::vector<Edge> edges = Edges();
		verticesSubdivided.clear();

		// Copiar los vértices originales
		for (const auto & v : IVertices) {
			verticesSubdivided.push_back(v);
		}

		// Subdividir todas las aristas
		for (auto & e : edges) {
			Common::CVector3 V1 = IVertices[e.vertex1];
			Common::CVector3 V2 = IVertices[e.vertex2];

			e.points.clear();

			for (int i = 1; i <= k; i++) {
				float t = float(i) / (k + 1);
				Common::CVector3 P = Slerp(V1, V2, t);
				verticesSubdivided.push_back(P);
				e.points.push_back(verticesSubdivided.size() - 1);
			}
		}

		// Subdividir el interior de cada cara usando los puntos de las aristas
		for (int f = 0; f < IFaces.size(); f++) {
			int i0 = IFaces[f][0];
			int i1 = IFaces[f][1];
			int i2 = IFaces[f][2];

			Common::CVector3 V0 = IVertices[i0];
			Common::CVector3 V1 = IVertices[i1];
			Common::CVector3 V2 = IVertices[i2];

			// Crear filas internas (fila 1..k-1)
			for (int i = 2; i <= k; i++) {
				//float t_corrected = 0.5f - 0.5f * cosf(M_PI * (float(i) / (k + 1)));
				float t_corrected = M_PI * (float(i) / (k + 1));
				Common::CVector3 rowStart = Slerp(V0, V2, t_corrected);
				Common::CVector3 rowEnd = Slerp(V0, V1, t_corrected);

				// Interpolación interna de la fila
				for (int j = 1; j <= i - 1; j++) {
					float t = float(j) / (i);
					Common::CVector3 P = Slerp(rowStart, rowEnd, t);
					verticesSubdivided.push_back(P);
				}
			}
		}

		std::cout << "Num vertices subdivided: " << verticesSubdivided.size() << std::endl;
		return verticesSubdivided;
	}

private:
	bool setupDone;

	using SourceModelPtr = std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel>;
	std::vector<SourceModelPtr> speakers;

	std::vector<Common::CVector3> verticesSubdivided;

	std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel> CreateOmnidirectionalSoundSource(
		BRTBase::CBRTManager & brtManager, const std::string & _soundSourceID) {
		brtManager.BeginSetup();
		auto _brtSoundSource = brtManager.CreateSoundSource<BRTSourceModel::CSourceOmnidirectionalModel>(_soundSourceID);
		brtManager.EndSetup();
		if (!_brtSoundSource) std::cout << "Error creating sound source" << std::endl;
		return _brtSoundSource;
	}

	Common::CVector3 Slerp(const Common::CVector3 & u, const Common::CVector3 & v, float t) {
		float dot = u.x * v.x + u.y * v.y + u.z * v.z;
		dot = std::clamp(dot, -1.0f, 1.0f); 
		float theta = acos(dot);

		if (theta < 1e-6f) {
			return u * (1.0f - t) + v * t;
		}

		float sinTheta = sin(theta);
		float w1 = sin((1.0f - t) * theta) / sinTheta;
		float w2 = sin(t * theta) / sinTheta;

		Common::CVector3 P = u * w1 + v * w2;
		return P;
	}



	std::vector<Edge> Edges() {
		std::vector<Edge> edges;
		//Para cada arista, se divide entre k partes iguales
		for (int i = 0; i < IFaces.size(); i++) {
			for (int j = 0; j < 3; j++) {
				Edge e;
				e.vertex1 = IFaces[i][j];
				e.vertex2 = IFaces[i][(j + 1) % 3];
				//Comprobar si la arista ya existe
				bool exists = false;
				for (const auto & existingEdge : edges) {
					if (edgesEqual(e, existingEdge)) {
						exists = true;
						break;
					}
				}
				if (!exists) {
					edges.push_back(e);
				}
			}
		}
		return edges;
	}

	// Returns true if the two edges have the same vertices, regardless of order
	bool edgesEqual(const Edge & e1, const Edge & e2) {
		return (e1.vertex1 == e2.vertex1 && e1.vertex2 == e2.vertex2) || (e1.vertex1 == e2.vertex2 && e1.vertex2 == e2.vertex1);
	}
};


#endif // _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_
