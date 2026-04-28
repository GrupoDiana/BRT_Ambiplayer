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
 * \date November 2025
 *
 * \author R. Liceras-Ramírez under the supervision of A. Reyes-Lecuona and D. Gonzalez-Toledo (3DI-DIANA Research Group)
 * \b Institution: Universidad de Málaga
 *
 * \b Context:
 * Developed as part of the Bachelor's Thesis (TFG), This component belongs to the CAMBISONIC framework
 * for Ambisonic processing and decoding.
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


#ifndef _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_
#define _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_

#include "AmbisonicSoundField.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"
#include "ConfigurationA.hpp"
#include "CommonDefinitions.hpp"

#include <cmath>
#include <iostream>
#include <vector>

using namespace Common;
using namespace CommonDefinitions;

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
float decrement = 0.001;

enum PlatonicSolid {
	octahedron,
	cube,
	icosahedron,
	dodecahedron
};

namespace Audio {
enum class ProjectionPlane {
	XY,
	ZX,
	ZY
};
}


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

	std::vector<float> ComputeDirectionalEnergyMap(const CAmbisonicSoundField & field, std::vector<Common::CVector3> points) {
		std::vector<float> energies(points.size(), 0.0f);

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

		for (size_t i = 0; i < points.size(); ++i) {
			const Common::CVector3 & pos = points[i];
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

					sample += SQRT_3 * pos.x * pos.y * channels[4][n] + SQRT_3 * pos.y * pos.z * channels[5][n] + 0.5f * ((3.f * pos.z * pos.z) - 1.f) * channels[6][n] + SQRT_3 * pos.x * pos.z * channels[7][n] + SQRT_3_DIV_4 * (pos.x * pos.x - pos.y * pos.y) * channels[8][n];
				}

				if (order >= 3 && numChannels > 15) {
					//const float Q = channels[9][n];
					//const float O = channels[10][n];
					//const float M = channels[11][n];
					//const float K = channels[12][n];
					//const float L = channels[13][n];
					//const float N = channels[14][n];
					//const float P = channels[15][n];

					sample += SQRT_5_DIV_8 * pos.y * (3.f * pos.x * pos.x - pos.y * pos.y) * channels[9][n] + SQRT_15 * pos.x * pos.y * pos.z * channels[10][n] + SQRT_3_DIV_8 * pos.y * (5.f * pos.z * pos.z - 1.f) * channels[11][n] + 0.5f * pos.z * (5.f * pos.z * pos.z - 3.f) * channels[12][n] + SQRT_3_DIV_8 * pos.x * (5.f * pos.z * pos.z - 1.f) * channels[13][n] + SQRT_15_DIV_4 * pos.z * (pos.x * pos.x - pos.y * pos.y) * channels[14][n] + SQRT_5_DIV_8 * pos.x * (pos.x * pos.x - 3.f * pos.y * pos.y) * channels[15][n];
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

	std::vector<Common::CVector3> GeneratePlanarDirections(Audio::ProjectionPlane plane, int numPoints) {
		std::vector<Common::CVector3> dirs;
		dirs.reserve(numPoints);

		for (int i = 0; i < numPoints; ++i) {
			float theta = 2.f * M_PI * i / numPoints;

			Common::CVector3 d;

			switch (plane) {
			case Audio::ProjectionPlane::XY:
				d = { cosf(theta), sinf(theta), 0.f };
				break;

			case Audio::ProjectionPlane::ZX :
				d = { cosf(theta), 0.f, sinf(theta) };
				break;

			case Audio::ProjectionPlane::ZY :
				d = { 0.f, cosf(theta), sinf(theta) };
				break;
			}

			d.Normalize();
			dirs.push_back(d);
		}

		return dirs;
	}

	void baricentricInterpolationSlerp(int k,
		const std::vector<float> & energies,
		std::vector<Common::CVector3> & outVertices,
		std::vector<float> & outEnergies) {

		outVertices.clear();
		outEnergies.clear();

		float a = 0.0f, b = 0.0f, c = 0.0f;
		int i0, i1, i2;

		for (int faceIndex = 0; faceIndex < facesSubdivided.size(); ++faceIndex) {
			i0 = facesSubdivided[faceIndex].v0;
			i1 = facesSubdivided[faceIndex].v1;
			i2 = facesSubdivided[faceIndex].v2;

			Common::CVector3 V0 = IVertices[i0];
			Common::CVector3 V1 = IVertices[i1];
			Common::CVector3 V2 = IVertices[i2];

			float E0 = energies[i0];
			float E1 = energies[i1];
			float E2 = energies[i2];

			for (int i = 0; i <= k; ++i) {
				for (int j = 0; j <= k - i; ++j) {
					a = float(i) / k;
					b = float(j) / k;
					c = 1.0f - a - b;

					Common::CVector3 rowStart = Slerp(V0, V2, a);
					Common::CVector3 rowEnd = Slerp(V0, V1, a);
					Common::CVector3 P = Slerp(rowStart, rowEnd, b / (1.0f - a + 1e-6f));

					float E = a * E0 + b * E1 + c * E2;

					outVertices.push_back(P);
					outEnergies.push_back(E);
				}
			}
		}

		std::cout << "Generated vertices: " << outVertices.size() << std::endl;
	}

	std::vector<Common::CVector3> kDivision(int k) {

		std::vector<Edge> edges = Edges();
		verticesSubdivided.clear();
		facesSubdivided.clear();

		for (const auto& v : IVertices)
			verticesSubdivided.push_back(v);

		// --- Edge points ---
		for (auto& e : edges) {
			Common::CVector3 V1 = IVertices[e.vertex1];
			Common::CVector3 V2 = IVertices[e.vertex2];

			e.points.clear();

			for (int i = 1; i <= k; ++i) {
				float t = float(i) / (k + 1);
				Common::CVector3 P = Slerp(V1, V2, t);
				verticesSubdivided.push_back(P);
				e.points.push_back(verticesSubdivided.size() - 1);
			}
		}

		// --- Internal points ---
		for (const auto& face : IFaces) {

			int i0 = face[0];
			int i1 = face[1];
			int i2 = face[2];

			Edge * e01 = findEdge(i0, i1, edges);
			Edge * e02 = findEdge(i0, i2, edges);
			Edge * e21 = findEdge(i2, i1, edges);

			std::vector<std::vector<int>> grid(k + 2);
			for (int i = 0; i <= k + 1; ++i)
				grid[i].resize(i + 1);

			grid[0][0] = i0;
			grid[k + 1][0] = i2;
			grid[k + 1][k + 1] = i1;

			for (int i = 1; i <= k; ++i) {
				grid[i][0] = e02->points[i - 1];        // V0–V2
				grid[i][i] = e01->points[i - 1];        // V0–V1
				grid[k + 1][i] = e21->points[i - 1];    // V2–V1
			}

			for (int i = 2; i <= k; ++i) {
				Common::CVector3 rowStart = Slerp(IVertices[i0], IVertices[i2], float(i) / (k + 1));
				Common::CVector3 rowEnd   = Slerp(IVertices[i0], IVertices[i1], float(i) / (k + 1));

				for (int j = 1; j < i; ++j) {
					float t = float(j) / i;
					Common::CVector3 P = Slerp(rowStart, rowEnd, t);
					verticesSubdivided.push_back(P);
					grid[i][j] = verticesSubdivided.size() - 1;
				}
			}

			for (int i = 0; i < k + 1; ++i) {
				for (int j = 0; j <= i; ++j) {

					int a = grid[i][j];
					int b = grid[i + 1][j];
					int c = grid[i + 1][j + 1];

					facesSubdivided.push_back({ a, b, c });

					if (j < i) {
						int d = grid[i][j + 1];
						facesSubdivided.push_back({ a, c, d });
					}
				}
			}
		}

		std::cout << "Vertices: " << verticesSubdivided.size() << std::endl;

		return verticesSubdivided;
	}

	const std::vector<Common::CVector3> & GetVerticesSubdivided() const {
		return verticesSubdivided;
	}


private:
	bool setupDone;

	using SourceModelPtr = std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel>;
	std::vector<SourceModelPtr> speakers;

	std::vector<Common::CVector3> verticesSubdivided;
	std::vector<Face> facesSubdivided;

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

	Edge * findEdge(int a, int b, std::vector<Edge> & edges) {
		for (int i = 0; i < edges.size(); ++i) {
			if ((edges[i].vertex1 == a && edges[i].vertex2 == b) || (edges[i].vertex1 == b && edges[i].vertex2 == a)) {
				return &edges[i]; 
			}
		}
		return nullptr;
	}


	std::vector<Edge> Edges() {
		std::vector<Edge> edges;
		for (int i = 0; i < IFaces.size(); i++) {
			for (int j = 0; j < 3; j++) {
				Edge e;
				e.vertex1 = IFaces[i][j];
				e.vertex2 = IFaces[i][(j + 1) % 3];

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
