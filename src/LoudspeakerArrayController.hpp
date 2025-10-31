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

	void DecodeToSpeakerArray(const CAmbisonicSoundField & OutBuffer, std::string decoding) {
		if (!setupDone) return;

		const auto & field = OutBuffer.GetSoundFieldArray();
		size_t order = OutBuffer.GetOrder();

		//Esto esta asumiendo ACN
		//Falta ampliar al orden 3
		const CMonoBuffer<float> & Wbuf = field[0];
		const CMonoBuffer<float> & Xbuf = field[3];
		const CMonoBuffer<float> & Ybuf = field[1];
		const CMonoBuffer<float> & Zbuf = field[2];

		const CMonoBuffer<float> * Vbuf = (order >= 2 && field.size() > 6) ? &field[6] : nullptr;
		const CMonoBuffer<float> * Tbuf = (order >= 2 && field.size() > 7) ? &field[7] : nullptr;
		const CMonoBuffer<float> * Rbuf = (order >= 2 && field.size() > 5) ? &field[5] : nullptr;
		const CMonoBuffer<float> * Sbuf = (order >= 2 && field.size() > 8) ? &field[8] : nullptr;
		const CMonoBuffer<float> * Ubuf = (order >= 2 && field.size() > 4) ? &field[4] : nullptr;

		const CMonoBuffer<float> * Qbuf = (order >= 2 && field.size() > 12) ? &field[12] : nullptr;
		const CMonoBuffer<float> * Obuf = (order >= 2 && field.size() > 13) ? &field[13] : nullptr;
		const CMonoBuffer<float> * Mbuf = (order >= 2 && field.size() > 11) ? &field[11] : nullptr;
		const CMonoBuffer<float> * Kbuf = (order >= 2 && field.size() > 14) ? &field[14] : nullptr;
		const CMonoBuffer<float> * Lbuf = (order >= 2 && field.size() > 10) ? &field[10] : nullptr;
		const CMonoBuffer<float> * Nbuf = (order >= 2 && field.size() > 15) ? &field[15] : nullptr;
		const CMonoBuffer<float> * Pbuf = (order >= 2 && field.size() > 9) ? &field[9] : nullptr;

		const float * W = Wbuf.data();
		const float * X = Xbuf.data();
		const float * Y = Ybuf.data();
		const float * Z = Zbuf.data();

		const float * V = Vbuf ? Vbuf->data() : nullptr;
		const float * T = Tbuf ? Tbuf->data() : nullptr;
		const float * R = Rbuf ? Rbuf->data() : nullptr;
		const float * S = Sbuf ? Sbuf->data() : nullptr;
		const float * U = Ubuf ? Ubuf->data() : nullptr;

		const float * Q = Vbuf ? Vbuf->data() : nullptr;
		const float * O = Tbuf ? Tbuf->data() : nullptr;
		const float * M = Rbuf ? Rbuf->data() : nullptr;
		const float * K = Sbuf ? Sbuf->data() : nullptr;
		const float * L = Ubuf ? Ubuf->data() : nullptr;
		const float * N = Sbuf ? Sbuf->data() : nullptr;
		const float * P = Ubuf ? Ubuf->data() : nullptr;

		size_t numSamples = OutBuffer.GetBufferSize();

		float Gain = std::sqrt(4 * M_PI / speakers.size());

		float Wgain;
		float XYZgain;

		if (decoding == "basic") {
			Wgain = std::sqrt(2) / speakers.size();
			XYZgain = 3.0f / speakers.size();
		} else if (decoding == "maxrE") {
			Wgain = 1.0f / std::sqrt(speakers.size());
			XYZgain = std::sqrt(3.0f / 2.0f) / std::sqrt(speakers.size());

		} else {
			Wgain = 0.0;
			XYZgain = 0.0;
		}

		for (size_t i = 0; i < speakers.size(); ++i) {
			Common::CVector3 pos = speakers[i]->GetSourceTransform().GetPosition();

			CMonoBuffer<float> buffer;
			buffer.resize(numSamples);

			for (size_t n = 0; n < numSamples; ++n) {
				if (order == 1) {
					buffer[n] = Wgain * W[n] + XYZgain * (pos.x * X[n] + pos.y * Y[n] + pos.z * Z[n]);
				} else if (order == 2) {
					buffer[n] = Wgain * (W[n] + pos.x * X[n] + pos.y * Y[n] + pos.z * Z[n])
						+ XYZgain * (sqrt(3) * pos.x * pos.y * V[n] + sqrt(3) * pos.y * pos.z * T[n] + (1 / 2) * ((3 * pow(pos.z, 2)) - 1) * R[n] + sqrt(3) * pos.x * pos.z * S[n] + sqrt(3 / 4) * (pow(pos.x, 2) - pow(pos.y, 2)) * U[n]);
				} else if (order == 3) {
					buffer[n] = Wgain * (W[n] + pos.x * X[n] + pos.y * Y[n] + pos.z * Z[n])
						+ XYZgain * (sqrt(3) * pos.x * pos.y * V[n] + sqrt(3) * pos.y * pos.z * T[n] + (1 / 2) * ((3 * pow(pos.z, 2)) - 1) * R[n] + sqrt(3) * pos.x * pos.z * S[n] + sqrt(3 / 4) * (pow(pos.x, 2) - pow(pos.y, 2)) * U[n]
						+ sqrt(5 / 8) * pos.y * (3 * pow(pos.x, 2) - pow(pos.y, 2)) * Q[n] + sqrt(15) * pos.x * pos.y * pos.z * O[n] + sqrt(3 / 8) * pos.y * (5 * pow(pos.z, 2) - 1) * M[n] + (1 / 2) * pos.z * (5 * pow(pos.z, 2) - 3) * K[n] + sqrt(3 / 8) * pos.x * (5 * pow(pos.z, 2) - 1) * L[n] + sqrt(15 / 4) * pos.z * (pow(pos.x, 2) - pow(pos.y, 2)) * N[n] + sqrt(5 / 8) * pos.x * (pow(pos.x, 2) - 3 * pow(pos.y, 2)) * P[n]);
				}
			}

			speakers[i]->SetBuffer(buffer);
		}
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
